#ifndef VSNET_SOCKET_TCP_H
#define VSNET_SOCKET_TCP_H

#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

#include <list>

#include "vsnet_socket.h"

/***********************************************************************
 * VsnetTCPSocket - declaration
 ***********************************************************************/
 
class VsnetTCPSocket : public VsnetSocket
{
public:
    VsnetTCPSocket( )
        : _incomplete_packet( 0 )
	, _incomplete_len_field( 0 )
	, _connection_closed( false )
    { }

    VsnetTCPSocket( int sock, const AddressIP& remote_ip )
        : VsnetSocket( sock, remote_ip )
        , _incomplete_packet( 0 )
	, _incomplete_len_field( 0 )
	, _connection_closed( false )
    { }

    VsnetTCPSocket( const VsnetTCPSocket& orig )
        : VsnetSocket( orig )
    { }

    VsnetTCPSocket& operator=( const VsnetTCPSocket& orig )
    {
        VsnetSocket::operator=( orig );
        return *this;
    }

    virtual ~VsnetTCPSocket( );

    virtual bool isTcp() const { return true; }

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to);
    virtual int  recvbuf( void *buffer, unsigned int &len, AddressIP *from);
    virtual int  recvbuf( PacketMem& buffer, AddressIP *from);
    virtual void ack( );

    virtual void disconnect( const char *s, bool fexit );

    virtual void dump( std::ostream& ostr ) const;

    virtual void watch( SocketSet& set );
    virtual bool isActive( SocketSet& set );

private:
    /* --- BEGIN section for nonblocking receive support --- */

    struct Blob;
#if 0
    struct Blob
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
#endif

    /** if we have received part of a TCP packet but not the complete packet,
     *  the expected length and received number of bytes are stored in
     *  _incomplete_packet. If several packets have been received at once, but
     *  the application processes them one at a time, the received, unprocessed
     *  packets are stored in the list.
     */
    std::list<Blob*> _complete_packets;
    Blob*            _incomplete_packet;

    /** We send 4 bytes as a packet length indicator. Unfortunately, even these
     *  4 bytes may be split by TCP. These two variables are needed for
     *  collecting the 4 bytes.
     *  Note: for the obvious reason that this happens rarely, the collection
     *        code can not be considered tested.
     */
    int         _incomplete_len_field;
    char        _len_field[4];

    /** Closed connections are noticed in isActive but evaluated by the
     *  application after recvbuf. So, we remember the situation here until
     *  the application notices it.
     */
    bool        _connection_closed;

    /* --- END section for nonblocking receive support --- */
};

#endif /* VSNET_SOCKET_TCP_H */

