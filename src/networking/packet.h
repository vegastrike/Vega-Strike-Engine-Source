#ifndef __PACKET_H
#define __PACKET_H

#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <list>
#include "const.h"
#include "vsnet_cmd.h"
#include "vsnet_address.h"
#include "vsnet_socket.h"

using std::cout;
using std::endl;
using std::list;
using namespace std;

class Packet
{
        struct Header
        {
            unsigned char   command;
            ObjSerial       serial;
            unsigned int    timestamp;
            //unsigned int  delay;
            unsigned short  data_length;
            unsigned short  flags;

            void hton( char* c );
            void ntoh( const void* c );
        };

        LOCALCONST_DECL(unsigned short,header_length,sizeof( struct Header))

        Header          h;
        PacketMem       _packet;

        unsigned short  nbsent;
        AddressIP*      destaddr;
        SOCKETALT       socket; // Socket to send on (in TCP mode)
        unsigned int    oldtimestamp;

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
                  char * buf, unsigned int length,
                  int flags,
                  const AddressIP* dst, const SOCKETALT& sock,
                  const char* caller_file, int caller_line );

	/// flags is a bitwise OR of PCKTFLAGS
        inline void bc_create( Cmd cmd, ObjSerial nserial,
                               char * buf, unsigned int length,
                               int flags,
                               const AddressIP* dst, const SOCKETALT& sock,
                               const char* caller_file, int caller_line )
        {
            create( cmd, nserial, buf, length, flags, dst, sock, caller_file, caller_line );
        }

        inline int bc_send( )
        {
            return send( );
        }

        void    display( const char* file, int line );
        void    displayHex();

        inline unsigned short getDataLength() const { return h.data_length; }

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
        void                   setNetwork( const AddressIP * dst, SOCKETALT sock);

        // void            ack( );

        char*       getData();
        const char* getData() const;

        void    reset() { h.command = 0;}

private:
        void    create( Cmd cmd, ObjSerial nserial,
                        char * buf, unsigned int length,
                        int flags,
                        const AddressIP* dst, const SOCKETALT& sock,
                        const char* caller_file, int caller_line );
        int     send( );

        static bool packet_uncompress( PacketMem&           dest,
	                               const unsigned char* src,
				       size_t               sz,
				       Header&              header );
};

#endif

