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
