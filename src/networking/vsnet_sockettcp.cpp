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

    inline size_t missing( ) const {
        return expected_len - present_len;
    }

    inline char* base( ) {
        return &buf[present_len];
    }

private:
    Blob( const Blob& orig );             // forbidden
    Blob& operator=( const Blob& orig );  // forbidden
};

/***********************************************************************
 * VsnetTCPSocket - definition
 ***********************************************************************/
 
VsnetTCPSocket::VsnetTCPSocket( int sock, const AddressIP& remote_ip, SocketSet& sets )
    : VsnetSocket( sock, remote_ip, sets )
    , _incomplete_packet( 0 )
    , _incomplete_len_field( 0 )
    , _connection_closed( false )
{ }

VsnetTCPSocket::~VsnetTCPSocket( )
{
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

int VsnetTCPSocket::recvbuf( PacketMem& buffer, AddressIP* )
{
    _cpq_mx.lock( );
    if( _cpq.empty() == false )
    {
        buffer = _cpq.front();
        _cpq.pop();
        _cpq_mx.unlock( );
        _set.dec_pending( );
        return buffer.len();
    }
    else if( _connection_closed )
    {
        _cpq_mx.unlock( );
        _fd = -1;
        COUT << __PRETTY_FUNCTION__ << " connection is closed" << endl;
        return 0;
    }
    else
    {
        _cpq_mx.unlock( );
        return -1;
    }
}

void VsnetTCPSocket::child_disconnect( const char *s )
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

bool VsnetTCPSocket::isActive( )
{
    /* True is the correct answer when the connection is closed:
     * the app must call recvbuf once after this to receive 0
     * and start close processing.
     * We could return packets from the queue first, but that may
     * trigger an answer packet from the application, and give
     * us trouble because of the closed socket.
     */
    if( _connection_closed ) return true;

    _cpq_mx.lock( );
    if( _cpq.empty() == false )
    {
        _cpq_mx.unlock( );
        return true;
    }
    _cpq_mx.unlock( );

    return false;
}

void VsnetTCPSocket::lower_selected( )
{
    if( _connection_closed )
    {
        return; /* Pretty sure that recv will return 0.  */
    }

    bool endless   = true;

    if( get_nonblock() == false ) endless = false;

    do
    {
	    if( ( _incomplete_len_field > 0 ) ||
	        ( _incomplete_len_field == 0 && _incomplete_packet == 0 ) )
	    {
	        assert( _incomplete_packet == 0 );   // we expect a len, can not have data yet
	        assert( _incomplete_len_field < 4 ); // len is coded in 4 bytes
	        int len = 4 - _incomplete_len_field;
	        int ret = ::recv( _fd, &_len_field[_incomplete_len_field], len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
		            _connection_closed = true;
                    _fd = -1;
		        }
                else if( vsnetEWouldBlock() == false )
                {
                    perror( "receiving TCP packet length bytes" );
                }
                return;
	        }
	        if( ret > 0 ) _incomplete_len_field += ret;
	        if( _incomplete_len_field == 4 )
	        {
	            _incomplete_len_field = 0;
		        len = ntohl( *(unsigned int*)_len_field );
                _incomplete_packet = new Blob( len );
	        }
	    }
        if( _incomplete_packet != 0 )
	    {
	        int len = _incomplete_packet->missing( );
	        int ret = ::recv( _fd, _incomplete_packet->base(), len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
		            _connection_closed = true;
		        }
                else if( vsnetEWouldBlock() == false )
		        {
                    perror( "receiving TCP packet bytes" );
		        }
                return;
	        }
            else
            {
	            _incomplete_packet->present_len += ret;
	            if( ret == len )
	            {
		            assert( _incomplete_packet->expected_len ==
                            _incomplete_packet->present_len );
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

                    inner_complete_a_packet( _incomplete_packet );
		            _incomplete_packet = 0;
                    // either endless is false, or we exit with EWOULDBLOCK
	            }
            }
        }
    }
    while( endless );  // exit only for EWOULDBLOCK or closed socket
}

void VsnetTCPSocket::inner_complete_a_packet( Blob* b )
{
    assert( b );
    assert( b->present_len == b->expected_len );

    PacketMem mem( b->buf, b->present_len, PacketMem::TakeOwnership );
    b->buf = NULL;
    _cpq_mx.lock( );
    _cpq.push( mem );
    _cpq_mx.unlock( );
    _set.inc_pending( );
    delete b;
}

