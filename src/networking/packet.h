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
//using std::hex;

// Number of times we resend a "reliable" packet in UDP mode
#define NUM_RESEND 3
enum PCKTFLAGS
{
    NONE          = 0,
    SENDANDFORGET = 0x0001,
    SENT          = 0x0002,
    RESENT        = 0x0004,
    ACKED         = 0x0008,
    SENDRELIABLE  = 0x0010,
    COMPRESSED    = 0x1000
};

ostream& operator<<( ostream& ostr, PCKTFLAGS flags );

class PacketQueue;

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
        // char   databuffer[MAXBUFFER];

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

        int send( Cmd cmd, ObjSerial nserial,
                  char * buf, unsigned int length,
                  enum PCKTFLAGS prio,
                  const AddressIP* dst, const SOCKETALT& sock,
                  const char* caller_file, int caller_line );

        inline void bc_create( Cmd cmd, ObjSerial nserial,
                               char * buf, unsigned int length,
                               enum PCKTFLAGS prio,
                               const AddressIP* dst, const SOCKETALT& sock,
                               const char* caller_file, int caller_line )
        {
            create( cmd, nserial, buf, length, prio, dst, sock, caller_file, caller_line );
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

        void            ack( );

        char*       getData();
        const char* getData() const;

        void    reset() { h.command = 0;}

private:
        void    create( Cmd cmd, ObjSerial nserial,
                        char * buf, unsigned int length,
                        enum PCKTFLAGS prio,
                        const AddressIP* dst, const SOCKETALT& sock,
                        const char* caller_file, int caller_line );
        int     send( );

        static bool packet_uncompress( PacketMem&           dest,
	                               const unsigned char* src,
				       size_t               sz,
				       Header&              header );
};

#if 0
typedef list<Packet>::iterator PaI;
extern char nbpackets;
extern int  char_size;

// A class that manage packet with priority
class PacketQueue
{
    private:
        list<Packet>        packets;
    public:
        /*** Adds a packet to the list (in order to be sent later) ***/
        void add( Packet p);
        /*** Sends or resends the packets and removes those that are
         *   ACKED or never were ***/
        void send( NetUI * net);

        /*** Returns the front element from the list and remove it ***/
        Packet getNextPacket();
        /*** Tells us if the packet list is empty ***/
        bool empty()
        { return packets.empty();}
        /*** Mark a packet as ACKED for removal from the list ***/
        void ack( Packet p);

    private:
        /*** Receive all the packets in the network buffer and adds them to the list ***/
        /*** Also sends back an ack in UDP mode ***/
        /*** From the server, the ACK packet will have serial=0 ***/
        int receive( NetUI * net, SOCKETALT & sock, AddressIP & ipaddr, ObjSerial ser);
};

extern PacketQueue sendQueue;
extern PacketQueue recvQueue;
#endif

#endif

