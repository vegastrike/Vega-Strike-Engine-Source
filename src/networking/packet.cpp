//#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "packet.h"
#include "lin_time.h"

PacketQueue sendQueue;
PacketQueue recvQueue;

#ifdef _UDP_PROTO
	char	nbpackets = 0;
	int		char_size = sizeof( char)*8;
#endif

/*
#ifndef _WIN32
	static long nbsec=0;
	static long nbusec=0;
	static long old_nbsec=0;
	static long old_nbusec=0;
	static int  elapsed_usec;
#endif
*/
static	unsigned int microtime;

void	Packet::displayCmd( unsigned char com)
{
	switch( com)
	{
		case CMD_LOGIN: cout<<"CMD_LOGIN"<<endl;
		break;
		case CMD_LOGOUT: cout<<"CMD_LOGOUT"<<endl;
		break;
		case CMD_INITIATE: cout<<"CMD_INITIATE"<<endl;
		break;
		case CMD_POSUPDATE: cout<<"CMD_POSUPDATE"<<endl;
		break;
		case CMD_NEWCHAR: cout<<"CMD_NEWCHAR"<<endl;
		break;
		case CMD_LETSGO: cout<<"CMD_LETSGO"<<endl;
		break;
		case CMD_UPDATECLT: cout<<"CMD_UPDATECLT"<<endl;
		break;
		case CMD_ADDCLIENT: cout<<"CMD_ADDLCIENT"<<endl;
		break;
		case CMD_FULLUPDATE: cout<<"CMD_FULLUPDATE"<<endl;
		break;
		case CMD_PING: cout<<"CMD_PING"<<endl;
		break;
		case CMD_SNAPSHOT: cout<<"CMD_SNAPSHOT"<<endl;
		break;
		case CMD_CREATECHAR: cout<<"CMD_CREATECHAR"<<endl;
		break;
		case CMD_LOCATIONS: cout<<"CMD_LOCATIONS"<<endl;
		break;
		case LOGIN_ERROR: cout<<"LOGIN_ERROR"<<endl;
		break;
		case LOGIN_ACCEPT: cout<<"LOGIN_ACCEPT"<<endl;
		break;
		case LOGIN_ALREADY: cout<<"LOGIN_ALREADY"<<endl;
		break;
		case LOGIN_NEW: cout<<"LOGIN_NEW"<<endl;
		break;
		case CMD_NEWSUBSCRIBE: cout<<"CMD_NEWSUBSCRIBE"<<endl;
		break;
		case CMD_ENTERCLIENT: cout<<"CMD_ENTERCLIENT"<<endl;
		break;
		case CMD_EXITCLIENT: cout<<"CMD_EXITCLIENT"<<endl;
		break;
		case CMD_ADDEDYOU: cout<<"CMD_ADDEDYOU"<<endl;
		break;
		case CMD_DISCONNECT: cout<<"CMD_DISCONNECT"<<endl;
		break;
		default: cout<<"CMD UNKNOWN"<<endl;
	}
}

void	Packet::displayCmd()
{
	switch( this->command)
	{
		case CMD_LOGIN: cout<<"CMD_LOGIN"<<endl;
		break;
		case CMD_LOGOUT: cout<<"CMD_LOGOUT"<<endl;
		break;
		case CMD_INITIATE: cout<<"CMD_INITIATE"<<endl;
		break;
		case CMD_POSUPDATE: cout<<"CMD_POSUPDATE"<<endl;
		break;
		case CMD_NEWCHAR: cout<<"CMD_NEWCHAR"<<endl;
		break;
		case CMD_LETSGO: cout<<"CMD_LETSGO"<<endl;
		break;
		case CMD_UPDATECLT: cout<<"CMD_UPDATECLT"<<endl;
		break;
		case CMD_ADDCLIENT: cout<<"CMD_ADDLCIENT"<<endl;
		break;
		case CMD_FULLUPDATE: cout<<"CMD_FULLUPDATE"<<endl;
		break;
		case CMD_PING: cout<<"CMD_PING"<<endl;
		break;
		case CMD_SNAPSHOT: cout<<"CMD_SNAPSHOT"<<endl;
		break;
		case CMD_CREATECHAR: cout<<"CMD_CREATECHAR"<<endl;
		break;
		case CMD_LOCATIONS: cout<<"CMD_LOCATIONS"<<endl;
		break;
		case LOGIN_ERROR: cout<<"LOGIN_ERROR"<<endl;
		break;
		case LOGIN_ACCEPT: cout<<"LOGIN_ACCEPT"<<endl;
		break;
		case LOGIN_ALREADY: cout<<"LOGIN_ALREADY"<<endl;
		break;
		case LOGIN_NEW: cout<<"LOGIN_NEW"<<endl;
		break;
		case CMD_NEWSUBSCRIBE: cout<<"CMD_NEWSUBSCRIBE"<<endl;
		break;
		case CMD_ENTERCLIENT: cout<<"CMD_ENTERCLIENT"<<endl;
		break;
		case CMD_EXITCLIENT: cout<<"CMD_EXITCLIENT"<<endl;
		break;
		case CMD_ADDEDYOU: cout<<"CMD_ADDEDYOU"<<endl;
		break;
		case CMD_DISCONNECT: cout<<"CMD_DISCONNECT"<<endl;
		break;
		default: cout<<"CMD UNKNOWN"<<endl;
	}
}

void	Packet::create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio, AddressIP * dst, SOCKETALT sock)
{
	this->flags = prio;
	// Get a timestamp for packet (used for interpolation on client side)
	double curtime = getNewTime();
	microtime = (unsigned int) (floor(curtime*1000));
	this->timestamp = microtime;
	this->command = cmd;

	// buf is an allocated char * containing message
	if( length > MAXBUFFER)
	{
		cout<<"Error : initializing network packet with data length > MAX (length="<<length<<")"<<endl;
		cout<<"Command was : ";
		this->displayCmd();
		exit(1);
	}
	this->serial = nserial;
	//cout<<"Command : "<<cmd<<" - serial : "<<nserial<<" - command : "<<this->command<<" - serial : "<<this->serial<<endl;
	this->data_length = length;
	memset( this->databuffer, 0, MAXBUFFER);
	if( length)
		memcpy( this->databuffer, buf, length);
	// Here we memcpy because we don't know the size of these types and if they are pointers or not (they may be)
	memcpy( &this->destaddr, dst, sizeof( AddressIP));
	memcpy( &this->socket, &sock, sizeof( SOCKETALT));
}

void	Packet::setNetwork( AddressIP * dst, SOCKETALT sock)
{
	memcpy( &this->destaddr, dst, sizeof( AddressIP));
	memcpy( &this->socket, &sock, sizeof( SOCKETALT));
}

void	Packet::create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, enum PCKTFLAGS prio)
{
	this->flags = prio;
	// Get a timestamp for packet (used for interpolation on client side)
	//UpdateTime();
	double curtime = getNewTime();
	//cout<<"FULL TIMESTAMP : "<<curtime<<endl;
	microtime = (unsigned int) (floor(curtime*1000));
	//cout<<"TIMESTAMP 1 : "<<microtime<<endl;
/*
	long elapstmp;
#ifndef _WIN32
	struct timeval tvtmp;
	gettimeofday( &tvtmp, NULL);
	old_nbsec = nbsec;
	nbsec = tvtmp.tv_sec;
	old_nbusec = nbusec;
	nbusec = tvtmp.tv_usec;
	microtime = (unsigned int) (nbusec - old_nbusec);
	if( (elapstmp = nbsec-old_nbsec))
		microtime += elapstmp*1000000;
#endif
*/
	this->timestamp = microtime;
	//cout<<"TIMESTAMP 2 : "<<this->timestamp<<endl;
	this->command = cmd;

//#ifdef _UDP_PROTO
	// Increment packet id modulo 256 (stored on a char)
//	nbpackets = (char) modf((nbpackets+1), (double *)&char_size);
//#endif
	// buf is an allocated char * containing message
	if( length > MAXBUFFER)
	{
		cout<<"Error : initializing network packet with data length > MAX (length="<<length<<")"<<endl;
		cout<<"Command was : ";
		this->displayCmd();
		exit(1);
	}
	this->serial = nserial;
	//cout<<"Command : "<<cmd<<" - serial : "<<nserial<<" - command : "<<this->command<<" - serial : "<<this->serial<<endl;
	this->data_length = length;
	memset( this->databuffer, 0, MAXBUFFER);
	if( length)
		memcpy( this->databuffer, buf, length);
}

void	Packet::display()
{
	cout<<"*** Packet display -- Command : "<<this->command;
	cout<<" - Serial : "<<this->serial<<" - Flags : "<<this->flags<<endl;
	cout<<"***                   Buffer : ";
	for( int i=0; i<data_length-1; i++)
		cout<<databuffer[i];
	cout<<endl;
}

void	Packet::displayHex()
{
	//cout<<"Packet : "<<hex<<this->command<<" | "<<hex<<this->serial<<" | ";
	cout<<"Packet : "<<this->command<<" | "<<this->serial<<" | ";
	for( int i=0; i<data_length-1; i++)
		cout<<databuffer[i]<<" ";
	cout<<endl;
}
union floatint {
  float f;
  int i;
};
void	Packet::received()
{
	// TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
	this->serial = ntohs( this->serial);
	floatint f;f.f=this->timestamp;
	f.i = ntohl(f.i);
	this->timestamp = f.f;
	//this->delay = ntohl( this->delay);
	this->data_length = ntohs( this->data_length);
}

void	Packet::tosend()
{
	// TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
	this->serial = htons( this->serial);
	floatint f;f.f=this->timestamp;
	f.i = ntohl(f.i);
	this->timestamp = f.f;	
	//this->delay = htonl( this->delay);
	this->data_length = htons( this->data_length);
}


/*** Receive all the packets in the network buffer and adds them to the list ***/
/*** Also sends back an ack in UDP mode ***/
int		PacketQueue::receive( NetUI * net, SOCKETALT & sock, AddressIP & ipaddr, ObjSerial ser)
{
	// Read the networkfor packets
	Packet p;
	// Can receive 8 packets at the same time for now ? Seems reasonable
	int nb=0;
	unsigned int len = 0;
	char buffer[MAXBUFFER*8];
	unsigned int recvsize=0, totalsize=0;

	if( (len=net->recvbuf( sock, buffer, len, &ipaddr))<=0)
	{
		//perror( "Error recv -1 ");
		nb = -1;
		//net->closeSocket( this->clt_sock);
	}
	else
	{
		// Until we haven't read all the buffer
		while( totalsize<=len)
		{
			if( len<MAXBUFFER)
				recvsize = len;
			else
				recvsize = header_length+MAXBUFFER;
			totalsize += recvsize;
			// We receive packets
			memcpy( &p, buffer, recvsize);
			p.received();
			packets.push_back( p);
#ifdef _UDP_PROTO
			// Prepare an ACK for the packet
			p.create( CMD_ACK, ser, NULL, 0, SENDRELIABLE, &p.destaddr, p.socket);
			sendQueue.add( p);
#endif
			nb++;
		}
	}
	return nb;
}

/*** Returns the front element from the list and remove it ***/
Packet PacketQueue::getNextPacket()
{
	Packet ret = packets.front();
	packets.pop_front();
	return ret;
}

/*** Mark a packet as ACKED for removal from the list ***/
void PacketQueue::ack( Packet p)
{
	for( PaI i=packets.begin(); i!=packets.end(); i++)
	{
		if( i->timestamp==p.timestamp && i->serial==p.serial)
			i->flags = ACKED;
	}
}

/*** Sends or resends the packets and removes those that are ACKED or never were ***/
void PacketQueue::send( NetUI * net)
{
	// In UDP, we should wait for a timeout before sending the packet again.
	bool timeout_expired = false;
	double now = getNewTime();
	int ret;
	for( PaI i=packets.begin(); i!=packets.end(); i++)
	{
		// Here compute if the ack timeout has been reached
		if( i->nbsent > 0 && ((now - i->timestamp)*(i->nbsent) > (UDP_TIMEOUT*(i->nbsent)) ) )
			timeout_expired = true;
		// See if the packet has to be sent
		if( i->flags == SENDANDFORGET || (i->flags == SENDRELIABLE && i->nbsent < NUM_RESEND && timeout_expired))
		{
			if( (ret = net->sendbuf( i->socket, (char *)&(*i), i->getSendLength(), &i->destaddr)) == -1)
			{
				perror( "Error sending packet ");
				i->displayCmd();
			}
			i->oldtimestamp = now;
			i->nbsent++;
		}
		timeout_expired = false;
	}
	for( PaI j=packets.begin(); j!=packets.end(); j++)
	{
		// in TCP every packet has to be deleted since it is reliable
		// In UDP mode we must not delete the packet since it may not have been received
		//
		// NOTE - NOTE - NOTE
		// For now, UDP retransmission mecanism is incomplete and thus is disabled

#ifdef _UDP_PROTO
		// Or removed because ACKED or sent too many times or a SENDANDFORGET packet has been sent
		//if( j->flags==ACKED || j->nbsent>=NUM_RESEND || (j->flags==SENDANDFORGET && j->nbsent>0))
		//{
#endif
			packets.remove( (*j));
#ifdef _UDP_PROTO
		//}
#endif
	}
}

/*** Adds a packet to the list (in order to be sent later) ***/
void PacketQueue::add( Packet p)
{
	p.tosend();
	packets.push_back( p);
}
