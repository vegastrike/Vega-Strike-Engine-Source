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

void VsnetUDPSocket::ack( )
{
    /* as soon as windows have been introduced, these ACKs will get meaning again */
}

int VsnetUDPSocket::recvbuf( void *buffer, unsigned int& len, AddressIP* from)
{
    COUT << " enter " << __PRETTY_FUNCTION__ << " with buffer " << buffer
         << " len=" << len << endl;

    int       ret = 0;

    socklen_t len1;

    AddressIP dummy;
    if( from == NULL ) from = &dummy;

    // In UDP mode, always receive data on sock
    len1 = sizeof(sockaddr_in);
    ret = recvfrom( _fd, (char *)buffer, len, 0, (sockaddr*)(sockaddr_in*)from, &len1 );
    if( ret < 0 )
    {
        COUT << " fd=" << _fd << " error receiving: " << vsnetLastError() << endl;
        ret = -1;
    }
    else if( ret == 0 )
    {
        COUT << " Received " << ret << " bytes : " << buffer << " (UDP socket closed, strange)" << endl;
        ret = -1;
    }
    else
    {
        len = ret;
	COUT << "NETUI : Recvd " << len << " bytes" << " <- " << *from << endl;
    }
    return ret;
}

int VsnetUDPSocket::recvbuf( PacketMem& buffer, AddressIP* from)
{
    char         buf[MAXBUFFER];
    unsigned int len = MAXBUFFER;
    int          ret = 0;

    ret = this->VsnetUDPSocket::recvbuf( buf, len, from );
    if( ret > 0 )
    {
        buffer.set( buf, len, PacketMem::LeaveOwnership );
    }

    return ret;
}

void VsnetUDPSocket::disconnect( const char *s, bool fexit )
{
    if( fexit )
        exit(1);
}

void VsnetUDPSocket::dump( std::ostream& ostr ) const
{
    ostr << "( s=" << _fd << " UDP r=" << _remote_ip << " )";
}

void VsnetUDPSocket::watch( SocketSet& set )
{
    set.setRead(_fd);
}

