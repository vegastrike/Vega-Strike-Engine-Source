#ifndef __PACKET_H
#define __PACKET_H

#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <list>
#include "networking/const.h"
#include "vsnet_cmd.h"
#include "vsnet_address.h"
#include "vsnet_socket.h"
#include "vsnet_socketflags.h"
#include "vsnet_debug.h"

using std::cout;
using std::endl;
using std::list;
using namespace std;

class Packet
{
    DECLARE_VALID

        struct Header
        {
            unsigned char   command;
            ObjSerial       serial;
            unsigned int    timestamp;
            //unsigned int  delay;
            unsigned int    data_length;
            unsigned short  flags;

            void hton( char* c );
            void ntoh( const void* c );
        };

        LOCALCONST_DECL(unsigned short,header_length,sizeof( struct Header))

        Header          h;
        PacketMem       _packet;

    public:

        Packet();
        Packet( const void* buffer, size_t sz );
        Packet( PacketMem& buffer );
        Packet( const Packet &a );

        ~Packet();

        Packet& operator=( const Packet &a )
        {
            copyfrom( a );
            return *this;
        }

        void copyfrom( const Packet& a );

        bool operator==( const Packet &p ) const
        {
            if( _packet == p._packet ) return true;
            cout<<"Packets are different"<<endl;
            return false;
        }

	/// flags is a bitwise OR of PCKTFLAGS
        int send( Cmd cmd, ObjSerial nserial,
                  const char * buf, unsigned int length,
                  int flags,
                  const AddressIP* dst, const SOCKETALT& sock,
                  const char* caller_file, int caller_line );

	/// flags is a bitwise OR of PCKTFLAGS
        inline void bc_create( Cmd cmd, ObjSerial nserial,
                               const char * buf, unsigned int length,
                               int flags,
                               const char* caller_file, int caller_line )
        {
            create( cmd, nserial, buf, length, flags, caller_file, caller_line );
        }

        inline int bc_send( const AddressIP& dst, const SOCKETALT& sock )
        {
            return send( sock, &dst );
        }

        void    display( const char* file, int line );
        void    displayHex();

        inline unsigned int getDataLength() const { return h.data_length; }

        static unsigned short   getHeaderLength()
        {
            return header_length;
        }

        inline ObjSerial       getSerial() const    { return h.serial;}
        inline unsigned int    getTimestamp() const { return h.timestamp;}
        inline Cmd             getCommand() const   { return (Cmd)h.command;}
        inline unsigned short  getFlags() const     { return h.flags;}
        inline void            setFlag ( enum PCKTFLAGS fl ) { h.flags |= fl; }
        inline void            setFlags( unsigned short fl ) { h.flags = fl; }

        const char* getData() const;
        const char* getSendBuffer() const;
        int         getSendBufferLength() const;

        void    reset() { h.command = 0;}

private:
        void    create( Cmd cmd, ObjSerial nserial,
                        const char * buf, unsigned int length,
                        int flags,
                        const char* caller_file, int caller_line );
        int     send( SOCKETALT dst_s, const AddressIP* dst_a );

        static bool packet_uncompress( PacketMem&           dest,
                                       const unsigned char* src,
                                       size_t               sz,
                                       Header&              header );
};

#endif

