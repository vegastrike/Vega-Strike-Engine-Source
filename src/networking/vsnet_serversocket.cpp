#include <config.h>

#include "vsnet_serversocket.h"

#include "const.h"
#include "vsnet_err.h"
using std::cout;
using std::cerr;
using std::endl;
using std::hex;

#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#include <winsock.h>
#else
    #include <sys/ioctl.h>
#endif

std::ostream& operator<<( std::ostream& ostr, const ServerSocket& s )
{
    ostr << "( s=" << s._fd << " l=" << s._srv_ip << " )";
    return ostr;
}

bool operator==( const ServerSocket& l, const ServerSocket& r )
{
    return (l._fd == r._fd);
}

void ServerSocket::child_disconnect( const char *s )
{
    if( _fd > 0 )
    {
        close_socket( _fd );
    }
    COUT << s << " :\tWarning: disconnected" << strerror(errno) << endl;
}

void ServerSocketTCP::lower_selected( )
{
    COUT << endl
         << endl
         << "------------------------------------------" << endl
         << "ServerSocketTCP for " << _fd << " selected" << endl
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
    int sock = ::accept( _fd, (sockaddr *)&remote_ip, &len );
    if( sock > 0 )
    {
        COUT << "accepted" << endl;
        SOCKETALT newsock( sock, SOCKETALT::TCP, remote_ip, _set );
	    _accepted_connections.push( newsock );
    }
    else
    {
        COUT << "accept failed" << endl;
        COUT << "Error accepting new conn: " << vsnetLastError() << endl;
    }
}

ServerSocketTCP::ServerSocketTCP( int fd, const AddressIP& adr, SocketSet& set )
    : ServerSocket( fd, adr, set )
{ }

SOCKETALT ServerSocketTCP::acceptNewConn( )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    if( !_accepted_connections.empty() )
    {
        SOCKETALT ret( _accepted_connections.front() );
        _accepted_connections.pop();
        return ret;
    }
    else
    {
        SOCKETALT ret;
        return ret;
    }
}

ServerSocketUDP::ServerSocketUDP( int fd, const AddressIP& adr, SocketSet& set )
    : ServerSocket( fd, adr, set )
{ }

SOCKETALT ServerSocketUDP::acceptNewConn( )
{
    SOCKETALT ret( _fd, SOCKETALT::UDP, _srv_ip, _set );
    return ret;
}

