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
    VsnetUDPSocket( int sock, const AddressIP& remote_ip, SocketSet& set )
        : VsnetSocket( sock, remote_ip, set )
    { }

    virtual bool isTcp() const { return false; }

    virtual int  sendbuf( PacketMem& packet, const AddressIP* to);
    virtual int  recvbuf( PacketMem& buffer, AddressIP *from);
    virtual void ack( );

    virtual void dump( std::ostream& ostr ) const;

private:
    virtual int  inner_recvbuf( void *buffer, unsigned int &len, AddressIP *from);

private:
    VsnetUDPSocket( );
    VsnetUDPSocket( const VsnetUDPSocket& orig );
    VsnetUDPSocket& operator=( const VsnetUDPSocket& orig );
};

#endif /* VSNET_SOCKET_UDP_H */

