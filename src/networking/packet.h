#ifndef __PACKET_H
#define __PACKET_H

#include <iostream>
#include <string.h>
#include <list>
#include "const.h"
#include "netclass.h"

using std::cout;
using std::endl;
using std::list;
using namespace std;
//using std::hex;

const static unsigned short	header_length = sizeof( ObjSerial) + sizeof( char) + sizeof( unsigned short) + sizeof( unsigned int); // + sizeof( unsigned int);

// Number of times we resend a "reliable" packet in UDP mode
#define NUM_RESEND 3
enum PCKTFLAGS {NONE=0x0,SENDANDFORGET=0x1,SENT=0x2,RESENT=0x4,ACKED=0x8,SENDRELIABLE=0x10};

class PacketQueue;

class Packet
{
//#ifdef _UDP_PROTO
//		char			id;
//#endif
		unsigned char	command;
		ObjSerial		serial;
		float			timestamp;
		//unsigned int	delay;
		unsigned short	data_length;
		char			databuffer[MAXBUFFER];
		enum PCKTFLAGS	flags;
		unsigned short	nbsent;
		AddressIP		destaddr;
		SOCKETALT		socket; // Socket to send on (in TCP mode)
		float			oldtimestamp;

	public:

		Packet()
		{
			memset( databuffer, 0, MAXBUFFER);
			flags = NONE;
			serial = 0;
			command = 0;
			nbsent = 0;
			this->data_length = 0;
		}
		Packet( Packet &a)
		{
			this->command = a.command;
			this->serial = a.serial;
			this->timestamp = a.timestamp;
			this->data_length = a.data_length;
			this->flags = a.flags;
			this->nbsent = a.nbsent;
			memcpy( this->databuffer, a.databuffer, MAXBUFFER);
			memcpy( &this->destaddr, &a.destaddr, sizeof( AddressIP));
			this->socket = a.socket;
		}
		Packet( const Packet &a)
		{
			this->command = a.command;
			this->serial = a.serial;
			this->timestamp = a.timestamp;
			this->data_length = a.data_length;
			this->flags = a.flags;
			this->nbsent = a.nbsent;
			memcpy( this->databuffer, a.databuffer, MAXBUFFER);
			memcpy( &this->destaddr, &a.destaddr, sizeof( AddressIP));
			this->socket = a.socket;
		}
		bool operator==( const Packet &p)
		{
			bool ret = false;
			if( !memcmp( this, &p, header_length+data_length))
				ret=true;
			else
				cout<<"Packets are different"<<endl;
			return ret;
		}
		~Packet()
		{
		}

		void	create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio);
		void	create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio, AddressIP * dst, SOCKETALT sock);
		void	display();
		void	displayCmd();
		void	displayCmd( unsigned char com);
		void	displayHex();
		void	received();
		void	tosend();

		void	setLength( int length) { this->data_length = length - header_length;}
		//void	setDelay( unsigned int del) { this->delay = hotonl( del);}

		unsigned short			getLength()	{ return (header_length + this->data_length);}
		unsigned short			getSendLength()	{ return (header_length + ntohs(this->data_length));}
		unsigned short			getDataLength() {return this->data_length;}
		static unsigned short	getHeaderLength()
		{
			return header_length;
		}

		ObjSerial		getSerial() { return this->serial;}
		float			getTimestamp() { return this->timestamp;}
		unsigned char	getCommand() { return this->command;}
		enum PCKTFLAGS	getFlags() { return this->flags;}
		void			setFlags( enum PCKTFLAGS fl) { this->flags = fl;}
		void			setNetwork( AddressIP * dst, SOCKETALT sock);

		char * getData() { return databuffer;}

		void	reset()	{ this->command = 0;}

#ifdef _UDP_PROTO
		friend class PacketQueue;
#endif
};

typedef list<Packet>::iterator PaI;
extern char nbpackets;
extern int	char_size;

// A class that manage packet with priority
class PacketQueue
{
	private:
		list<Packet>		packets;
	public:
		/*** Adds a packet to the list (in order to be sent later) ***/
		void add( Packet p);
		/*** Sends or resends the packets and removes those that are ACKED or never were ***/
		void send( NetUI * net);
		/*** Receive all the packets in the network buffer and adds them to the list ***/
		/*** Also sends back an ack in UDP mode ***/
		/*** From the server, the ACK packet will have serial=0 ***/
		int receive( NetUI * net, SOCKETALT & sock, AddressIP & ipaddr, ObjSerial ser);
		/*** Returns the front element from the list and remove it ***/
		Packet getNextPacket();
		/*** Tells us if the packet list is empty ***/
		bool empty()
		{ return packets.empty();}
		/*** Mark a packet as ACKED for removal from the list ***/
		void ack( Packet p);
};

extern PacketQueue sendQueue;
extern PacketQueue recvQueue;

#endif
