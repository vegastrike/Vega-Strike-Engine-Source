#include <stdio.h>
#include "cmd/unit_generic.h"
#include "packet.h"
#include "client.h"

ClientState::ClientState()
{
	this->delay=50;
	this->client_serial = 0;
	this->pos.position.Set( 20400,-35400,84700000.0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
}

ClientState::ClientState( ObjSerial serial)
{
	this->client_serial = serial;
	this->pos.position.Set( 20400,-35400,84700000.0);
	//pos.orientation.Set( 100,0,0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
	this->delay = 50;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc)
{
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	this->accel = acc;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, unsigned int del)
{
	this->delay = del;
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	this->accel = acc;
}

ClientState::ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, unsigned int del)
{
	this->delay = del;
	this->client_serial = serial;
	this->pos = trans;
	this->veloc = velocity;
	this->accel = acc;
}

void		ClientState::display()
{
	cout<<"SERIAL="<<this->client_serial;
	cout<<" - Position="<<pos.position.i<<","<<pos.position.j<<","<<pos.position.k;
	cout<<" - Orientation="<<pos.orientation.v.i<<","<<pos.orientation.v.j<<","<<pos.orientation.v.k;
	cout<<" - Velocity="<<veloc.i<<","<<veloc.j<<","<<veloc.k;
	cout<<" - Acceleration="<<accel.i<<","<<accel.j<<","<<accel.k<<endl;
}

int		ClientState::operator==( const ClientState & ctmp)
{
	// Need == operator for Transformation class
	return (this->client_serial==ctmp.client_serial && this->pos.position==ctmp.pos.position && this->pos.orientation==ctmp.pos.orientation);
	//return 0;
}

void	ClientState::tosend()
{
	this->delay = htonl( this->delay);
	this->client_serial = htons( this->client_serial);
}
void	ClientState::received()
{
	this->delay = ntohl( this->delay);
	this->client_serial = ntohs( this->client_serial);
}

// Warning char* is allocated here
void	LoadXMLUnit( Unit * unit, const char * filename, char *buf)
{
	// Load XML of unit
	if( unit != NULL)
	{
		cout<<"Error : unit already allocated !"<<endl;
		exit( 1);
	}
	//unit = new Unit( 0);
	//unit->LoadXML( filename, NULL);

	int maxsave = MAXBUFFER - Packet::getHeaderLength() - 2*NAMELEN, readsize=0;
	// Read the save or default save in buf after the login info
	if( buf!=NULL)
	{
		FILE *fp = fopen( filename, "r");
		if( fp == NULL)
		{
			cout<<"Error opening file "<<filename<<" - THAT SHOULD NOT HAPPEN !!"<<endl;
		}
		readsize = fread( (buf), sizeof( char), maxsave, fp);
		if( readsize>=maxsave)
		{
			cout<<"Error : save file is bigger than "<<maxsave<<" ("<<readsize<<")"<<endl;
			exit( 1);
		}
	}
}

void	WriteXMLUnit( const char * filename, char * xmlbuf, int tsize)
{
	FILE *fp = fopen( filename, "w");
	if( fp==NULL)
	{
		cout<<"File not found : "<<filename<<endl;
	}
	int ret = fwrite( xmlbuf, sizeof( char), tsize, fp);
	if( ret < 0)
	{
		cout<<"Error writing in xml file : "<<filename<<endl;
		exit( 1);
	}
}
