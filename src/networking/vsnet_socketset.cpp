#include <config.h>
#include <sstream>

#ifdef _WIN32
#include <winsock.h>
#endif
#include "vsnet_socket.h"
#include "vsnet_socketset.h"
#include "vsnet_pipe.h"
#include "const.h"

using namespace std;

SocketSet::SocketSet( bool blockmainthread )
    : VSThread( false )
    , _blockmain( blockmainthread )
    , _blockmain_pending( 0 )
{
    _thread_end = false;
}

SocketSet::~SocketSet( )
{
    _thread_mx.lock( );
    _thread_end = true;
    _blockmain  = false; // signalling would be dangerous
    _thread_wakeup.closewrite();
    _thread_cond.wait( _thread_mx );
    _thread_wakeup.closeread();
    _thread_mx.unlock( );
}

void SocketSet::set( VsnetSocketBase* s )
{
    _autoset.insert( s );
    private_wakeup( );
}

void SocketSet::unset( VsnetSocketBase* s )
{
    _autoset.erase( s );
    private_wakeup( );
}

#ifdef USE_NO_THREAD
void SocketSet::wait( )
{
    assert( _blockmain ); // can't call wait if we haven't ordered the feature
    if( _blockmain_pending == 0 )
    {
        private_select( NULL );
    }
    else
    {
        struct timeval tv;
        tv.tv_sec  = 0;
        tv.tv_usec = 0;
        private_select( &tv );
    }
}
#else
void SocketSet::wait( )
{
    assert( _blockmain ); // can't call wait if we haven't ordered the feature
    _blockmain_mx.lock( );
    if( _blockmain_pending == 0 )
    {
        _blockmain_cond.wait( _blockmain_mx );
    }
    _blockmain_mx.unlock( );
}
#endif

void SocketSet::dec_pending( )
{
    if( _blockmain )
    {
        _blockmain_mx.lock( );
        _blockmain_pending--;
        _blockmain_mx.unlock( );
    }
}

void SocketSet::inc_pending( )
{
    if( _blockmain )
    {
        _blockmain_mx.lock( );
        _blockmain_pending++;
        _blockmain_mx.unlock( );
    }
}

int SocketSet::private_select( timeval* timeout )
{
    fd_set read_set_select;
    int    max_sock_select = 0;

    FD_ZERO( &read_set_select );

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
            FD_SET( fd, &read_set_select );
            if( fd >= max_sock_select ) max_sock_select = fd+1;
        }
    }

#ifdef VSNET_DEBUG
    ostr << _thread_wakeup.getread() << "(w)";
    if( timeout )
        ostr << " t=" << timeout->tv_sec << ":" << timeout->tv_usec;
    else
        ostr << " t=NULL (blocking)";
    ostr << ends;
    if( !timeout || timeout->tv_sec >= 1 ) COUT << ostr.str() << endl;
#endif

    FD_SET( _thread_wakeup.getread(), &read_set_select );
    if( _thread_wakeup.getread() > max_sock_select )
        max_sock_select = _thread_wakeup.getread() + 1;

    int ret = ::select( max_sock_select, &read_set_select, 0, 0, timeout );

    if( _blockmain )
    {
        // whatever the reason for leaving select, if we have been asked
        // to signal the main thread on wakeup, we do it
        _blockmain_mx.lock( );
        _blockmain_cond.signal( );
        _blockmain_mx.unlock( );
    }

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
            if( fd >= 0 && FD_ISSET(fd,&read_set_select) )
            {
#ifdef VSNET_DEBUG
                ostr << fd << " ";
#endif
                (*it)->lower_selected( );
            }
        }

        if( FD_ISSET( _thread_wakeup.getread(), &read_set_select ) )
        {
#ifdef VSNET_DEBUG
            ostr << _thread_wakeup.getread() << "(w)";
#endif
            char c;
            _thread_wakeup.read( &c, 1 );
        }

#ifdef VSNET_DEBUG
        ostr << ends;
        COUT << "select saw activity on fds=" << ostr.str() << endl;
#endif
    }
    return ret;
}

void SocketSet::private_wakeup( )
{
#ifdef VSNET_DEBUG
    COUT << "calling wakeup" << endl;
#endif
    char c = 'w';
    _thread_wakeup.write( &c, 1 );
}

#ifdef USE_NO_THREAD
void SocketSet::waste_time( long sec, long usec )
{
    struct timeval tv;
    tv.tv_sec  = sec;
    tv.tv_usec = usec;
    private_select( &tv );
}
#else
void SocketSet::waste_time( long sec, long usec )
{
    struct timeval tv;
    tv.tv_sec  = sec;
    tv.tv_usec = usec;
    select( 0, NULL, NULL, NULL, &tv );
}
#endif

void SocketSet::run( )
{
    while( !_thread_end )
    {
        private_select( NULL );
    }
    _thread_mx.lock( );
    _thread_cond.signal( );
    _thread_mx.unlock( );
}

