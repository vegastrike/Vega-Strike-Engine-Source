#ifndef VSNET_PIPE_H
#define VSNET_PIPE_H

#include "vsnet_headers.h"


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
    bool _failed;
    int  _pipe[2];
};

#endif /* VSNET_PIPE_H */

