#include <config.h>
#include <sstream>

#include "vsnet_pipe.h"

#if defined( _WIN32) && !defined(__CYGWIN__)

VSPipe::VSPipe( )
{
	SECURITY_ATTRIBUTES sec_attr; //don't know what it is for.
	sec_attr.nLength=sizeof(SECURITY_ATTRIBUTES);
	sec_attr.bInheritHandle=FALSE;
	sec_attr.lpSecurityDescriptor=NULL;
	BOOL isPipe=CreatePipe(&_readPipe,&_writePipe,&sec_attr,0);
	if (!isPipe)
        _failed = true;
    else
        _failed = false;
}

int VSPipe::write( const char* buf, int size )
{
	unsigned long numwritten;
	BOOL worked=WriteFile(_writePipe,(LPVOID)buf, size, &numwritten,NULL);
	if (!worked)
		return -1;
	if (numwritten>INT_MAX)
		numwritten=INT_MAX;
	return numwritten;
}

int VSPipe::read( char* buf, int size )
{
	unsigned long numread;
	BOOL worked=ReadFile(_readPipe,(LPVOID)buf, size, &numread,NULL);
	if (!worked)
		return -1;
	if (numread>INT_MAX)
		numread=INT_MAX;
	return numread;
}

int VSPipe::closewrite( )
{
	int retVal=0;
	BOOL worked=CloseHandle(_writePipe);
	if (!worked)
		retVal=-1;
	return retVal;
}

int VSPipe::closeread( )
{
	int retVal=0;
	BOOL worked=CloseHandle(_readPipe);
	if (!worked)
		retVal=-1;
	return retVal;
}

int VSPipe::getread( ) const
{
    return (int)_readPipe;
}

bool VSPipe::ok( ) const
{
    return !_failed;
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

int VSPipe::closewrite( )
{
    return ::close( _pipe[1] );
}

int VSPipe::closeread( )
{
    return ::close( _pipe[0] );
}

int VSPipe::write( const char* buf, int size )
{
    return ::write( _pipe[1], buf, size );
}

int VSPipe::read( char* buf, int size )
{
    return ::read( _pipe[0], buf, size );
}

int VSPipe::getread( ) const
{
    return _pipe[0];
}

bool VSPipe::ok( ) const
{
    return !_failed;
}

#endif

