#include "netserver.h"
#include "vsnet_debug.h"
#include "netbuffer.h"
#include "vsnet_debug.h"
#include "lin_time.h"

// WEAPON STUFF

void	NetServer::BroadcastUnfire( ObjSerial serial, int weapon_index, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;

	//netbuf.addSerial( serial);
	netbuf.addInt32( weapon_index);

	//p.send( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, __LINE__);
	p.bc_create( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1695) );
	zonemgr->broadcast( zone, serial, &p );
}

// In BroadcastFire we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetServer::BroadcastFire( ObjSerial serial, int weapon_index, ObjSerial missile_serial, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;
	// bool found = false;

	netbuf.addInt32( weapon_index);
	netbuf.addSerial( missile_serial);

	p.bc_create( CMD_FIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1710) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED -> SHOULD USE broadcastNoSelf instead if we dont want that
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendDamages( ObjSerial serial, unsigned short zone, Shield shield, Armor armor, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addFloat( amt);
	netbuf.addFloat( ppercentage);
	netbuf.addFloat( spercentage);
	netbuf.addVector( pnt);
	netbuf.addVector( normal);
	netbuf.addColor( color);
	netbuf.addShield( shield);
	netbuf.addArmor( armor);

	p.bc_create( CMD_DAMAGE, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1729) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendKill( ObjSerial serial, unsigned short zone)
{
	Packet p;
	Unit * un;

	cerr<<"SENDING A KILL for serial "<<serial<<" in zone "<<zone<<endl;
	// Find the client in the udp & tcp client lists in order to set it out of the game (not delete it yet)
	ClientPtr clt = this->getClientFromSerial( serial);
	if( !clt )
	{
		COUT<<"Killed a non client Unit = "<<serial<<endl;
		un = zonemgr->getUnit( serial, zone);
		zonemgr->removeUnit( un, zone);
	}
	else
	{
		COUT<<"Killed client serial = "<<serial<<endl;
		zonemgr->removeClient( clt );
	}

	p.bc_create( CMD_KILL, serial, NULL, 0, SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1771) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendJump( ObjSerial serial, bool ok)
{
	Packet p2;
	NetBuffer netbuf;
	string file_content;
	ClientPtr clt = this->getClientFromSerial( serial);

	// Send a CMD_JUMP to tell the client if the jump is allowed
	netbuf.addString( clt->jumpfile );

	// And remove the player from its old starsystem and set it out of game
	this->removeClient( clt );
	// Have to set new starsystem here
	// ??????

	// Test whether the jump was accepted or not by server
	if( ok)
	{
		// If jumpfile is empty the hash was correct
		if( clt->jumpfile=="" )
			p2.send( CMD_JUMP, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
		// New system file HASH is wrong tell the client with serial != player serial so he can ask for a new download
		else
			p2.send( CMD_JUMP, serial+1, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
	}
	else if( !ok || clt->jumpfile=="error")
		p2.send( CMD_JUMP, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
}

void	NetServer::sendDockAuthorize( ObjSerial serial, ObjSerial utdw_serial, int docknum, unsigned short zone)
{
	NetBuffer netbuf;
	Packet p;
	// Send a CMD_DOCK with serial, an ObjSerial = unit_to_dock_with_serial and an int = docking port num
	netbuf.addSerial( utdw_serial);
	netbuf.addInt32( docknum);
	p.bc_create( CMD_DOCK, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(118) );
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendDockDeny( ObjSerial serial, unsigned short zone)
{
	// In fact do nothing
}

void	NetServer::sendUnDock( ObjSerial serial, ObjSerial utdwserial, unsigned short zone)
{
	// SEND A CMD_UNDOCK TO OTHER CLIENTS IN THE ZONE with utdw serial
	NetBuffer netbuf;
	Packet p;
	netbuf.addSerial( utdwserial);
	p.bc_create( CMD_UNDOCK, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(134) );
	zonemgr->broadcastNoSelf( zone, serial, &p );
}

