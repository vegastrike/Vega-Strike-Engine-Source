#include <config.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif
#include <math.h>

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif /* HAVE_ZLIB_H */

#include "packet.h"
#include "vsnet_debug.h"
#include "vsnet_oss.h"
#include "lin_time.h"

LOCALCONST_DEF(Packet,unsigned short,header_length,sizeof( struct Header))

#include <boost/version.hpp>
#if defined(_WIN32) && defined(_MSC_VER) && BOOST_VERSION!=102800 //wierd error in MSVC
#  define __LINE__NOMSC	0
#else
#  define __LINE__NOMSC	__LINE__ 
#endif

Packet::Packet()
{
    MAKE_VALID

    h.command     = 0;
    h.serial      = 0;
    h.timestamp   = 0;
    h.data_length = 0;
    h.flags       = NONE;
}

Packet::Packet( const void* buffer, size_t sz )
{
    MAKE_VALID

    if( sz >= header_length )
    {
	    h.ntoh( buffer );

        sz -= header_length;
        if( h.data_length > sz )
        {
            COUT << "Packet not correctly received, not enough data for buffer" << endl
	         << "    should be still " << h.data_length
                 << " but buffer has only " << sz << endl;
	        display( __FILE__, __LINE__NOMSC );
        }
	    else if( h.flags & COMPRESSED )
	    {
	        if( packet_uncompress( _packet,
	                               (const unsigned char*)buffer,
			                       h.data_length,
			                       h ) == false )
	        {
		        display( __FILE__, __LINE__NOMSC );
	        }
	    }
	    else
	    {
            PacketMem mem( buffer, sz );
            _packet = mem;

#ifdef VSNET_DEBUG
	        COUT << "Parsed a packet with"
	             << " cmd=" << Cmd(h.command) << "(" << (int)h.command << ")"
	             << " ser=" << h.serial
	             << " ts=" << h.timestamp
	             << " len=" << h.data_length
	             << endl;
#endif
	    }
    }
    else
    {
        COUT << "Packet not correctly received, not enough data for header" << endl;
    }
}

Packet::Packet( PacketMem& buffer )
{
    MAKE_VALID

    if( buffer.len() >= header_length )
    {
	    h.ntoh( buffer.getConstBuf() );
	    size_t sz = buffer.len();
        sz -= header_length;

        if( h.data_length > sz )
        {
            COUT << "Packet not correctly received, not enough data for buffer" << endl
	             << "    should be still " << h.data_length
                 << " but buffer has only " << sz << endl;
	        display( __FILE__, __LINE__NOMSC );
        }
	    else if( h.flags & COMPRESSED )
	    {
#ifdef HAVE_ZLIB_H
	        if( packet_uncompress( _packet,
	                               (const unsigned char*)buffer.getConstBuf(),
			                       h.data_length,
			                       h ) == false )
	        {
	            display( __FILE__, __LINE__NOMSC );
	        }
#else /* HAVE_ZLIB_H */
            COUT << "Received compressed packet, but compiled without zlib" << endl;
            display( __FILE__, __LINE__NOMSC );
#endif /* HAVE_ZLIB_H */
	    }
	    else
	    {
            _packet = buffer;

#ifdef VSNET_DEBUG
	        COUT << "Parsed a packet with"
	             << " cmd=" << Cmd(h.command) << "(" << (int)h.command << ")"
	             << " ser=" << h.serial
	             << " ts=" << h.timestamp
	             << " len=" << h.data_length
	             << endl;
#endif
	    }
    }
    else
    {
        COUT << "Packet not correctly received, not enough data for header" << endl;
    }
}

Packet::Packet( const Packet &a )
{
    MAKE_VALID

    copyfrom( a );
}

Packet::~Packet()
{
    CHECK_VALID
    MAKE_INVALID
}

void Packet::copyfrom( const Packet& a )
{
    CHECK_VALID
    h.command     = a.h.command;
    h.serial      = a.h.serial;
    h.timestamp   = a.h.timestamp;
    h.data_length = a.h.data_length;
    h.flags       = a.h.flags;
    _packet       = a._packet;
}

int Packet::send( Cmd cmd, ObjSerial nserial, const char * buf, unsigned int length,
                  int prio, const AddressIP* dst,
		          const SOCKETALT& sock, const char* caller_file,
		          int caller_line )
{
    CHECK_VALID
    create( cmd, nserial, buf, length, prio, caller_file, caller_line );
    return send( sock, dst );
}

void Packet::create( Cmd cmd, ObjSerial nserial,
                     const char * buf, unsigned int length,
                     int prio,
                     const char* caller_file, int caller_line )
{
    CHECK_VALID
    unsigned int microtime;

    // Get a timestamp for packet (used for interpolation on client side)
    double curtime = getNewTime();
    microtime = (unsigned int) (floor(curtime*1000));
    h.timestamp = microtime;

#ifdef VSNET_DEBUG
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
         << "    *** from " << caller_file << ":" << caller_line << endl
         << "    *** create " << cmd << " ser=" << nserial << ", " << length
         << "    *** curtime " << curtime
                 << " microtime " << microtime
                 << " timestamp " << h.timestamp << endl;
#else
    COUT << "*** create " << cmd << " ser=" << nserial << ", " << length << endl;
#endif

    h.command   = cmd;
    h.flags     = prio;

    // buf is an allocated char * containing message
    h.serial = nserial;
    
    bool packet_filled = false;

#ifdef HAVE_ZLIB_H
    if( prio & COMPRESSED )
    {
        size_t sz;   // complicated zlib rules for safety reasons
        sz = length + ( length/10 ) + 15 + header_length;

        char*          c      = new char[sz];
        unsigned long  clen_l = length;
        unsigned int   ulen_i;
        unsigned char* dest   = (unsigned char*)&c[header_length+sizeof(ulen_i)];
        int            zlib_errcode;

        zlib_errcode = ::compress2( dest, &clen_l, (unsigned char*)buf, length, 9 );

        if( zlib_errcode == Z_OK )
        {
            if( clen_l < length + 2 )
            {
                ulen_i = htonl( (unsigned int)length );
                VsnetOSS::memcpy( &c[header_length], &ulen_i, sizeof(ulen_i) );

                h.data_length = clen_l + sizeof(ulen_i);
                h.hton( c );

#ifdef VSNET_DEBUG
                COUT << "Created a compressed packet of length "
                     << h.data_length+header_length << " for sending" << endl;
#endif

                _packet.set( c, h.data_length+header_length, PacketMem::TakeOwnership );
                packet_filled = true;
            }
            else
            {
#ifdef VSNET_DEBUG
                COUT << "Compressing " << cmd
	                 << " packet refused - bigger than original" << std::endl;
#endif

                delete [] c;
                _packet = PacketMem( );
            }
        }
        else
        {
            delete [] c;
            _packet = PacketMem( );
        }
    }
#endif /* HAVE_ZLIB_H */

    if( packet_filled == false )
    {
        h.flags &= ( ~COMPRESSED );    // make sure that it's never set here
        h.data_length = length;

        char* c = new char[ length + header_length ];
        h.hton( c );
        VsnetOSS::memcpy( &c[header_length], buf, length );
        _packet.set( c, length + header_length, PacketMem::TakeOwnership );
#ifdef VSNET_DEBUG
        COUT << "Created a packet of length "
             << length+header_length << " for sending" << endl;
#endif
    }
}

void    Packet::display( const char* file, int line )
{
    CHECK_VALID
    cout << "*** " <<  file << ":" << line << " " << endl;
    cout << "*** Packet display -- Command : " << Cmd(h.command)
         << " - Serial : " << h.serial << " - Flags : " << h.flags << endl;
    cout<<"***                   Size   : " << getDataLength() + header_length << endl;
    cout<<"***                   Buffer : " << endl;
    _packet.dump( cout, 4 );
}

void    Packet::displayHex()
{
    CHECK_VALID
    cout<<"Packet : "<<h.command<<" | "<<h.serial<<" | ";
    const char* c = _packet.getConstBuf();
    for( size_t i=0; i<_packet.len(); i++)
        cout<<c[i]<<" ";
    cout<<endl;
}

void Packet::Header::ntoh( const void* buf )
{
    // TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
    const Header* h = (const Header*)buf;
    command         = h->command;
    serial          = ntohs( h->serial );
    timestamp       = ntohl( h->timestamp );
    data_length     = ntohl( h->data_length );
    flags           = ntohs( h->flags );
}

void Packet::Header::hton( char* buf )
{
    // TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
    Header* h      = (Header*)buf;
    h->command     = command;
    h->serial      = htons( serial );
    h->timestamp   = htonl( timestamp );
    h->data_length = htonl( data_length );
    h->flags       = htons( flags );
}

int Packet::send( SOCKETALT dst_s, const AddressIP* dst_a )
{
    CHECK_VALID
#ifdef VSNET_DEBUG
    if( dst_a == NULL )
        COUT << "sending " << Cmd(h.command) << " through " << dst_s << " to "
             << "NULL" << endl;
    else
        COUT << "sending " << Cmd(h.command) << " through " << dst_s << " to "
             << *dst_a << endl;
#endif

    int ret;
    // if( (ret = dst_s.sendbuf( _packet, dst_a, h.flags )) == -1)
    if( (ret = dst_s.sendbuf( this, dst_a, h.flags )) == -1)
    {
        h.ntoh( _packet.getConstBuf() );
        perror( "Error sending packet ");
        cout << Cmd(h.command) << endl;
    }
    else
    {
#ifdef VSNET_DEBUG
	    COUT << "After successful sendbuf" << endl;
        h.ntoh( _packet.getConstBuf() );

#if 0
	    PacketMem m( _packet.getVarBuf(), _packet.len(), PacketMem::LeaveOwnership );
	    m.dump( cout, 3 );
#endif
#endif
    }
    return ret;
}

const char* Packet::getData() const
{
    CHECK_VALID
    const char* c = _packet.getConstBuf();
    c += header_length;
    return c;
}

const char* Packet::getSendBuffer() const
{
    CHECK_VALID
    const char* c = _packet.getConstBuf();
    return c;
}

int Packet::getSendBufferLength() const
{
    return ( h.data_length + header_length );
}

#ifdef HAVE_ZLIB_H
bool Packet::packet_uncompress( PacketMem& outpacket, const unsigned char* src, size_t sz, Header& header )
{
    unsigned char* dest;
    unsigned int   ulen_i;
    unsigned long  ulen_l;
    int            zlib_errcode;

    src    += header_length;
    ulen_i = ntohl( *(unsigned int*)src );
    src    += sizeof(ulen_i);
    sz     -= sizeof(ulen_i);

    PacketMem mem( ulen_i + header_length );

    dest   = (unsigned char*)mem.getVarBuf();
    dest   += header_length;
    ulen_l = ulen_i;

    zlib_errcode = ::uncompress( dest, &ulen_l, src, sz );
    if( zlib_errcode != Z_OK )
    {
        COUT << "Compressed packet not correctly received, "
             << "decompression failed with zlib errcode "
	     << zlib_errcode << endl;
        return false;
    }
    else if( ulen_l != ulen_i )
    {
        COUT << "Compressed packet not correctly received, "
             << "expected len " << ulen_i << ", "
             << "received len " << ulen_l << endl;
        return false;
    }
    else
    {
	    outpacket = mem;
	    header.data_length = ulen_i;

#ifdef VSNET_DEBUG
	    COUT << "Parsed a compressed packet with"
	         << " cmd=" << Cmd(header.command) << "(" << (int)header.command << ")"
	         << " ser=" << header.serial
	         << " ts=" << header.timestamp
	         << " len=" << header.data_length
	         << endl;
#endif
        return true;
    }
}
#else /* HAVE_ZLIB_H */
bool Packet::packet_uncompress( PacketMem& , const unsigned char* , size_t , Header& )
{
    return false;
}
#endif /* HAVE_ZLIB_H */

