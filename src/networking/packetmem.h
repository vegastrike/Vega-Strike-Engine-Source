#ifndef PACKETMEM_H
#define PACKETMEM_H

#include <sys/types.h>
#include <iostream>

#include "const.h"

/***********************************************************************
 * PacketMem - declaration
 ***********************************************************************/

class PacketMem
{
public:
    LOCALCONST_DECL(bool,TakeOwnership,1)
    LOCALCONST_DECL(bool,LeaveOwnership,0)

private:
    char*   _buffer;
    size_t  _len;
    size_t* _cnt;      // delete or return to pool when all references are gone

public:
    PacketMem( );
    PacketMem( const PacketMem& );
    PacketMem( size_t bytesize );

    /// this constructor allows only copy-in of the buffer
    PacketMem( const void* buffer, size_t size );

    /** this constructor makes it possible to copy-in the buffer, but also to
     *  acquire only a temporary reference
     */
    PacketMem( void* buffer, size_t size, bool own );

    ~PacketMem( );

    void set( void* buffer, size_t size, bool own );

    size_t      len() const;
    char*       getVarBuf( );
    const char* getConstBuf( ) const;

    /** Performs if(len<_len) _len=len;
     *  Dangerous, use with care.
     *  This function does not touch the buffer itself, but it fakes a shorter
     *  buffer than actually present. Existing copies will not share the fake
     *  length value but new copies will.
     */
    void trunc( size_t len );
    
    /** Dump the content of this buffer onto the given ostream, identing
     *  every line by indent_depth spaces.
     */
    void dump( std::ostream& ostr, size_t indent_depth ) const;

    PacketMem& operator=( const PacketMem& );

    bool operator==( const PacketMem& orig ) const;

private:
    void release( );
    void inner_set( void* buffer, size_t size, bool own );
    void ref( );
    void unref( );
};

/***********************************************************************
 * PacketMemShadow - declaration
 ***********************************************************************/

class PacketMemShadow
{
    PacketMem _mem;
    size_t    _idx;    /* 0-based offset */
    size_t    _len;    /* fake length in bytes */
public:
    PacketMemShadow( );
    PacketMemShadow( const PacketMemShadow& orig );
    PacketMemShadow( const PacketMem& mem );
    PacketMemShadow( const PacketMem& mem, size_t idx size_t len );

    PacketMemShadow& operator=( const PacketMemShadow& orig );

    size_t      len() const;
    char*       getVarBuf( );
    const char* getConstBuf( ) const;
};

#endif

