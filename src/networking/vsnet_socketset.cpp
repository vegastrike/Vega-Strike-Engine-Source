#include <config.h>
#include <sstream>

#include "vsnet_socket.h"
#include "vsnet_socketset.h"
#include "const.h"

using namespace std;

SocketSet::SocketSet( )
{
}

void SocketSet::set( VsnetSocketBase* s )
{
    _autoset.insert( s );
}

void SocketSet::unset( VsnetSocketBase* s )
{
    _autoset.erase( s );
}

bool SocketSet::is_set( int fd ) const
{
#ifdef VSNET_DEBUG
    COUT << "adding " << fd << " to set" << endl;
#endif
    if( FD_ISSET( fd, &_read_set_select ) ) return true;
    return false;
}

// bool SocketSet::is_setRead( int fd ) const
// {
//     return ( FD_ISSET( fd, &_read_set_select ) );
// }

int SocketSet::select( long sec, long usec )
{
    timeval tv;
    tv.tv_sec  = sec;
    tv.tv_usec = usec;
    return select( &tv );
}

int SocketSet::select( timeval* timeout )
{
    bool   have_always_true = false;
    fd_set always_set;
    int    always_set_max;

    FD_ZERO( &_read_set_select );
    _max_sock_select = 0;
    FD_ZERO( &always_set );
    always_set_max = 0;

#ifdef VSNET_DEBUG
    std::ostringstream ostr;
    ostr << "calling select with fds=";
#endif
    for( Set::iterator it = _autoset.begin(); it != _autoset.end(); it++ )
    {
        int fd = (*it)->get_fd();
        if( fd >= 0 )
        {
#ifdef VSNET_DEBUG
            ostr << fd << " ";
#endif
            FD_SET( fd, &_read_set_select );
            if( fd >= _max_sock_select ) _max_sock_select = fd+1;

            if( have_always_true==false && (*it)->needReadAlwaysTrue() )
            {
                have_always_true = true;
                FD_SET( fd, &always_set );
                if( fd >= always_set_max ) always_set_max = fd + 1;
            }
        }
    }

    if( have_always_true )
    {
        /* There is pending data in the read queue. We must not wait until
         * more data arrives. It is dumb that dontwait needs reinitialization
         * but the Linux version of select requires it.
         */
        static timeval dontwait;
        dontwait.tv_sec  = 0;
        dontwait.tv_usec = 0;
	    timeout = &dontwait;
#ifdef VSNET_DEBUG
        ostr << " t=0:0 (packets pending)";
#endif
    }
#ifdef VSNET_DEBUG
    else
    {
        if( timeout )
            ostr << " t=" << timeout->tv_sec << ":"
                          << timeout->tv_usec;
        else
            ostr << " t=NULL (blocking)";
    }
    ostr << ends;
    COUT << ostr.str() << endl;
#endif

    int ret = ::select( _max_sock_select, &_read_set_select, 0, 0, timeout );
    if( ret == -1 )
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
        if( WSAGetLastError()!=WSAEINVAL)
            COUT<<"WIN32 error : "<<WSAGetLastError()<<endl;
#else
        perror( "Select failed : ");
#endif
    }
    else if( ret == 0 )
    {
    }
    else
    {
#ifdef VSNET_DEBUG
        std::ostringstream ostr;
#endif
        for( Set::iterator it = _autoset.begin(); it != _autoset.end(); it++ )
        {
            int fd = (*it)->get_fd();
            if( fd >= 0 && FD_ISSET(fd,&_read_set_select) )
            {
#ifdef VSNET_DEBUG
                ostr << fd << " ";
#endif
                (*it)->lower_selected( );
            }
        }
#ifdef VSNET_DEBUG
        ostr << ends;
        COUT << "select saw activity on fds=" << ostr.str() << endl;
#endif
    }

    if( have_always_true )
    {
        for( int i=0; i<always_set_max; i++ )
        {
            if( FD_ISSET( i, &always_set ) )
            {
                FD_SET( i, &_read_set_select );
                ret++;
            }
        }
    }
    return ret;
}

