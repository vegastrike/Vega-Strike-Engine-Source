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
#endif

std::ostream& operator<<( std::ostream& ostr, const ServerSocket& s )
{
    ostr << "( s=" << s._fd
	 << " l=" << s._srv_ip << " )";
    return ostr;
}

bool operator==( const ServerSocket& l, const ServerSocket& r )
{
    return (l._fd == r._fd);
}

void ServerSocket::disconnect( const char *s, bool fexit )
{
    if( _fd > 0 )
    {
        close_socket( _fd );
    }
    COUT << s << " :\tWarning: disconnected" << strerror(errno) << endl;
    if( fexit )
        exit(1);
}

/**************************************************************/
/**** Accept a new connection                              ****/
/**************************************************************/
// Returns the connection socket or 0 if failed or if no activity
// Not used in standard UDP mode
// Returns channel number in SDL UDP mode

void ServerSocketTCP::watchForNewConn( SocketSet& set, int )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
    set.setRead( _fd );
}

SOCKETALT ServerSocketTCP::acceptNewConn( SocketSet& set )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;

    struct sockaddr_in remote_ip;
#if defined (__APPLE__) || defined (_WIN32) 
    int
#else
    socklen_t          
#endif
      len = sizeof( struct sockaddr_in );
    if( set.is_set( _fd ) )
    {
        int sock = accept( _fd, (sockaddr *)&remote_ip, &len );
        if( sock > 0 )
        {
            SOCKETALT ret( sock, SOCKETALT::TCP, remote_ip );
	    return ret;
        }
        else
        {
            COUT << "Error accepting new conn: " << vsnetLastError() << endl;
            SOCKETALT ret( 0, SOCKETALT::TCP, remote_ip );
	    return ret;
        }
    }
    else
    {
        SOCKETALT ret( 0, SOCKETALT::TCP, _srv_ip );
	return ret;
    }
}

/**************************************************************/
/**** Accept a new connection                              ****/
/**************************************************************/
// Returns the connection socket or 0 if failed or if no activity
// Not used in standard UDP mode
// Returns channel number in SDL UDP mode

void ServerSocketUDP::watchForNewConn( SocketSet& , int )
{
}

SOCKETALT ServerSocketUDP::acceptNewConn( SocketSet& )
{
    SOCKETALT ret( _fd, SOCKETALT::UDP, _srv_ip );
    return ret;
}

