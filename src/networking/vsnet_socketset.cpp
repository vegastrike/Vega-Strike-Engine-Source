#include <config.h>

#include "vsnet_socketset.h"
#include "const.h"

SocketSet::SocketSet( )
{
    clear();
}

void SocketSet::setRead( int fd )
{
    FD_SET( fd, &_read_set_select );
    if( fd >= _max_sock_select ) _max_sock_select = fd+1;
}

void SocketSet::setReadAlwaysTrue( int fd )
{
    FD_SET( fd, &_read_set_always_true );
    if( fd >= _max_sock_always_true ) _max_sock_always_true = fd+1;
}

bool SocketSet::is_set( int fd ) const
{
    COUT << "adding " << fd << " to set" << std::endl;
    return ( FD_ISSET( fd, &_read_set_select ) ||
             FD_ISSET( fd, &_read_set_always_true ) );
}

bool SocketSet::is_setRead( int fd ) const
{
    return ( FD_ISSET( fd, &_read_set_select ) );
}

void SocketSet::clear( )
{
    FD_ZERO( &_read_set_select );
    _max_sock_select = 0;
    FD_ZERO( &_read_set_always_true );
    _max_sock_always_true = -1;
}

int SocketSet::select( timeval* timeout )
{
#ifdef VSNET_DEBUG
    COUT << "enter " << __PRETTY_FUNCTION__ << " fds=";
    for( int i=0; i<_max_sock_select; i++ )
    {
        if( FD_ISSET(i,&_read_set_select) ) std::cout << i << " ";
    }
    if( timeout )
        std::cout << " t=" << timeout->tv_sec << ":"
                  << timeout->tv_usec << std::endl;
    else
        std::cout << " t=NULL (blocking)" << std::endl;
#endif

    if( _max_sock_always_true > 0 )
    {
        /* There is pending data in the read queue. We must not wait until
         * more data arrives. It is dumb that dontwait need reinitialization
         * but the Linux version of select requires it.
         */
        static timeval dontwait;
        dontwait.tv_sec  = 0;
        dontwait.tv_usec = 0;
	    timeout = &dontwait;
    }

    int ret = ::select( _max_sock_select, &_read_set_select, 0, 0, timeout );
    if( ret == -1 )
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
        if( WSAGetLastError()!=WSAEINVAL)
            COUT<<"WIN32 error : "<<WSAGetLastError()<<std::endl;
#else
        perror( "Select failed : ");
#endif
    }
    return ret;
}

