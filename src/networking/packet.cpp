//#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "packet.h"
#include "lin_time.h"

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

void	Packet::create( unsigned char cmd, ObjSerial nserial, char * buf, unsigned int length, int prio)
{
	this->priority = prio;
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
	this->timestamp = htonl( microtime);
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
	this->serial = htons( nserial);
	//cout<<"Command : "<<cmd<<" - serial : "<<nserial<<" - command : "<<this->command<<" - serial : "<<this->serial<<endl;
	this->data_length = length;
	memset( this->databuffer, 0, MAXBUFFER);
	if( length)
		memcpy( this->databuffer, buf, length);
	this->data_length = htons( this->data_length);
}

void	Packet::display()
{
	cout<<"*** Packet display -- Command : "<<this->command;
	cout<<" - Serial : "<<this->serial<<" - Priority : "<<this->priority<<endl;
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

void	Packet::received()
{
	// TO CHANGE IF ObjSerial IS NOT A SHORT ANYMORE
	this->serial = ntohs( this->serial);
	this->timestamp = ntohl( this->timestamp);
	//this->delay = ntohl( this->delay);
	this->data_length = ntohl( this->data_length);
}
