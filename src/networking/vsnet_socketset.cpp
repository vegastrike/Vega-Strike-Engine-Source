#include <config.h>
#include <sstream>

#ifdef _WIN32
#include <winsock.h>
#endif
#include "vsnet_socket.h"
#include "vsnet_socketset.h"
#include "const.h"

#ifdef _WIN32
#include <windows.h>
//std::multimap<int,HANDLE> pipeMap;

int close(int file) {
	int retVal=0;
	BOOL worked=CloseHandle((HANDLE)file);
	if (!worked)
		retVal=-1;
	return retVal;
}
int read(int file, void *buf, int size) {
	unsigned long numread;
	BOOL worked=ReadFile((HANDLE)file,(LPVOID)buf, size, &numread,NULL);
	if (!worked)
		return -1;
	if (numread>INT_MAX)
		numread=INT_MAX;
	return numread;
}
int write(int file, const void *buf, int size) {
	unsigned long numwritten;
	BOOL worked=WriteFile((HANDLE)file,(LPVOID)buf, size, &numwritten,NULL);
	if (!worked)
		return -1;
	if (numwritten>INT_MAX)
		numwritten=INT_MAX;
	return numwritten;
}
int pipe(int *file) {
	HANDLE readPipe,writePipe;
	SECURITY_ATTRIBUTES sec_attr; //don't know what it is for.
	sec_attr.nLength=sizeof(SECURITY_ATTRIBUTES);
	sec_attr.bInheritHandle=FALSE;
	sec_attr.lpSecurityDescriptor=NULL;
	BOOL isPipe=CreatePipe(&readPipe,&writePipe,&sec_attr,0);
	if (!isPipe)
		return -1;
	file[0]=(int)readPipe;
	file[1]=(int)writePipe;
	return 0;
}
#endif

using namespace std;

SocketSet::SocketSet( bool blockmainthread )
    : VSThread( false )
    , _blockmain( blockmainthread )
    , _blockmain_pending( 0 )
{
    ::pipe( _thread_wakeup );
    _thread_end = false;
}

SocketSet::~SocketSet( )
{
    _thread_mx.lock( );
    _thread_end = true;
    _blockmain  = false; // signalling would be dangerous
    ::close( _thread_wakeup[1] );
    _thread_cond.wait( _thread_mx );
    ::close( _thread_wakeup[0] );
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
    ostr << _thread_wakeup[0] << "(w)";
    if( timeout )
        ostr << " t=" << timeout->tv_sec << ":" << timeout->tv_usec;
    else
        ostr << " t=NULL (blocking)";
    ostr << ends;
    if( !timeout || timeout->tv_sec >= 1 ) COUT << ostr.str() << endl;
#endif

    FD_SET( _thread_wakeup[0], &read_set_select );
    if( _thread_wakeup[0] > max_sock_select )
        max_sock_select = _thread_wakeup[0] + 1;

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

        if( FD_ISSET( _thread_wakeup[0], &read_set_select ) )
        {
#ifdef VSNET_DEBUG
            ostr << _thread_wakeup[0] << "(w)";
#endif
            char c;
            read( _thread_wakeup[0], &c, 1 );
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
    write( _thread_wakeup[1], &c, 1 );
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

