#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

#include <list>

#include "const.h"
#include "vsnet_sockettcp.h"
#include "vsnet_err.h"

using namespace std;

using std::cout;
using std::cerr;
using std::endl;

/***********************************************************************
 * VsnetTCPSocket::Blob - declaration
 ***********************************************************************/

struct VsnetTCPSocket::Blob
{
    char*  buf;
    size_t present_len;
    size_t expected_len;

    Blob( ) : buf(0), present_len(0), expected_len(0) { }

    Blob( size_t len ) : present_len(0), expected_len(len)
    {
        buf = new char[len];
    }

    ~Blob( )
    {
        delete [] buf;
    }

private:
    Blob( const Blob& orig );             // forbidden
    Blob& operator=( const Blob& orig );  // forbidden
};

/***********************************************************************
 * VsnetTCPSocket - definition
 ***********************************************************************/
 
VsnetTCPSocket::~VsnetTCPSocket( )
{
    while( !_complete_packets.empty() )
    {
        Blob* b = _complete_packets.front();
	    _complete_packets.pop_front();
	    delete b;
    }

    if( _incomplete_packet )
    {
	    delete _incomplete_packet;
    }
}

int VsnetTCPSocket::sendbuf( PacketMem& packet, const AddressIP* to)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    int numsent;

#ifdef VSNET_DEBUG
    //
    // DEBUG block - remove soon
    //
    {
        COUT << "trying to send buffer with len " << packet.len() << ": " << endl;
	    packet.dump( cout, 0 );
    }
#endif

    unsigned int len = htonl( packet.len() );
    if( (numsent=send( _fd, (char *)&len, 4, 0 )) < 0 )
    {
        perror( "\tsending TCP packet len : ");
        if( errno == EBADF) return -1;
    }
    if( (numsent=send( _fd, packet.getConstBuf(), packet.len(), 0))<0)
    {
        perror( "\tsending TCP data : ");
        if( errno == EBADF) return -1;
    }
    return numsent;
}

void VsnetTCPSocket::ack( )
{
    /* meaningless, TCP is reliable */
}

int VsnetTCPSocket::recvbuf( void *buffer, unsigned int& len, AddressIP* from)
{
    if( _complete_packets.empty() == false )
    {
        Blob* b = _complete_packets.front();
        _complete_packets.pop_front();
        assert( b );
        assert( b->present_len == b->expected_len );
        if( len > b->present_len ) len = b->present_len;
        memcpy( buffer, b->buf, len );
        delete b;
	    assert( len > 0 );
        return len;
    }
    else
    {
        if( _connection_closed )
	    {
	        COUT << __PRETTY_FUNCTION__ << " connection is closed" << endl;
	        return 0;
	    }
        return -1;
    }
}

int VsnetTCPSocket::recvbuf( PacketMem& buffer, AddressIP* )
{
    if( _complete_packets.empty() == false )
    {
	    unsigned int ret;
        Blob* b = _complete_packets.front();
        _complete_packets.pop_front();
        assert( b );
        assert( b->present_len == b->expected_len );

	    ret = b->present_len;
	    buffer.set( b->buf, ret, PacketMem::TakeOwnership );
	    b->buf = NULL;
        delete b;
        return ret;
    }
    else
    {
        if( _connection_closed )
        {
	        _fd = -1;
            COUT << __PRETTY_FUNCTION__ << " connection is closed" << endl;
            return 0;
        }
        return -1;
    }
}

void VsnetTCPSocket::disconnect( const char *s, bool fexit )
{
    if( _fd > 0 )
    {
        if( close_socket( _fd ) < 0 )
        {
            COUT << s << " :\tWarning: disconnected" << strerror(errno) << endl;
        }
        else
        {
            COUT << s << " :\tWarning: disconnected" << endl;
        }
        _fd = -1;
    }
    else
    {
        COUT << s << " :\tWarning: disconnected" << strerror(errno) << endl;
    }

    if( fexit )
    {
        exit(1);
    }
}

void VsnetTCPSocket::dump( std::ostream& ostr ) const
{
    ostr << "( s=" << _fd << " TCP r=" << _remote_ip << " )";
}

ostream& operator<<( ostream& ostr, const VsnetSocket& s )
{
    s.dump( ostr );
    return ostr;
}

void VsnetTCPSocket::watch( SocketSet& set )
{
#ifdef FIND_WIN_NBIO
        COUT << "Wait for data on socket " << (*this) << " ("
             << ( get_nonblock() ? "non-blocking" : "blocking" ) << ")"
             << endl;
#endif
    set.setRead( _fd );
    if( _complete_packets.empty() == false )
    {
#ifdef FIND_WIN_NBIO
        COUT << "Socket " << (*this) << " has completed packets" << endl;
#endif
        set.setReadAlwaysTrue( _fd );
    }
}

bool VsnetTCPSocket::isActive( SocketSet& set )
{
    COUT << "enter " << "isActive" << endl;

    /* True is the correct answer when the connection is closed:
     * the app must call recvbuf once after this to receive 0
     * and start close processing.
     */
    if( _connection_closed )
    {
        COUT << "leave " << "isActive" << endl;
        return true;
    }

    if( set.is_setRead(_fd) == false )
    {
        if( _complete_packets.empty() == false )
        {
            COUT << "leave " << "isActive" << endl;
            return true;
        }
        else
        {
            COUT << "leave " << "isActive" << endl;
            return false;
        }
    }

    bool endless   = true;
    bool gotpacket = false;

    if( _noblock == 0 ) endless = false;

    do
    {
	    if( ( _incomplete_len_field > 0 ) ||
	        ( _incomplete_len_field == 0 && _incomplete_packet == 0 ) )
	    {
	        assert( _incomplete_packet == 0 );   // we expect a len, can not have data yet
	        assert( _incomplete_len_field < 4 ); // len is coded in 4 bytes
	        int len = 4 - _incomplete_len_field;
	        int ret = recv( _fd, &_len_field[_incomplete_len_field], len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
		            _connection_closed = true;
                    COUT << "leave " << "isActive" << endl;
		            return ( _complete_packets.empty() == false );
		        }
                if( vsnetEWouldBlock() )
		        {
                    COUT << "leave " << "isActive"  << endl;
		            return ( _complete_packets.empty() == false );
		        }
		        else
		        {
                    COUT << "leave " << "isActive" << endl;
	                return false;
		        }
	        }
	        if( ret > 0 ) _incomplete_len_field += ret;
	        if( _incomplete_len_field == 4 )
	        {
	            _incomplete_len_field = 0;
		        len = ntohl( *(unsigned int*)_len_field );
		        COUT << "Next packet to receive has length " << len << endl;
                _incomplete_packet = new Blob( len );
	        }
	    }
        if( _incomplete_packet != 0 )
	    {
	        int len = _incomplete_packet->expected_len - _incomplete_packet->present_len;
	        int ret = recv( _fd, _incomplete_packet->buf, len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
		            _connection_closed = true;
                    COUT << "leave " << "isActive" << endl;
		            return ( _complete_packets.empty() == false );
		        }
                if( vsnetEWouldBlock() )
		        {
                    COUT << "leave " << "isActive" << endl;
		            return ( _complete_packets.empty() == false );
		        }
		        else
		        {
                    COUT << "leave " << "isActive" << endl;
	                return false;
		        }
	        }
	        if( ret > 0 ) _incomplete_packet->present_len += len;
	        if( ret == len )
	        {
		        assert( _incomplete_packet->expected_len ==
                        _incomplete_packet->present_len );
                _complete_packets.push_back( _incomplete_packet );
		        gotpacket = true;
#ifdef VSNET_DEBUG
                //
                // DEBUG block - remove soon
                //
                {
                    Blob* b = _incomplete_packet;
                    COUT << "received buffer with len " << b->present_len << ": " << endl;
		            PacketMem m( b->buf, b->present_len, PacketMem::LeaveOwnership );
		            m.dump( cout, 3 );
                }
#endif

		        _incomplete_packet = 0;
                // either endless is false, or we exit with EWOULDBLOCK
	        }
        }
    }
    while( endless );  // exit only for EWOULDBLOCK or closed socket

    COUT << "leave " << "isActive" << " blocking socket" << endl;

    return gotpacket;
}

