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
#include "lin_time.h"

LOCALCONST_DEF(Packet,unsigned short,header_length,sizeof( struct Header))

#if defined(_WIN32) && defined(_MSC_VER) && defined(USE_BOOST_129) //wierd error in MSVC
#  define __LINE__NOMSC	0
#else
#  define __LINE__NOMSC	__LINE__ 
#endif

Packet::Packet()
{
    h.command     = 0;
    h.serial      = 0;
    h.timestamp   = 0;
    h.data_length = 0;
    h.flags       = NONE;

    nbsent = 0;
    destaddr = NULL;
}

#ifdef HAVE_ZLIB_H
bool Packet::packet_uncompress( PacketMem& outpacket, const unsigned char* src, size_t sz, Header& header )
{
    unsigned char* dest;
    unsigned short ulen_s;
    unsigned long  ulen_l;
    int            zlib_errcode;

    src    += header_length;
    ulen_s = ntohs( *(unsigned short*)src );
    src    += sizeof(unsigned short);
    sz     -= sizeof(unsigned short);

    PacketMem mem( ulen_s + header_length );

    dest   = (unsigned char*)mem.getVarBuf();
    dest   += header_length;
    ulen_l = ulen_s;

    zlib_errcode = ::uncompress( dest, &ulen_l, src, sz );
    if( zlib_errcode != Z_OK )
    {
        COUT << "Compressed packet not correctly received, "
             << "decompression failed" << endl;
        return false;
    }
    else if( ulen_l != ulen_s )
    {
        COUT << "Compressed packet not correctly received, "
             << "expected len " << ulen_s << ", "
             << "received len " << ulen_l << endl;
        return false;
    }
    else
    {
	    outpacket = mem;
	    header.data_length = ulen_s;

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

Packet::Packet( const void* buffer, size_t sz )
{
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
#ifdef HAVE_ZLIB_H
	        if( packet_uncompress( _packet,
	                               (const unsigned char*)buffer,
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
    nbsent = 0;
    destaddr = NULL;
}

Packet::Packet( PacketMem& buffer )
{
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
    nbsent = 0;
    destaddr = NULL;
}

Packet::Packet( const Packet &a )
{
    destaddr = 0;
    copyfrom( a );
}

Packet::~Packet()
{
    if( destaddr ) delete destaddr;
}

void Packet::copyfrom( const Packet& a )
{
    h.command     = a.h.command;
    h.serial      = a.h.serial;
    h.timestamp   = a.h.timestamp;
    h.data_length = a.h.data_length;
    h.flags       = a.h.flags;
    _packet       = a._packet;
    nbsent        = a.nbsent;

    if( a.destaddr ) {
        if( destaddr==NULL ) {
            destaddr = new AddressIP;
        }
        memcpy( destaddr, a.destaddr, sizeof( AddressIP));
    } else if( destaddr ) {
        delete destaddr;
        destaddr = 0;
    }

    socket = a.socket;
}

void    Packet::setNetwork( const AddressIP * dst, SOCKETALT sock)
{
    if( destaddr == NULL )
    {
        if( dst != NULL )
        {
            destaddr = new AddressIP;
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
    }
    else
    {
        if( dst != NULL )
        {
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
        else
        {
            delete destaddr;
            destaddr = NULL;
        }
    }
    this->socket = sock;
}

int Packet::send( Cmd cmd, ObjSerial nserial, char * buf, unsigned int length,
                  int prio, const AddressIP* dst,
		          const SOCKETALT& sock, const char* caller_file,
		          int caller_line )
{
    create( cmd, nserial, buf, length, prio, dst, sock, caller_file, caller_line );
    return send( );
}

int Packet::send( Cmd cmd, ObjSerial nserial, const char * buf, unsigned int length,
                  int prio, const AddressIP* dst,
		          const SOCKETALT& sock, const char* caller_file,
		          int caller_line )
{
	char * buffer = new char[length];
	memcpy( buffer, buf, length);
    create( cmd, nserial, buffer, length, prio, dst, sock, caller_file, caller_line );
	delete buffer;
    return send( );
}

void Packet::create( Cmd cmd, ObjSerial nserial, char * buf,
                     unsigned int length, int prio,
                     const AddressIP* dst, const SOCKETALT& sock,
                     const char* caller_file, int caller_line )
{
    unsigned int microtime;

    // Get a timestamp for packet (used for interpolation on client side)
    double curtime = getNewTime();
    microtime = (unsigned int) (floor(curtime*1000));
    h.timestamp = microtime;

#ifdef VSNET_DEBUG
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
         << "    *** from " << caller_file << ":" << caller_line << endl
         << "    *** send " << cmd << " ser=" << nserial << ", " << length
                 << " bytes to socket " << sock << endl
         << "    *** curtime " << curtime
                 << " microtime " << microtime
                 << " timestamp " << h.timestamp << endl;
#else
    COUT << "*** send " << cmd << " ser=" << nserial << ", "
         << length << " bytes to socket " << sock << endl;
#endif

    h.command   = cmd;
    h.flags     = prio;

    // buf is an allocated char * containing message
    h.serial = nserial;
    
#ifdef HAVE_ZLIB_H
    bool packet_filled = false;

    if( ( prio & COMPRESSED ) && sock.isCompressAllowed() )
    {
        size_t sz;   // complicated zlib rules for safety reasons
        sz = length + ( length/10 ) + 15 + header_length;

        _packet = PacketMem( sz );

        char*          c      = _packet.getVarBuf( );
        unsigned long  clen_l = length;
        unsigned short ulen_s;
        unsigned char* dest   = (unsigned char*)&c[header_length+sizeof(unsigned short)];
        int            zlib_errcode;

        zlib_errcode = ::compress2( dest, &clen_l, (unsigned char*)buf, length, 9 );

        if( zlib_errcode == Z_OK )
        {
            if( clen_l < length + 2 )
            {
                ulen_s = htons( (unsigned short)length );
                memcpy( &c[header_length], &ulen_s, sizeof(unsigned short) );

                h.data_length = (unsigned short)clen_l + sizeof(unsigned short);
                h.hton( c );

                _packet.trunc( h.data_length+header_length );

                COUT << "Created a compressed packet of length "
                     << h.data_length+header_length << " for sending" << endl;
                //_packet.dump( cout, 0 );
                packet_filled = true;
            }
            else
            {
                COUT << "Compressing " << cmd
	                 << " packet refused - bigger than original" << std::endl;
            }
        }
    }

    if( packet_filled == false )
    {
#endif /* HAVE_ZLIB_H */
        h.flags &= ( ~COMPRESSED );    // make sure that it's never set here
        h.data_length = length;

        _packet = PacketMem( length + header_length );
        char* c = _packet.getVarBuf( );
        h.hton( c );
        memcpy( &c[header_length], buf, length );
#ifdef VSNET_DEBUG
        COUT << "Created a packet of length "
             << length+header_length << " for sending" << endl;
#endif
        //_packet.dump( cout, 0 );
#ifdef HAVE_ZLIB_H
    }
#endif /* HAVE_ZLIB_H */

    if( destaddr == NULL )
    {
        if( dst != NULL )
        {
            destaddr = new AddressIP;
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
    }
    else
    {
        if( dst != NULL )
        {
            memcpy( destaddr, dst, sizeof( AddressIP));
        }
        else
        {
            delete destaddr;
            destaddr = NULL;
        }
    }

    socket = sock;
}

void    Packet::display( const char* file, int line )
{
    cout << "*** " <<  file << ":" << line << " " << endl;
    cout << "*** Packet display -- Command : " << Cmd(h.command)
         << " - Serial : " << h.serial << " - Flags : " << h.flags << endl;
    cout<<"***                   Size   : " << getDataLength() + header_length << endl;
    cout<<"***                   Buffer : " << endl;
    _packet.dump( cout, 4 );
}

void    Packet::displayHex()
{
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
    data_length     = ntohs( h->data_length );
    flags           = ntohs( h->flags );
}

void Packet::Header::hton( char* buf )
{
    // TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
    Header* h      = (Header*)buf;
    h->command     = command;
    h->serial      = htons( serial );
    h->timestamp   = htonl( timestamp );
    h->data_length = htons( data_length );
    h->flags       = htons( flags );
}

Cmd Packet::getBufCommand( const PacketMem& buf )
{
    Header h;
    h.ntoh( buf.getConstBuf() );
    return Cmd(h.command);
}

int Packet::send( )
{
    //COUT << "sending " << Cmd(h.command) << " through " << socket << " to ";
    if( destaddr==0 )
        cout << "NULL" << endl;
    else
        cout << *destaddr << endl;

    int ret;
    if( (ret = socket.sendbuf( _packet, destaddr, h.flags )) == -1)
    {
        h.ntoh( _packet.getConstBuf() );
        perror( "Error sending packet ");
        cout << Cmd(h.command) << endl;
    }
    else
    {
#ifdef VSNET_DEBUG
	COUT << "packet after sendbuf: " << endl;
#endif
        h.ntoh( _packet.getConstBuf() );

	PacketMem m( _packet.getVarBuf(), _packet.len(), PacketMem::LeaveOwnership );
	//m.dump( cout, 3 );
    }
    return ret;
}

// void Packet::ack( )
// {
//     socket.ack( );
// }

char* Packet::getData()
{
    char* c = _packet.getVarBuf();
    c += header_length;
    return c;
}

const char* Packet::getData() const
{
    const char* c = _packet.getConstBuf();
    c += header_length;
    return c;
}

