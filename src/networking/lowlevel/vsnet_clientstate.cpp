#include <string>
#include <stdio.h>
#include "networking/client.h"
#include "vsnet_clientstate.h"
#include "cmd/unit_generic.h"

using std::string;

ClientState::ClientState()
{
	//this->delay=50;
	this->client_serial = 0;
	this->pos.position.Set( 20400,-35400,84700000.0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
}

ClientState::ClientState( Unit * un)
{
	this->client_serial = un->GetSerial();
	this->pos.position = un->curr_physical_state.position;
	this->pos.orientation = un->curr_physical_state.orientation;
	this->veloc = un->Velocity;
	this->accel = un->GetAcceleration();
}
ClientState::ClientState( ObjSerial serial)
{
	this->client_serial = serial;
	this->pos.position.Set( 20400,-35400,84700000.0);
	//pos.orientation.Set( 100,0,0);
	this->veloc.Set( 0,0,0);
	this->accel.Set( 0,0,0);
	//this->delay = 50;
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
	//this->delay = del;
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	this->accel = acc;
}

ClientState::ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, unsigned int del)
{
	//this->delay = del;
	this->client_serial = serial;
	this->pos = trans;
	this->veloc = velocity;
	this->accel = acc;
}

void		ClientState::display() const
{
	cerr<<"SERIAL="<<this->client_serial;
	cerr<<" - Position="<<pos.position.i<<","<<pos.position.j<<","<<pos.position.k;
	cerr<<" - Orientation="<<pos.orientation.v.i<<","<<pos.orientation.v.j<<","<<pos.orientation.v.k;
	cerr<<" - Velocity="<<veloc.i<<","<<veloc.j<<","<<veloc.k;
	cerr<<" - Acceleration="<<accel.i<<","<<accel.j<<","<<accel.k<<endl;
}

int		ClientState::operator==( const ClientState & ctmp)
{
	// Need == operator for Transformation class
	return (this->client_serial==ctmp.client_serial && this->pos.position==ctmp.pos.position && this->pos.orientation==ctmp.pos.orientation);
	//return 0;
}

void	ClientState::netswap()
{
	// Switch everything to host or network byte order
	//this->delay = POSH_BigU32( this->delay);
	//this->client_serial = OBJSERIAL_TONET( this->client_serial);

	this->pos.netswap();
	this->veloc.netswap();
	this->accel.netswap();
}

std::ostream& operator<<( std::ostream& ostr, const Client& c )
{
    ostr << "(clnt addr " << c.cltadr
         << " sock=" << c.sock;
    return ostr;
}

