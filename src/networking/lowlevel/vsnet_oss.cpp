#ifndef VSNET_OSS_CPP
#define VSNET_OSS_CPP

#include <config.h>

#include "vsnet_headers.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#endif
#include <iostream>

#include "vsnet_oss.h"

using namespace std;

namespace VsnetOSS
{

INLINE int close_socket( int fd )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
	return ::closesocket( fd );
#else
	return ::close( fd );
#endif
}

INLINE int inet_aton( const char *host, struct in_addr *inp )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    inp->s_addr = ::inet_addr( host );
    if( inp->s_addr == INADDR_NONE ) return 0;
    return 1;
#else
    return ::inet_aton( host, inp );
#endif
}

INLINE int socket(int domain, int type, int protocol)
{
    int ret = ::socket( domain, type, protocol );
#if defined(_WIN32) && !defined(__CYGWIN__)
    if( ret == INVALID_SOCKET ) return -1;
#else
    if( ret < 0 ) return -1;
#endif
    return ret;
}

INLINE int recv(int fd, void* buf, unsigned int len, int flags )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    int ret = ::recv( fd, (char*)buf, len, flags );
#else
    int ret = ::recv( fd, buf, len, flags );
#endif
    return ret;
}

INLINE void memcpy( void* dest, const void* src, int bytesize )
{
    /* If your memcpy needs a (char* src), make ifdefs and a typecast
     * here.
     */
    ::memcpy( dest, src, bytesize );
}

};

#endif /* VSNET_OSS_CPP */

