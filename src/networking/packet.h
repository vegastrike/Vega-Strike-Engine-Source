#ifndef __PACKET_H
#define __PACKET_H

#include <iostream>
//#include "incnet.h"
#include "const.h"
#include "netclass.h"

using std::cout;
using std::endl;
//using std::hex;

#ifdef _UDP_PROTO
	extern char nbpackets;
	extern int	char_size;
#endif

class Packet
{
//#ifdef _UDP_PROTO
//		char			id;
//#endif
		unsigned char	command;
		ObjSerial		serial;
		unsigned int	timestamp;
		//unsigned int	delay;
		unsigned short	data_length;
		char			databuffer[MAXBUFFER];
		int				priority;

		const static unsigned short	header_length = sizeof( ObjSerial) + sizeof( char) + sizeof( unsigned short) + sizeof( unsigned int); // + sizeof( unsigned int);

	public:

		Packet()
		{
			memset( databuffer, 0, MAXBUFFER);
			priority = 0;
			serial = 0;
			command = 0;
			this->data_length = 0;
		}
		~Packet()
		{
		}

		void	create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, int prio);
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
		unsigned int	getTimestamp() { return this->timestamp;}
		unsigned char	getCommand() { return this->command;}
		int				getPriority() { return this->priority;}

		char * getData() { return databuffer;}

		void	reset()	{ this->command = 0;}
};

#endif
