#include <config.h>

#include "vsnet_serversocket.h"

#include "const.h"
using std::cout;
using std::cerr;
using std::endl;
using std::hex;

#if defined(_WIN32) && !defined(__CYGWIN__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	#include <winsock.h>
#else
	//#warning "GCC platform"
	#define SOCKET_ERROR -1
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
#if defined(_WIN32) && !defined(__CYGWIN__)
        closesocket( _fd );
#else
        close( _fd );
#endif
    }
    cout << __FILE__ << ":" << __LINE__ << " "
         << s << " :\tWarning: disconnected" << strerror(errno) << endl;
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
    cout << __FILE__ << ":" << __LINE__ << " enter " << __PRETTY_FUNCTION__ << endl;
    set.set( _fd );
}

SOCKETALT ServerSocketTCP::acceptNewConn( SocketSet& set )
{
    cout << __FILE__ << ":" << __LINE__ << " enter " << __PRETTY_FUNCTION__ << endl;

    struct sockaddr_in remote_ip;
#ifdef __APPLE__
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
#if defined(_WIN32) && !defined(__CYGWIN__)
            cout<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
            perror("Error accepting new conn\n");
#endif
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

SOCKETALT ServerSocketUDP::acceptNewConn( SocketSet& /*set*/ )
{
    SOCKETALT ret( _fd, SOCKETALT::UDP, _srv_ip );
    return ret;
}

