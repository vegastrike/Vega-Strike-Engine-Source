#include <config.h>
#include <sstream>

#include "vsnet_pipe.h"
#include "vsnet_socket.h"

#if defined( _WIN32) && !defined(__CYGWIN__)

VSPipe::VSPipe( )
    : _failed( false )
{
    int                retval;
    struct sockaddr_in addr;

    _pipe[0] = ::socket( PF_INET, SOCK_DGRAM, 0 );
    if( _pipe[0] == INVALID_SOCKET )
    {
        _failed = true;
        return;
    }

    memset( &addr, 0, sizeof(struct sockaddr_in) );
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = 0;
    addr.sin_family      = AF_INET;
    retval = ::bind( _pipe[0], (sockaddr *)&addr, sizeof(addr) );
    if( retval == SOCKET_ERROR )
    {
        close_socket( _pipe[0] );
        _failed = true;
        return;
    }

    socklen_t addrlen = sizeof(addr);
    retval = ::getsockname( _pipe[0], (sockaddr *)&addr, &addrlen );
    if( retval == SOCKET_ERROR )
    {
        close_socket( _pipe[0] );
        _failed = true;
        return;
    }

    _pipe[1] = ::socket( PF_INET, SOCK_DGRAM, 0 );
    if( _pipe[1] == INVALID_SOCKET )
    {
        close_socket( _pipe[1] );
        _failed = true;
        return;
    }

    retval = ::connect( _pipe[1], (sockaddr *)&addr, addrlen );
    if( retval == SOCKET_ERROR )
    {
        close_socket( _pipe[1] );
        close_socket( _pipe[0] );
        _failed = true;
        return;
    }
}

int VSPipe::write( const char* buf, int size )
{
    return ::send( _pipe[1], buf, size, 0 );
}

int VSPipe::read( char* buf, int size )
{
    return ::recv( _pipe[0], buf, size, 0 );
}

#else
#include <stdio.h>
#include <unistd.h>

VSPipe::VSPipe( )
{
    int ret = pipe( _pipe );
    if( ret != 0 )
        _failed = true;
    else
        _failed = false;
}

int VSPipe::write( const char* buf, int size )
{
    return ::write( _pipe[1], buf, size );
}

int VSPipe::read( char* buf, int size )
{
    return ::read( _pipe[0], buf, size );
}

#endif

int VSPipe::closewrite( )
{
    return ::close_socket( _pipe[1] );
}

int VSPipe::closeread( )
{
    return ::close_socket( _pipe[0] );
}

int VSPipe::getread( ) const
{
    return _pipe[0];
}

bool VSPipe::ok( ) const
{
    return !_failed;
}

