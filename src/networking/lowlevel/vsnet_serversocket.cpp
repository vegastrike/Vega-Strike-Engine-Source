#include <config.h>
#include "vsnet_socket.h"
#include "vsnet_serversocket.h"
#include "vsnet_oss.h"
#include "vsnet_debug.h"
#include "vsnet_err.h"
using std::cout;
using std::cerr;
using std::endl;
using std::hex;

#include "vsnet_headers.h"

std::ostream& operator<<( std::ostream& ostr, const ServerSocket& s )
{
    ostr << "( s=" << s.get_fd() << " l=" << s._srv_ip << " )";
    return ostr;
}

bool operator==( const ServerSocket& l, const ServerSocket& r )
{
    return (l.get_fd() == r.get_fd());
}

void ServerSocket::child_disconnect( const char *s )
{
    if( get_fd() > 0 )
    {
        VsnetOSS::close_socket( get_fd() );
    }
    COUT << s << " :\tWarning: disconnected" << strerror(errno) << endl;
}

void ServerSocketTCP::lower_selected( )
{
    COUT << endl
         << endl
         << "------------------------------------------" << endl
         << "ServerSocketTCP for " << get_fd() << " selected" << endl
         << "------------------------------------------" << endl
         << endl
         << endl;

    struct sockaddr_in remote_ip;
#if defined (__APPLE__) || defined (_WIN32) 
    int
#else
    socklen_t          
#endif
    len = sizeof( struct sockaddr_in );
    int sock = ::accept( get_fd(), (sockaddr *)&remote_ip, &len );
    if( sock > 0 )
    {
        COUT << "accepted new sock " << sock  << endl;
        SOCKETALT newsock( sock, SOCKETALT::TCP, remote_ip, _set );
        _ac_mx.lock( );
	    _accepted_connections.push( newsock );
        _ac_mx.unlock( );
        _set.add_pending( get_fd() );
    }
    else
    {
        COUT << "accept failed" << endl;
        COUT << "Error accepting new conn: " << vsnetLastError() << endl;
    }
}

ServerSocketTCP::ServerSocketTCP( int fd, const AddressIP& adr, SocketSet& set )
    : ServerSocket( fd, adr, "ServerSocketTCP", set )
{ }

bool ServerSocketTCP::isActive( )
{
    _ac_mx.lock( );
    bool ret = ( _accepted_connections.empty() == false );
    _ac_mx.unlock( );
    return ret;
}

SOCKETALT ServerSocketTCP::acceptNewConn( )
{
    _ac_mx.lock( );
    if( !_accepted_connections.empty() )
    {
        COUT << "A connection has been accepted" << endl;
        SOCKETALT ret( _accepted_connections.front() );
        _accepted_connections.pop();
        _ac_mx.unlock( );
        return ret;
    }
    else
    {
        COUT << "No accepted TCP connection" << endl;
        _ac_mx.unlock( );
        _set.rem_pending( get_fd() );
        SOCKETALT ret;
        return ret;
    }
}

ServerSocketUDP::ServerSocketUDP( int fd, const AddressIP& adr, SocketSet& set )
    : ServerSocket( fd, adr, "ServerSocketUDP", set )
{ }

bool ServerSocketUDP::isActive( )
{
    return false;
}

SOCKETALT ServerSocketUDP::acceptNewConn( )
{
    SOCKETALT ret( get_fd(), SOCKETALT::UDP, _srv_ip, _set );
    return ret;
}

