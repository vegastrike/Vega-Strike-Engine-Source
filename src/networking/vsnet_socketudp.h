#ifndef VSNET_SOCKET_UDP_H
#define VSNET_SOCKET_UDP_H

#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

#include "vsnet_socket.h"

/***********************************************************************
 * VsnetUDPSocket - declaration
 ***********************************************************************/
 
class VsnetUDPSocket : public VsnetSocket
{
public:
    VsnetUDPSocket( ) { }

    VsnetUDPSocket( int sock, const AddressIP& remote_ip )
        : VsnetSocket( sock, remote_ip )
    { }

    VsnetUDPSocket( const VsnetUDPSocket& orig )
        : VsnetSocket( orig )
    { }

    VsnetUDPSocket& operator=( const VsnetUDPSocket& orig )
    {
        VsnetSocket::operator=( orig );
        return *this;
    }

    virtual bool isTcp() const { return false; }

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to);
    virtual int  recvbuf( void *buffer, unsigned int &len, AddressIP *from);
    virtual int  recvbuf( PacketMem& buffer, AddressIP *from);
    virtual void ack( );

    virtual void disconnect( const char *s, bool fexit );

    virtual void dump( std::ostream& ostr ) const;

    virtual void watch( SocketSet& set );
    virtual bool isActive( SocketSet& set ) { return set.is_set(_fd); }
};

#endif /* VSNET_SOCKET_UDP_H */

