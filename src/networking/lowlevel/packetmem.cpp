#include <config.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#include <string.h>
#include <iomanip>

#include "packetmem.h"

using std::ostream;
using std::endl;

LOCALCONST_DEF(PacketMem,bool,TakeOwnership,1)
LOCALCONST_DEF(PacketMem,bool,LeaveOwnership,0)

/***********************************************************************
 * PacketMem - definition
 ***********************************************************************/

PacketMem::PacketMem( )
{
    MAKE_VALID
    _len     = 0;
}

PacketMem::PacketMem( const PacketMem& orig )
{
    MAKE_VALID
    _buffer  = orig._buffer;
    _len     = orig._len;
}

PacketMem::PacketMem( size_t bytesize )
{
    MAKE_VALID
    _buffer  = ptr( new char[bytesize] );
    _len     = bytesize;
}

PacketMem::PacketMem( const void* buffer, size_t size )
{
    MAKE_VALID
    if( buffer != NULL )
    {
        char* v  = new char[size];
        _len     = size;
        memcpy( v, buffer, size );
        _buffer  = ptr( v );
    }
    else
    {
        _len     = 0;
    }
}

PacketMem::PacketMem( void* buffer, size_t size, bool own )
{
    MAKE_VALID
    inner_set( buffer, size, own );
}

PacketMem::~PacketMem( )
{
    CHECK_VALID
    MAKE_INVALID
}

void PacketMem::set( void* buffer, size_t size, bool own )
{
    CHECK_VALID
    inner_set( buffer, size, own );
}

PacketMem& PacketMem::operator=( const PacketMem& orig )
{
    CHECK_VALID
    _buffer  = orig._buffer;
    _len     = orig._len;
    return *this;
}

void PacketMem::inner_set( void* buffer, size_t size, bool own )
{
    CHECK_VALID
    if( buffer != NULL )
    {
        if( own == false )
        {
            char* v  = new char[size];
            _len     = size;
            memcpy( v, buffer, size );
            _buffer = ptr( v );
        }
        else
        {
            _buffer  = ptr( (char*)buffer );
            _len     = size;
        }
    }
    else
    {
        _buffer.reset( 0 );
        _len = 0;
    }
}

void PacketMem::dump( ostream& ostr, size_t indent_depth ) const
{
    CHECK_VALID
    static const size_t LEN = 15;

    char x[LEN];
    char* buf  = _buffer.get();
    size_t len = _len;
    size_t i=0;
    while( len > 0 )
    {
        for( i=0; i<indent_depth; i++ ) ostr << " ";
        memset( x, 0, LEN );
        for( i=0; i<LEN && len>0; i++ )
        {
            x[i] = *buf;
            buf++;
            len--;
        }
        for( i=0; i<LEN; i++ )
        {
            ostr << " " << std::setw(2) << std::hex << (((unsigned int)x[i])&0xff);
        }
        ostr << "   ";
        for( i=0; i<LEN; i++ )
        {
	    int v = x[i];
            if( isprint(v) ) ostr << x[i]; else ostr << "@";
        }
        ostr << endl;
    }
    ostr << std::dec;
}

bool PacketMem::operator==( const PacketMem& r ) const
{
    CHECK_VALID
    if( _buffer == r._buffer ) return true;
    if( _len != r._len ) return false;
    if( memcmp( _buffer.get(), r._buffer.get(), _len ) == 0 ) return true;
    return false;
}

size_t PacketMem::len() const
{
    CHECK_VALID
    return _len;
}

const char* PacketMem::getConstBuf() const
{
    CHECK_VALID
    return _buffer.get();
}

char* PacketMem::getVarBuf()
{
    CHECK_VALID
    return _buffer.get();
}

