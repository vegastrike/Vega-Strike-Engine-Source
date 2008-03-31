#include <string>
#include <stdio.h>
#include "networking/client.h"
#include "vsnet_clientstate.h"
#include "vsnet_address.h"
#include "cmd/unit_generic.h"

using std::string;

ClientState::ClientState()
{
	//this->delay=50;
	this->client_serial = 0;
	this->SPECWarpMultiplier = 0.0;
	this->SPECRampCounter = 0.0;
	this->pos.position.Set( 0,0,0);
	this->veloc.Set( 0,0,0);
	//this->accel.Set( 0,0,0);
        this->angveloc.Set(0,0,0);
}

ClientState::ClientState( const Unit * un)
{
	this->client_serial = un->GetSerial();
	this->pos.position = un->curr_physical_state.position;
	this->pos.orientation = un->curr_physical_state.orientation;
	this->veloc = un->Velocity;
	//this->accel = un->GetAcceleration();
	this->angveloc=un->AngularVelocity;
	
	if (un->graphicOptions.RampCounter!=0 || un->graphicOptions.InWarp) {
		this->SPECRampCounter = (un->graphicOptions.InWarp?1:-1) *
			un->graphicOptions.RampCounter;
		this->SPECWarpMultiplier = un->graphicOptions.WarpFieldStrength;
	} else {
		this->SPECRampCounter = 0.0;
		this->SPECWarpMultiplier = 1.0;
	}
}

void ClientState::setUnitState( Unit * un) const
{
	un->curr_physical_state = this->pos;
	un->Velocity = this->veloc;
	un->AngularVelocity = this->angveloc;

	if (SPECWarpMultiplier>=1.0) {
		// Otherwise, do not change.
		un->graphicOptions.InWarp = (SPECWarpMultiplier != 1.0 || SPECRampCounter != 0.0);
		if (un->graphicOptions.InWarp) {
			if (SPECRampCounter < 0) {
				un->graphicOptions.InWarp = 0;
				un->graphicOptions.RampCounter = -SPECRampCounter;
			} else {
				un->graphicOptions.RampCounter = SPECRampCounter;
			}
			un->graphicOptions.WarpFieldStrength = SPECWarpMultiplier;
		} else {
			un->graphicOptions.WarpFieldStrength = 1.0;
			un->graphicOptions.RampCounter = 0.0;
		}
		un->graphicOptions.WarpRamping = 0;
	}
}

ClientState::ClientState( ObjSerial serial)
{
	this->client_serial = serial;
	this->pos.position.Set( 0,0,0);
	//pos.orientation.Set( 100,0,0);
	this->veloc.Set( 0,0,0);
	//this->accel.Set( 0,0,0);
        this->angveloc.Set(0,0,0);
	this->SPECWarpMultiplier = 0.0;
	this->SPECRampCounter = 0.0;
	//this->delay = 50;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, Vector angvel)
{
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	//this->accel = acc;
        this->angveloc=angvel;
	this->SPECWarpMultiplier = 0.0;
	this->SPECRampCounter = 0.0;
}

ClientState::ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, Vector angvel, unsigned int del)
{
	//this->delay = del;
	this->client_serial = serial;
	this->pos.position = posit;
	this->pos.orientation = orientat;
	this->veloc = velocity;
	//this->accel = acc;
        this->angveloc=angvel;
	this->SPECWarpMultiplier = 0.0;
	this->SPECRampCounter = 0.0;
}

ClientState::ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, Vector angvel, unsigned int del)
{
	//this->delay = del;
	this->client_serial = serial;
	this->pos = trans;
	this->veloc = velocity;
	//this->accel = acc;
        this->angveloc=angvel;
	this->SPECWarpMultiplier = 0.0;
	this->SPECRampCounter = 0.0;
}

void    ClientState::display() const
{
    display( cerr );
    cerr << endl;
}
bool operator == (const Quaternion& a, const Quaternion &b) {
  return a.v==b.v&&a.s==b.s;
}
void    ClientState::display( std::ostream& ostr ) const
{
	ostr << "SERIAL=" <<this->client_serial
	     << " - Position="<<pos.position.i<<","<<pos.position.j<<","<<pos.position.k
	     << " - Orientation="<<pos.orientation.v.i<<","<<pos.orientation.v.j<<","<<pos.orientation.v.k
	     << " - Velocity="<<veloc.i<<","<<veloc.j<<","<<veloc.k
          //	     << " - Acceleration="<<accel.i<<","<<accel.j<<","<<accel.k;
	     << " - Ang Velocity="<<angveloc.i<<","<<angveloc.j<<","<<angveloc.k;
    if (SPECWarpMultiplier!=0.0) {
        ostr << " - SPEC="<<SPECWarpMultiplier<<", Ramp="<<SPECRampCounter;
    }
}

int		ClientState::operator==( const ClientState & ctmp) const
{
	// Need == operator for Transformation class
	return (this->client_serial==ctmp.client_serial && this->pos.position==ctmp.pos.position && this->pos.orientation==ctmp.pos.orientation);
	//return 0;
}

// Not used any more...
/*
void	ClientState::netswap()
{
	// Switch everything to host or network byte order
	//this->delay = POSH_BigU32( this->delay);
	//this->client_serial = OBJSERIAL_TONET( this->client_serial);

	this->pos.netswap();
	this->veloc.netswap();
        //	this->accel.netswap();
	this->angveloc.netswap();
	this->SPECWarpMultiplier = NetSwap(this->SPECWarpMultiplier);
	this->SPECRampCounter = NetSwap(this->SPECRampCounter);
}
*/

std::ostream& operator<<( std::ostream& ostr, const Client& c )
{
    ostr << "(clnt addr " << c.cltadr
         << " tcpsock=" << c.tcp_sock << ", lossysock=";
	if (c.lossy_socket) {
		ostr << *c.lossy_socket;
	} else {
		ostr << "NULL";
	}
    return ostr;
}

std::ostream& operator<<( std::ostream& ostr, const ClientState& cs )
{
    cs.display( ostr );
    return ostr;
}

