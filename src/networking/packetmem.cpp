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
    _buffer  = NULL;
    _len     = 0;
    _cnt     = NULL;
}

PacketMem::PacketMem( const PacketMem& orig )
{
    _buffer  = orig._buffer;
    _len     = orig._len;
    _cnt     = orig._cnt;

    if( _cnt ) ref();
}

PacketMem::PacketMem( size_t bytesize )
{
    _buffer  = new char[bytesize];
    _len     = bytesize;
    _cnt     = new size_t;
    *_cnt    = 1;
}

PacketMem::PacketMem( const void* buffer, size_t size )
{
    if( buffer != NULL )
    {
        _buffer  = new char[size];
        _len     = size;
        _cnt     = new size_t;
        *_cnt    = 1;

        memcpy( _buffer, buffer, size );
    }
    else
    {
        _buffer  = NULL;
        _len     = 0;
        _cnt     = NULL;
    }
}

PacketMem::PacketMem( void* buffer, size_t size, bool own )
{
    inner_set( buffer, size, own );
}

void PacketMem::set( void* buffer, size_t size, bool own )
{
    release( );
    inner_set( buffer, size, own );
}

PacketMem::~PacketMem( )
{
    release( );
}

PacketMem& PacketMem::operator=( const PacketMem& orig )
{
    release( );

    _buffer  = orig._buffer;
    _len     = orig._len;
    _cnt     = orig._cnt;

    if( _cnt ) ref( );

    return *this;
}

void PacketMem::release( )
{
    if( _cnt == NULL ) return;

    unref( );
    if( *_cnt != 0 ) return;

    delete _cnt;
    if( _buffer == NULL ) return;

    delete [] _buffer;
}

void PacketMem::inner_set( void* buffer, size_t size, bool own )
{
    if( buffer != NULL )
    {
        if( own == false )
        {
            _buffer  = new char[size];
            _len     = size;
            _cnt     = new size_t;
            *_cnt    = 1;

            memcpy( _buffer, buffer, size );
        }
        else
        {
            _buffer  = (char*)buffer;
            _len     = size;
            _cnt     = new size_t;
            *_cnt    = 1;
        }
    }
    else
    {
        _buffer  = NULL;
        _len     = 0;
        _cnt     = NULL;
    }
}

void PacketMem::dump( ostream& ostr, size_t indent_depth ) const
{
    static const size_t LEN = 15;

    char x[LEN];
    char* buf  = _buffer;
    size_t len = _len;
    size_t i=0;
    while( len > 0 )
    {
        for( i=0; i<indent_depth; i++ ) ostr << " ";
        memset( x, -1, LEN );
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
            if( isprint(x[i]) ) ostr << x[i]; else ostr << "@";
        }
        ostr << endl;
    }
    ostr << std::dec;
}

bool PacketMem::operator==( const PacketMem& r ) const
{
    if( _buffer == r._buffer ) return true;
    if( _len != r._len ) return false;
    if( memcmp( _buffer, r._buffer, _len ) == 0 ) return true;
    return false;
}

size_t PacketMem::len() const
{
    return _len;
}

void PacketMem::trunc( size_t len )
{
    if( len >= 0 && len < _len ) _len = len;
}

const char* PacketMem::getConstBuf() const
{
    return _buffer;
}

char* PacketMem::getVarBuf()
{
    return _buffer;
}

void PacketMem::ref( )
{
    assert( _cnt );
    (*_cnt)++;
}

void PacketMem::unref( )
{
    assert( _cnt );
    assert( *_cnt > 0 );
    (*_cnt)--;
}

/***********************************************************************
 * PacketMemShadow - definition
 ***********************************************************************/

PacketMemShadow::PacketMemShadow( )
    : _idx( 0 )
    , _len( 0 )
{ }

PacketMemShadow::PacketMemShadow( const PacketMemShadow& orig )
    : _mem( orig._mem )
    , _idx( orig._idx )
    , _len( orig._len )
{ }

PacketMemShadow::PacketMemShadow( const PacketMem& mem )
    : _mem( mem )
    , _idx( 0 )
    , _len( mem.len() )
{ }

PacketMemShadow::PacketMemShadow( const PacketMem& mem, size_t idx size_t len )
    : _mem( mem )
    , _idx( idx )
    , _len( len )
{
    if( _idx + _len > _mem.len() )
    {
        if( _idx > _mem.len() )
            _len = 0;
        else
            _len = _mem.len() - _idx;
    }
}

PacketMemShadow& PacketMemShadow::operator=( const PacketMemShadow& orig )
{
    _mem = orig._mem;
    _idx = orig._idx;
    _len = orig._len;
    return *this;
}

size_t PacketMemShadow::len() const
{
    return _len;
}

char* PacketMemShadow::getVarBuf( )
{
    char* r = _mem.getVarBuf( );
    return &r[_idx];
}

const char* PacketMemShadow::getConstBuf( ) const
{
    const char* r = _mem.getConstBuf( );
    return &r[_idx];
}

