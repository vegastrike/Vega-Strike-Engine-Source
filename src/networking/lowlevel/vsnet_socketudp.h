#ifndef VSNET_SOCKET_UDP_H
#define VSNET_SOCKET_UDP_H

#include "vsnet_headers.h"
#include <queue>

#include "vsnet_socketbase.h"
#include "vsnet_thread.h"
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

    virtual int  optPayloadSize( ) const;
    virtual int  queueLen( int pri );

    virtual bool isActive( );
    // virtual int  sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags );
    virtual int  sendbuf( Packet* packet, const AddressIP* to, int pcktflags );
    virtual int  recvbuf( Packet* p, AddressIP* ipadr );

    virtual bool lower_selected( int datalen=-1 );

    virtual void dump( std::ostream& ostr ) const;
    virtual bool setRemoteAddress(const AddressIP &inp);
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
    VSMutex             _cpq_mx;

    size_t _negotiated_max_size;
    char*  _recv_buf;

private:
    /** This will eventually contains the MTU size estimation in bytes.
     *  Dummy for now.
     */
    int _mtu_size_estimation;

private:
    VsnetUDPSocket( );
    VsnetUDPSocket( const VsnetUDPSocket& orig );
    VsnetUDPSocket& operator=( const VsnetUDPSocket& orig );
};

#endif /* VSNET_SOCKET_UDP_H */

