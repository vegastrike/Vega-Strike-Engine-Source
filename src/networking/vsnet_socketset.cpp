#include <config.h>
#include <pthread.h>

#include "vsnet_socketset.h"
#include "const.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
#else
  #ifndef SOCKET_ERROR
  #define SOCKET_ERROR -1
  #endif
#endif

int SocketSet::select( timeval* timeout )
{
	COUT << "t=" << pthread_self() << " enter " << __PRETTY_FUNCTION__
	     << " fds=";
	for( int i=0; i<_max_sock; i++ )
	{
	    if( FD_ISSET(i,&_set) ) std::cout << i << " ";
	}
	if( timeout )
	    std::cout << " t=" << timeout->tv_sec << ":" << timeout->tv_usec << std::endl;
        else
            std::cout << " t=NULL" << std::endl;

	int ret = ::select( _max_sock, &_set, 0, 0, timeout );
	if( ret == SOCKET_ERROR )
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

