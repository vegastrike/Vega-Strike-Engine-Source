#include <config.h>

#include "networking/const.h"
#include "vsnet_headers.h"

#include <list>

#include "vsnet_sockettcp.h"
#include "vsnet_err.h"
#include "vsnet_debug.h"

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
    , _incomplete_header( 0 )
    , _connection_closed( false )
    , _sq_off( 0 )
    , _mtu_size_estimation( 1024 )
{
#ifdef HAVE_FIOASYNC
    _sq_fd = ::dup2( sock );
    const int on = 1;
    ::ioctl( _sq_fd, FIOASYNC, &on ); // fine if it works, ignore if not
#else
    _sq_fd = sock;
#endif
}

VsnetTCPSocket::~VsnetTCPSocket( )
{
#ifdef HAVE_FIOASYNC
    ::close( _sq_fd );
#endif
    if( _incomplete_packet )
    {
        delete _incomplete_packet;
    }
}

int VsnetTCPSocket::sendbuf( PacketMem& packet, const AddressIP*, int pcktflags )
{
    int  idx = 1;
    if( pcktflags & LOPRI ) idx = 0;
    if( pcktflags & HIPRI ) idx = 2;

    /* Use a priority queue instead of a standard queue.
     * Add a timestamp here -- drop packets when they get too old.
     */

    _sq_mx.lock( );
    bool e = _sq.empty();

    _sq.push( SqPair(idx,packet) );
    _sq_count[idx]++;
    _sq_mx.unlock( );

    if( e ) _set.wakeup( );
    return packet.len();
}

void VsnetTCPSocket::private_nothread_conditional_write( )
{
#ifdef USE_NO_THREAD
    if( _connection_closed ) return;
    _set.waste_time( 0, 0 ); // waste zero time, but check sockets
#endif
}

bool VsnetTCPSocket::need_test_writable( )
{
    _sq_mx.lock( );
    bool e = ( _sq.empty() && _sq_current.empty() );
    _sq_mx.unlock( );
    return !e;
}

int VsnetTCPSocket::get_write_fd( ) const
{
    return _sq_fd;
}

int VsnetTCPSocket::optPayloadSize( ) const
{
    return _mtu_size_estimation;
}

int VsnetTCPSocket::queueLen( int pri )
{
    _sq_mx.lock( );
    int retval = _sq_count[pri];
    _sq_mx.unlock( );
    return retval;
}

int VsnetTCPSocket::lower_sendbuf( )
{
    _sq_mx.lock( );
    if( _sq_current.empty() )
    {
        if( _sq.empty() )
        {
            _sq_mx.unlock( );
            return 0;
        }
        else
        {
            Header h( _sq.top().second.len() );
            _sq_current.push( PacketMem( &h, sizeof(Header) ) );
            _sq_current.push( _sq.top().second );
            _sq_count[_sq.top().first]--;
            _sq.pop( );
            _sq_off = 0;
        }
    }

    PacketMem packet( _sq_current.front() );

    int          numsent;
    unsigned int len = packet.len();
    assert( len > _sq_off );
    len -= _sq_off;
    const char* buf = packet.getConstBuf();
    numsent = ::send( _sq_fd, &buf[_sq_off], len, 0 );
    if( numsent < 0 )
    {
        switch( errno )
        {
#if defined( _WIN32) && !defined( __CYGWIN__)
        case WSAEWOULDBLOCK :
#else
        case EWOULDBLOCK :
#endif
        case EINTR :
            _sq_mx.unlock( );
            return 0;
        default :
            perror( "\tsending TCP data : ");
            _sq_mx.unlock( );
            return -1;
        }
    }
    else if( numsent == 0 )
    {
        // other side closed socket - what to do now?
        _sq_mx.unlock( );
        return numsent;
    }
    else if( (unsigned int)numsent == len )
    {
        _sq_off = 0;
        _sq_current.pop( );
        _sq_mx.unlock( );
#ifdef VSNET_DEBUG
        //
        // DEBUG block - remove soon
        //
        {
            COUT << "sent buffer with len " << packet.len() << ": " << endl;
            packet.dump( cout, 0 );
        }
#endif
        return numsent;
    }
    else
    {
        assert( (unsigned int)numsent < len ); // should be impossible
        _sq_off += numsent;
        _sq_mx.unlock( );
        return numsent;
    }
}

int VsnetTCPSocket::recvbuf( PacketMem& buffer, AddressIP* )
{
    _cpq_mx.lock( );
    if( _cpq.empty() == false )
    {
        buffer = _cpq.front();
        _cpq.pop();
        _cpq_mx.unlock( );
        return buffer.len();
    }
    else if( _connection_closed )
    {
        _cpq_mx.unlock( );
        _set.rem_pending( _sq_fd );
        _fd = -1;
        COUT << __PRETTY_FUNCTION__ << " connection is closed" << endl;
        return 0;
    }
    else
    {
        _set.rem_pending( _sq_fd );
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

    bool retval = false;

    _cpq_mx.lock( );
    if( _cpq.empty() == false )
    {
        retval = true;
    }
    else
    {
        _set.rem_pending( _sq_fd );
    }
    _cpq_mx.unlock( );

    private_nothread_conditional_write( );

    return retval;
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
        if( ( _incomplete_header > 0 ) ||
	        ( _incomplete_header == 0 && _incomplete_packet == 0 ) )
	    {
	        assert( _incomplete_packet == 0 );   // we expect a len, can not have data yet
	        assert( _incomplete_header < sizeof(Header) ); // len is coded in sizeof(Header) bytes
	        int len = sizeof(Header) - _incomplete_header;
            char* b = (char*)&_header;
	        int ret = ::recv( _fd, &b[_incomplete_header], len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
					COUT << "Connection closed" << endl;
		            _connection_closed = true;
                    _fd = -1;
                    _set.add_pending( _sq_fd );
		        }
                else if( vsnetEWouldBlock() == false )
                {
                    perror( "receiving TCP packetlength bytes" );
                }
                return;
	        }
	        if( ret > 0 ) _incomplete_header += ret;
	        if( _incomplete_header == sizeof(Header) )
	        {
	            _incomplete_header = 0;
		        len = _header.h_len();
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
                    _fd = -1;
                    _set.add_pending( _sq_fd );
		        }
                else if( vsnetEWouldBlock() == false )
		        {
                    perror( "receiving TCP packet" );
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
    _set.add_pending( _sq_fd );
    delete b;
}

