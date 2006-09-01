#include <config.h>

#include "vsnet_headers.h"

#include <list>

#include "networking/const.h"
#include "vsnet_socketudp.h"
#include "vsnet_err.h"
#include "vsnet_debug.h"
#include "packet.h"

using namespace std;

using std::cout;
using std::cerr;
using std::endl;

/***********************************************************************
 * VsnetUDPSocket - definition
 ***********************************************************************/
 
VsnetUDPSocket::VsnetUDPSocket( int sock, const AddressIP& remote_ip, SocketSet& socketset )
    : VsnetSocket( sock, remote_ip, "VsnetUDPSocket", socketset )
    , _mtu_size_estimation( 1024 )
{
    _negotiated_max_size = MAXBUFFER;
    _recv_buf            = new char[MAXBUFFER];
}

VsnetUDPSocket::~VsnetUDPSocket( )
{
    delete [] _recv_buf;
}

int VsnetUDPSocket::optPayloadSize( ) const
{
    return _mtu_size_estimation;
}

int VsnetUDPSocket::queueLen( int )
{
    return 0;
}

// int VsnetUDPSocket::sendbuf( PacketMem& packet, const AddressIP* to, int pcktflags )
// {
//     COUT << "enter " << __PRETTY_FUNCTION__ << endl;
//     int numsent;
// 
//     // In UDP mode, always send on this->sock
//     const sockaddr_in* dest = to;
//     if( dest == NULL ) dest = &_remote_ip;
// 
//     assert( dest != NULL );
// 
//     if( (numsent = sendto( get_fd(), packet.getConstBuf(), packet.len(), 0, (sockaddr*) dest, sizeof(struct sockaddr_in)))<0)
//     {
//         COUT << "Error sending: " << vsnetLastError() << endl;
//         return -1;
//     }
//     cout<<"Sent "<<numsent<<" bytes"<<" -> "<<inet_ntoa( dest->sin_addr)<<":"<<ntohs(dest->sin_port)<<endl;
//     return numsent;
// }
bool VsnetUDPSocket::setRemoteAddress(const AddressIP&to) {
  _remote_ip=to;
  return true;
}
int VsnetUDPSocket::sendbuf( Packet* packet, const AddressIP* to, int pcktflags )
{
//    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    int numsent;

    // In UDP mode, always send on this->sock
    const sockaddr_in* dest = to;
    if( dest == NULL ) dest = &_remote_ip;

    assert( dest != NULL );

    numsent = sendto( get_fd(),
                      packet->getSendBuffer(), packet->getSendBufferLength(),
                      0, (sockaddr*) dest, sizeof(struct sockaddr_in));
    if( numsent < 0 )
    {
        COUT << "Error sending: " << vsnetLastError() << endl;
        return -1;
    }
//    cout<<"Sent "<<numsent<<" bytes"<<" -> "<<inet_ntoa( dest->sin_addr)<<":"<<ntohs(dest->sin_port)<<endl;
    return numsent;
}

int VsnetUDPSocket::recvbuf( Packet* p, AddressIP* ipadr )
{
    _cpq_mx.lock( );
    if( _cpq.empty() )
    {
        _cpq_mx.unlock( );
        _set.rem_pending( get_fd() );
        return -1;
    }

    PacketMem buffer = _cpq.front().mem;
    if(ipadr) *ipadr = _cpq.front().ip;
    _cpq.pop();
    _cpq_mx.unlock( );
    int len = buffer.len();
    Packet packet( buffer );
    *p = packet;
    return len;
}

void VsnetUDPSocket::dump( std::ostream& ostr ) const
{
    ostr << "( s=" << get_fd() << " UDP r=" << _remote_ip << " )";
}

bool VsnetUDPSocket::isActive( )
{
    _cpq_mx.lock( );
    bool ret = ( _cpq.empty() == false );
    _cpq_mx.unlock( );
    return ret;
}

bool VsnetUDPSocket::lower_selected( int datalen )
{
    int       ret = 0;
    socklen_t len1;
    AddressIP from;
    size_t    lentoread = _negotiated_max_size;
    if (datalen!=-1)
        lentoread = datalen;

    // In UDP mode, always receive data on sock
    len1 = sizeof(sockaddr_in);
    ret = recvfrom( get_fd(), _recv_buf, lentoread,
                    0, (sockaddr*)(sockaddr_in*)&from, &len1 );
    if( ret < 0 )
    {
        COUT << " fd=" << get_fd() << " error receiving: "
             << vsnetLastError() << endl;
    }
    else if( ret == 0 )
    {
        COUT << " Received " << ret << " bytes : " << _recv_buf
             << " (UDP socket closed, strange)" << endl;
    }
    else
    {
//	    COUT << "NETUI : Recvd " << ret << " bytes" << " <- " << from << endl;
        Pending mem( _recv_buf, ret, from );
        _cpq_mx.lock( );
        _cpq.push( mem );
        _cpq_mx.unlock( );
        _set.add_pending( get_fd() );
		return true;
    }
	return false;
}

