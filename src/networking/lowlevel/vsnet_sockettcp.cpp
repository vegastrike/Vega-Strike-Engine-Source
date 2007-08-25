#include <config.h>

#include "networking/const.h"
#include "vsnet_headers.h"

#include <list>

#include "vsnet_sockettcp.h"
#include "vsnet_err.h"
#include "vsnet_oss.h"
#include "vsnet_debug.h"
#include "packet.h"

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
    : VsnetSocket( sock, remote_ip, "VsnetTCPSocket", sets )
    , _incomplete_packet( 0 )
    , _incomplete_header( 0 )
    , _connection_closed( false )
    , _sq_off( 0 )
    , _mtu_size_estimation( 1024 )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    _sq_fd = sock;
}

VsnetTCPSocket::~VsnetTCPSocket( )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    if( _incomplete_packet )
    {
        delete _incomplete_packet;
    }
	if (valid()&&!_connection_closed&&get_fd()>=0) {
		close_fd();
	}
}

// int VsnetTCPSocket::sendbuf( PacketMem& packet, const AddressIP*, int pcktflags )
// {
//     int  idx = 1;
//     if( pcktflags & LOPRI ) idx = 0;
//     if( pcktflags & HIPRI ) idx = 2;
// 
//     /* Use a priority queue instead of a standard queue.
//      * Add a timestamp here -- drop packets when they get too old.
//      */
// 
//     _sq_mx.lock( );
//     bool e = _sq.empty();
// 
//     _sq.push( idx, packet );
//     _sq_mx.unlock( );
// 
//     if( e ) _set.wakeup( );
//     return packet.len();
// }

int VsnetTCPSocket::sendbuf( Packet* packet, const AddressIP*, int pcktflags )
{
    int  idx = 1;
    if( pcktflags & LOPRI ) idx = 0;
    if( pcktflags & HIPRI ) idx = 2;

    /* Add a timestamp here -- drop packets when they get too old.
     */

    PacketPtr enq = PacketPtr( new Packet( *packet ) );

    _sq_mx.lock( );
    bool e = _sq.empty();

    _sq.push( idx, enq );
    _sq_mx.unlock( );

    if( e ) _set.wakeup( );
    return packet->getSendBufferLength();
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
#ifdef VSNET_DEBUG
    if( !e )
    {
        COUT << "_sq "
             << (_sq.empty()?"empty":"not empty")
             << ", _sq_current "
             << (_sq_current.empty()?"empty":"not empty") << endl;
    }
#endif
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
    int retval = _sq.getLength(pri);
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
#ifdef VSNET_DEBUG
            COUT << "both queues are empty" << endl;
#endif
            _sq_mx.unlock( );
            return 0;
        }
        else
        {
            PacketPtr m = _sq.pop( );
            int len = m->getSendBufferLength();
            Header h( len );
            _sq_current.push( PacketMem( &h, sizeof(Header) ) );
            _sq_current.push( PacketMem( m->getSendBuffer(), len ) );
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
        case WSAECONNRESET :   // other side closed socket
        case WSAECONNABORTED : // other side closed socket
        case WSAEWOULDBLOCK :
#else
        case ECONNRESET :   // other side closed socket
        case ECONNABORTED : // other side closed socket
        case EWOULDBLOCK :
#endif
        case EINTR :
            _sq_mx.unlock( );
            return 0;
        case EFAULT :
            _sq_mx.unlock( );
            COUT << "EFAULT" << endl
                 << "   *** An invalid user space address was specified for a parameter." << endl
                 << "   *** fd        : " << _sq_fd << endl
                 << "   *** buf       : " << std::hex << (long)buf << std::dec << endl
                 << "   *** sq offset : " << _sq_off << endl
                 << "   *** packet len: " << packet.len() << endl;
            return 0;
        default :
	    fprintf (stderr,"Failed sending TCP data of %d len to socket %d\n",len,_sq_fd);
            perror( "\tsending TCP data");
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
#if 0
            packet.dump( cout, 0 );
#endif
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

void VsnetTCPSocket::lower_clean_sendbuf( )
{
    _sq_mx.lock( );
    while( !_sq.empty() )
    {
        _sq.pop( );
#ifdef VSNET_DEBUG
        COUT << "forgetting a packet in _sq" << endl;
#endif
    }

    while( !_sq_current.empty() )
    {
        _sq_current.pop( );
#ifdef VSNET_DEBUG
        COUT << "forgetting a segment in _sq_current" << endl;
#endif
        _sq_off = 0;
    }

    _sq_mx.unlock( );
}

int VsnetTCPSocket::recvbuf( Packet* p, AddressIP* ipadr )
{
    _cpq_mx.lock( );
    if( _cpq.empty() == false )
    {
        PacketPtr ptr = _cpq.front();
        _cpq.pop();
        _cpq_mx.unlock( );

        AddressIP dummy;
        if(ipadr) *ipadr = _remote_ip;
        p->copyfrom( *ptr.get() );
        return ( ptr->getDataLength() + ptr->getHeaderLength() );
    }
    else if( _connection_closed )
    {
        _set.rem_pending( _sq_fd );
        _cpq_mx.unlock( );
        close_fd( );
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
    if( get_fd() > 0 )
    {
        if( close_fd() < 0 )
        {
            COUT << s << " :\tWarning: disconnect error: " << strerror(errno) << endl;
        }
        else
        {
            COUT << s << " :\tWarning: disconnected" << endl;
        }
    }
    else
    {
        COUT << s << " :\tWarning: disconnect null socket: " << strerror(errno) << endl;
    }
}

void VsnetTCPSocket::dump( std::ostream& ostr ) const
{
    ostr << "( s=" << get_fd() << " TCP r=" << _remote_ip << " )";
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
    if( _connection_closed )
    {
        return true;
    }

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

bool VsnetTCPSocket::lower_selected( int datalen )
{
    if( _connection_closed )
    {
		COUT << "Connection already closed" << endl;
        return true; /* Pretty sure that recv will return 0.  */
    }

    bool endless   = true;
	bool successful = false;

    if( get_nonblock() == false && datalen==-1 ) endless = false;

    do
    {
        if( ( _incomplete_header > 0 ) ||
	        ( _incomplete_header == 0 && _incomplete_packet == 0 ) )
	    {
	        assert( _incomplete_packet == 0 );   // we expect a len, can not have data yet
	        assert( _incomplete_header < sizeof(Header) ); // len is coded in sizeof(Header) bytes
	        int len = sizeof(Header) - _incomplete_header;
            char* b = (char*)&_header;
	        int ret = VsnetOSS::recv( get_fd(), &b[_incomplete_header], len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 || vsnetEConnAborted() || vsnetEConnReset() )
		        {
                    COUT << "Connection closed in header" << endl;
                    _connection_closed = true;
                    close_fd();
                    _set.add_pending( _sq_fd );
					return true;
		        }
                else if( vsnetEWouldBlock() == false )
                {
                    COUT << "recv returned " << ret
                         << " errno " << vsnetGetLastError()
                         << " = " << vsnetLastError()
                         << endl;
                    perror( "receiving TCP packetlength bytes" );
                    _connection_closed = true;
					close_fd();
					return true;
                } else {
		    //COUT << "Received EWOULDBLOCK." << (get_nonblock()?"true":"false") << endl;
		}
		    return successful;
	        }
	        if( ret > 0 ) _incomplete_header += ret;
            if (datalen!=-1) datalen -= ret;
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
            if (datalen>0) {
                len = datalen<len?datalen:len;
            }
	        int ret = VsnetOSS::recv( get_fd(), _incomplete_packet->base(), len, 0 );
	        assert( ret <= len );
	        if( ret <= 0 )
	        {
	            if( ret == 0 )
		        {
                    COUT << "Connection closed in data" << endl;
		            _connection_closed = true;
                    close_fd();
                    _set.add_pending( _sq_fd );
		        }
		    else if( vsnetEConnAborted() ) {
			static int i=0;
			//if (i++<1000||i%1023==0)
			perror( "receiving TCP packet" );
			
			if (get_fd()==-1) {
			    perror( "receiving dead TCP packet" );
			    }else{
			    COUT << "Connection closed in error" << endl;
			    _connection_closed = true;
			    close_fd();
			    _set.add_pending( _sq_fd );					
			}
			return true;
		    } else if (vsnetEWouldBlock()){
			static int i=0;
			if (i++%128==0)
			    COUT << "Received EWOULDBLOCK in data." << (get_nonblock()?"true":"false") << endl;
		    }
                return successful;
	        }
            else
            {
                if (datalen!=-1) {
                    datalen -= ret;
                    if (datalen<=0)
                        endless=false;
                }
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
#if 0
		                m.dump( cout, 3 );
#endif
                    }
#endif

                    inner_complete_a_packet( _incomplete_packet );
		            _incomplete_packet = 0;
					successful=true;
					endless = false; // If we're done, let's stop while we're ahead.
					
                    // either endless is false, or we exit with EWOULDBLOCK
	            }
            }
        }
    }
    while( endless );  // exit only for EWOULDBLOCK or closed socket
	return successful;
}

void VsnetTCPSocket::inner_complete_a_packet( Blob* b )
{
    assert( b );
    assert( b->present_len == b->expected_len );

    PacketMem mem( b->buf, b->present_len, PacketMem::TakeOwnership );
    PacketPtr ptr = PacketPtr( new Packet( mem ) );
    //if (ptr->getCommand()!=CMD_POSUPDATE && ptr->getCommand()!=CMD_SNAPSHOT)
    //    COUT << "Completely received a packet of type " << ptr->getCommand() << endl;
    b->buf = NULL;
    _cpq_mx.lock( );
    _cpq.push( ptr );
    _cpq_mx.unlock( );
    _set.add_pending( _sq_fd );
    delete b;
}

/***********************************************************************
 * VsnetTCPSocket::SqQueues
 ***********************************************************************/

VsnetTCPSocket::SqQueues::SqQueues( )
{
    _ct = 0;
    // _debug_array[0] = 0;
    // _debug_array[1] = 0;
    // _debug_array[2] = 0;
    // _debug_array[3] = 0;
}

bool VsnetTCPSocket::SqQueues::empty( ) const
{
    return (_ct == 0);
}

void VsnetTCPSocket::SqQueues::push( int idx, PacketPtr m )
{
    _q[idx].push( m );
    _ct++;
    // if( idx >= 0 && idx < 3 ) _debug_array[idx]++;
    // else                      _debug_array[3]++;
}

int VsnetTCPSocket::SqQueues::getLength( int idx )
{
    std::map<int,SqQueue>::iterator it;
    it = _q.find(idx);
    if( it == _q.end() ) return 0;
    return it->second.size( );
}

VsnetTCPSocket::PacketPtr VsnetTCPSocket::SqQueues::pop( )
{
    /* We need the reverse iterators because in this code, higher numbers
     * indicate higher priorities, and those are further "back" in the
     * map.
     */
    assert( _ct > 0 );
    std::map<int,SqQueue>::reverse_iterator it;
    for( it=_q.rbegin(); it!=_q.rend(); it++ )
    {
        if( it->second.empty() == false )
        {
            PacketPtr r = it->second.front( );
            it->second.pop( );
            _ct--;
            return r;
        }
    }
    assert( 0 );
    return PacketPtr();
}

