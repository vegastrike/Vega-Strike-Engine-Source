#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
    //#warning "Win32 platform"
    #include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

#include <list>

#include "const.h"
#include "vsnet_socketudp.h"
#include "vsnet_err.h"

using namespace std;

using std::cout;
using std::cerr;
using std::endl;

/***********************************************************************
 * VsnetUDPSocket - definition
 ***********************************************************************/
 
VsnetUDPSocket::VsnetUDPSocket( int sock, const AddressIP& remote_ip, SocketSet& socketset )
    : VsnetSocket( sock, remote_ip, socketset )
{
    _negotiated_max_size = MAXBUFFER;
    _recv_buf            = new char[MAXBUFFER];
}

VsnetUDPSocket::~VsnetUDPSocket( )
{
    delete [] _recv_buf;
}

int VsnetUDPSocket::sendbuf( PacketMem& packet, const AddressIP* to)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    int numsent;

    // In UDP mode, always send on this->sock
    const sockaddr_in* dest = to;
    if( dest == NULL ) dest = &_remote_ip;

    assert( dest != NULL );

    if( (numsent = sendto( _fd, packet.getConstBuf(), packet.len(), 0, (sockaddr*) dest, sizeof(struct sockaddr_in)))<0)
    {
        COUT << "Error sending: " << vsnetLastError() << endl;
        return -1;
    }
    cout<<"Sent "<<numsent<<" bytes"<<" -> "<<inet_ntoa( dest->sin_addr)<<":"<<ntohs(dest->sin_port)<<endl;
    return numsent;
}

int VsnetUDPSocket::recvbuf( PacketMem& buffer, AddressIP* from)
{
    _cpq_mx.lock( );
    if( _cpq.empty() )
    {
        _cpq_mx.unlock( );
        return -1;
    }

    buffer = _cpq.front().mem;
    if( from )
        *from = _cpq.front().ip;
    _cpq.pop();
    _cpq_mx.unlock( );
    _set.dec_pending( );
    return buffer.len();
}

void VsnetUDPSocket::dump( std::ostream& ostr ) const
{
    ostr << "( s=" << _fd << " UDP r=" << _remote_ip << " )";
}

bool VsnetUDPSocket::isActive( )
{
    _cpq_mx.lock( );
    bool ret = ( _cpq.empty() == false );
    _cpq_mx.unlock( );
    return ret;
}

void VsnetUDPSocket::lower_selected( )
{
    int       ret = 0;
    socklen_t len1;
    AddressIP from;

    // In UDP mode, always receive data on sock
    len1 = sizeof(sockaddr_in);
    ret = recvfrom( _fd, _recv_buf, _negotiated_max_size,
                    0, (sockaddr*)(sockaddr_in*)&from, &len1 );
    if( ret < 0 )
    {
        COUT << " fd=" << _fd << " error receiving: "
             << vsnetLastError() << endl;
    }
    else if( ret == 0 )
    {
        COUT << " Received " << ret << " bytes : " << _recv_buf
             << " (UDP socket closed, strange)" << endl;
    }
    else
    {
	    COUT << "NETUI : Recvd " << ret << " bytes" << " <- " << from << endl;
        Pending mem( _recv_buf, ret, from );
        _cpq_mx.lock( );
        _cpq.push( mem );
        _cpq_mx.unlock( );
        _set.inc_pending( );
    }
}

