#ifndef VSNET_PIPE_H
#define VSNET_PIPE_H

#ifdef _WIN32
#include <winsock.h>
#include <windows.h>
#endif


class VSPipe
{
public:
    VSPipe( );
    int  write( const char* buf, int size );
    int  read( char* buf, int size );
    int  closewrite( );
    int  closeread( );
    int  getread( ) const;
    bool ok( ) const;

private:
    bool   _failed;
#ifdef _WIN32
    HANDLE _readPipe;
    HANDLE _writePipe;
#else
    int  _pipe[2];
#endif
};

#endif /* VSNET_PIPE_H */

