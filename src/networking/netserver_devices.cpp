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
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
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
		// If jumpfile is empty the md5 was correct
		if( clt->jumpfile=="" )
			p2.send( CMD_JUMP, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
		// New system file MD5 is wrong tell the client with serial != player serial so he can ask for a new download
		else
			p2.send( CMD_JUMP, serial+1, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
	}
	else if( !ok || clt->jumpfile=="error")
		p2.send( CMD_JUMP, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
}

