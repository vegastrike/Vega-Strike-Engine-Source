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
#include <queue>

#include "vsnet_socket.h"
#include "vsnet_thread.h"
#include "packetmem.h"

/***********************************************************************
 * VsnetTCPSocket - declaration
 ***********************************************************************/
 
class VsnetTCPSocket : public VsnetSocket
{
public:
    VsnetTCPSocket( int sock, const AddressIP& remote_ip, SocketSet& set );

    virtual ~VsnetTCPSocket( );

    virtual bool isTcp() const { return true; }

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to);
    virtual int  recvbuf( PacketMem& buffer, AddressIP *from);
    // virtual void ack( );

    virtual void dump( std::ostream& ostr ) const;

    virtual bool isActive( );

    virtual void lower_selected( );

protected:
    virtual void child_disconnect( const char *s );

private:
    /* --- BEGIN section for nonblocking receive support --- */

    struct Blob;

    /** if we have received part of a TCP packet but not the complete packet,
     *  the expected length and received number of bytes are stored in
     *  _incomplete_packet. If several packets have been received at once, but
     *  the application processes them one at a time, the received, unprocessed
     *  packets are stored in the list.
     */
    Blob*                 _incomplete_packet;
    std::queue<PacketMem> _cpq;
    VSMutex               _cpq_mx;

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

    void inner_complete_a_packet( Blob* b );

private:
    VsnetTCPSocket( );
    VsnetTCPSocket( const VsnetTCPSocket& orig );
    VsnetTCPSocket& operator=( const VsnetTCPSocket& orig );
};

#endif /* VSNET_SOCKET_TCP_H */

