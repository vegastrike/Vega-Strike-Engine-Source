#ifndef VSNET_SOCKET_UDP_H
#define VSNET_SOCKET_UDP_H

#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif
#include <queue>

#include "vsnet_socket.h"
#include "packetmem.h"

/***********************************************************************
 * VsnetUDPSocket - declaration
 ***********************************************************************/
 
class VsnetUDPSocket : public VsnetSocket
{
public:
    VsnetUDPSocket( int sock, const AddressIP& remote_ip, SocketSet& set );
    ~VsnetUDPSocket( );

    virtual bool isTcp() const { return false; }

    virtual bool needReadAlwaysTrue( ) const;
    virtual bool isActive( );
    virtual int  sendbuf( PacketMem& packet, const AddressIP* to);
    virtual int  recvbuf( PacketMem& buffer, AddressIP *from);
    // virtual void ack( );

    virtual void lower_selected( );

    virtual void dump( std::ostream& ostr ) const;

private:
    struct Pending
    {
        Pending( const void* buffer, size_t size, AddressIP& addr )
            : mem( buffer, size )
            , ip( addr )
        { }

        PacketMem mem;
        AddressIP ip;
    };

    std::queue<Pending> _cpq;

    size_t _negotiated_max_size;
    char*  _recv_buf;

private:
    VsnetUDPSocket( );
    VsnetUDPSocket( const VsnetUDPSocket& orig );
    VsnetUDPSocket& operator=( const VsnetUDPSocket& orig );
};

#endif /* VSNET_SOCKET_UDP_H */

