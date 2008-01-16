#include "networking/netserver.h"
#include "networking/savenet_util.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/vsnet_sockethttp.h"
#include "lin_time.h"
#include "save_util.h"
#include "universe_util.h"

extern StarSystem * GetLoadedStarSystem( const char * system);

// WEAPON STUFF

void	NetServer::BroadcastTarget( ObjSerial serial, ObjSerial oldtarget, ObjSerial target, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;
	netbuf.addSerial( target);
	ClientPtr clt = this->getClientFromSerial(target);
	if (clt) {
		p.send( CMD_TARGET, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
			 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(24) );
	}

	netbuf.Reset();
	netbuf.addSerial( 0 );
	clt = this->getClientFromSerial(oldtarget);
	if (clt) {
		p.send( CMD_TARGET, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
				 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(34));
	}

	clt = this->getClientFromSerial(serial);
	if (clt && clt->netversion < 4951) {
		// Old clients needed confirmation to target...
		netbuf.Reset();
		netbuf.addSerial( target);
		p.send( CMD_TARGET, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
				 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(34));
	}
	
	
	/*
	netbuf.addSerial( target);
	p.bc_create( CMD_TARGET, serial,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(26) );
	zonemgr->broadcast( zone, serial, &p, true ); // NETFIXME: Should unfire be TCP?
	*/
}

void	NetServer::BroadcastUnfire( ObjSerial serial, const vector<int> &weapon_indicies, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;

	//netbuf.addSerial( serial);
	netbuf.addInt32( weapon_indicies.size());
	for (unsigned int i=0;i<weapon_indicies.size();i++) {
		netbuf.addInt32( weapon_indicies[i] );
	}

	//p.send( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, __LINE__);
	p.bc_create( CMD_UNFIREREQUEST, serial,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(1695) );
	zonemgr->broadcast( zone, serial, &p, true ); // NETFIXME: Should unfire be TCP?
}

// In BroadcastFire we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetServer::BroadcastFire( ObjSerial serial, const vector<int> &weapon_indicies, ObjSerial missile_serial, float player_energy, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;
	// bool found = false;

	netbuf.addFloat( player_energy );
	netbuf.addSerial( missile_serial);
	netbuf.addInt32( weapon_indicies.size());
	for (unsigned int i=0;i<weapon_indicies.size();i++) {
		netbuf.addInt32( weapon_indicies[i] );
	}

	p.bc_create( CMD_FIREREQUEST, serial,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(1710) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED -> SHOULD USE broadcastNoSelf instead if we dont want that
	zonemgr->broadcast( zone, serial, &p, true ); // NETFIXME: Should unfire be TCP?
}

void NetServer::BroadcastCargoUpgrade(ObjSerial sender, ObjSerial buyer, ObjSerial seller, const std::string &cargo,
				float price, float mass, float volume, bool mission, unsigned int quantity,
				int mountOffset, int subunitOffset, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( buyer ); // If the buyer is the player, it is buying cargo.
	netbuf.addSerial( seller ); // If seller is the player, it is selling cargo.
	netbuf.addInt32( quantity );
	netbuf.addString( cargo );
	netbuf.addFloat( price );
	netbuf.addFloat( mass );
	netbuf.addFloat( volume );
	if (mission) {
		netbuf.addInt32(1);
		netbuf.addInt32(1);
	} else {
		netbuf.addInt32( (unsigned int)(mountOffset) );
		netbuf.addInt32 ((unsigned int)(subunitOffset) );
	}
	
	p.bc_create( CMD_CARGOUPGRADE, sender,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(87) );
	zonemgr->broadcast( zone, sender, &p, true ); // NETFIXME: Should unfire be TCP?
}

void NetServer::sendCredits(ObjSerial serial, float creds) {
	Packet p;
	NetBuffer netbuf;
	netbuf.addFloat(creds);
	ClientPtr clt = this->getClientFromSerial(serial);
	if (!clt) return;
	p.send( CMD_CREDITS, serial,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,NULL,clt->tcp_sock,
                 __FILE__, PSEUDO__LINE__(97) );
}

void	NetServer::addUnitCargoSnapshot( const Unit *un, NetBuffer &netbuf) {
	unsigned int numCargo = un->numCargo();
	if (!numCargo) return;
	netbuf.addSerial(un->GetSerial());
	netbuf.addFloat(un->Mass);
	netbuf.addFloat(un->image->CargoVolume);
	netbuf.addFloat(un->image->UpgradeVolume);
	netbuf.addInt32(numCargo);
	for (unsigned int i=0;i<numCargo;i++) {
		const Cargo &carg = un->GetCargo(i);
		netbuf.addInt32(carg.GetQuantity());
		netbuf.addString(carg.GetContent());
		netbuf.addFloat(carg.GetPrice());
		netbuf.addFloat(carg.GetMass());
		netbuf.addFloat(carg.GetVolume());
	}
}

void	NetServer::sendCargoSnapshot( ObjSerial cltser, const UnitCollection &list) {
	ClientPtr clt = this->getClientFromSerial(cltser);
	if (!clt) return;
	NetBuffer netbuf;
	Packet p2;
	const Unit *un;
	for (un_kiter iter = list.constIterator(); (un = *iter); ++iter) {
		if (!un->GetSerial() || un->GetSerial()==cltser) continue;
		addUnitCargoSnapshot(un, netbuf);
	}
	netbuf.addSerial(0);
	p2.send( CMD_SNAPCARGO, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
			 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(196) );
}

void NetServer::sendSaveData( int cp, unsigned short type, int pos, string *key,
				  Mission *miss, string *strValue, float *floatValue) {
	/* Note to self: This function will do absolutely nothing
	   until it is implemented. */
	NetBuffer netbuf;
	Packet p2;
	Unit *un = _Universe->AccessCockpit(cp)->GetParent();
	if (!un) return;
	ClientPtr clt = this->getClientFromSerial(un->GetSerial());
	if (!clt) return;
	
	netbuf.addShort(type);
	if ((type&Subcmd::StringValue) ||(type&Subcmd::FloatValue)) {
		netbuf.addString(*key);
	}
	netbuf.addInt32(pos);
	if (type&Subcmd::Objective) {
		int num = miss->getPlayerMissionNumber();
		netbuf.addInt32(num-1);
	}
	if (type&Subcmd::SetValue) {
		if ((type&Subcmd::StringValue) || (type&Subcmd::Objective)) {
			netbuf.addString(*strValue);
		}
		if ((type&Subcmd::FloatValue) || (type&Subcmd::Objective)) {
			netbuf.addFloat(*floatValue);
		}
	}
	p2.send( CMD_SAVEDATA, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
			 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(164) );
}

void	NetServer::sendMission( int cp, unsigned short packetType, string mission, int pos)
{
	Packet p;
	NetBuffer netbuf;
	Unit *un = _Universe->AccessCockpit(cp)->GetParent();
	if (!un) return;
	ClientPtr clt = this->getClientFromSerial(un->GetSerial());
	if (!clt) return;
	
	netbuf.addShort(packetType);
	netbuf.addString(mission);
	netbuf.addInt32(pos);
	
	p.send(CMD_MISSION, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
		   NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(186));
}

void	NetServer::sendDamages( ObjSerial serial, unsigned short zone, float hull, const Shield &shield, const Armor &armor,
				float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color)
{
  static ObjSerial lastserial;
  static float timestamp=getNewTime();
  float curtime=getNewTime();
  static float mintime=XMLSupport::parse_float(vs_config->getVariable( "network", "min_time_btw_damage_updates", ".25" ));

  if (lastserial!=serial||curtime-timestamp>mintime) {
    timestamp=curtime;
    lastserial=serial;
	Packet p;
	NetBuffer netbuf;

	netbuf.addFloat( amt);
	netbuf.addFloat( ppercentage);
	netbuf.addFloat( spercentage);
	netbuf.addVector( pnt);
	netbuf.addVector( normal);
	netbuf.addColor( color);
	netbuf.addFloat( hull);
	netbuf.addShield( shield);
	netbuf.addArmor( armor);

	p.bc_create( CMD_DAMAGE, serial,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(1729) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p, false ); // NETFIXME: Should damages be TCP? NO..we have alternate method to deal with it
  }
}

void	NetServer::sendCustom( int cp, string command, string args, string id)
{
	Packet p2;
	NetBuffer netbuf;

	Unit *un = _Universe->AccessCockpit(cp)->GetParent();
	if (!un) {
		fprintf(stderr, "Attempt to sendCustom NULL player %d ; CMD %s %s ; ID %s\n",
				cp, command.c_str(), args.c_str(), id.c_str());
		return;
	}
	ClientPtr clt = this->getClientFromSerial(un->GetSerial());
	if (!clt) return;
	
	netbuf.addString(command);
	netbuf.addString(args);
	netbuf.addString(id);
	
	p2.send ( CMD_CUSTOM, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
			  NULL, clt->tcp_sock, __FILE__, __LINE__ );
}

void	NetServer::sendMessage( string from, string to, string message, float delay )
{
	NetBuffer netbuf;
	netbuf.addString(from);
	netbuf.addString(message);
	Packet p2;
	
	const char *tostr (to.c_str());
	if (to[0]=='p') {
		int playerto=-1;
		sscanf(tostr+1,"%d",&playerto);
		if (playerto!=-1 && playerto<_Universe->numPlayers()) {
			Cockpit *cp = _Universe->AccessCockpit(playerto);
			Unit *un = cp->GetParent();
			if (!un) return;
			ClientPtr clt = this->getClientFromSerial(un->GetSerial());
			if (!clt) return;

			p2.send( CMD_TXTMESSAGE, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
			 NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(223) );
		}
	} else {
		p2.bc_create( CMD_TXTMESSAGE, 0,
                 netbuf.getData(), netbuf.getDataLength(),
                 SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(229) );
		if (to == "broadcast") {
			for (ZoneMap::const_iterator iter=zonemgr->zones.begin();
				 iter!=zonemgr->zones.end();++iter)
				zonemgr->broadcast( (*iter).first, 0, &p2, false );
		} else {
			zonemgr->broadcast( _Universe->activeStarSystem()->GetZone(), 0, &p2, false);
		}
	}
}

void	NetServer::sendKill( ObjSerial serial, unsigned short zone)
{
	Packet p;
	Unit * un;

	cerr<<"SENDING A KILL for serial "<<serial<<" in zone "<<zone<<endl;
	// Find the client in the udp & tcp client lists in order to set it out of the game (not delete it yet)
	ClientPtr clt = this->getClientFromSerial( serial);
	if (clt) {
		un = clt->game_unit.GetUnit();
	} else {
		un = zonemgr->getUnit(serial, zone);
	}
	// It's the server's responsibility to kill missions in this case.
	// NETFIXME: Note that the client might not hear the request to terminate missions.
	int cp = _Universe->whichPlayerStarship(un);
	if (cp>=0) {
		// Note 1 not 0 is to avoid killing the player's main mission.
		Mission *mis;
		while ((mis = Mission::getNthPlayerMission(cp, 1))) {
			mis->terminateMission();
		}
	}
	
	p.bc_create( CMD_KILL, serial,
                 NULL, 0, SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(1771) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p, true );

	if( !clt )
	{
		COUT<<"Killed a non client Unit = "<<serial<<endl;
		/*
		un = zonemgr->getUnit( serial, zone);
		if (un) {
			zonemgr->removeUnit( un, zone);
		}
		*/
	}
	else
	{
		COUT<<"Killed client serial = "<<serial<<endl;
		zonemgr->removeClient( clt );
	}
}

void    NetServer::sendJumpFinal(ClientPtr clt) {
    Packet p2;
	p2.send(CMD_JUMP,0,NULL,0,SENDANDFORGET,NULL,clt->tcp_sock,__FILE__,148);
	if (clt&&0/*dont discon until client requests it*/)
		logoutList.push_back(clt);
}

void	NetServer::sendJump(Unit * un, Unit * dst,std::string dststr)
{
  ClientPtr clt = this->getClientFromSerial(un->GetSerial());
  if (!clt) return;
  int cpnum = -1;
  if (clt&&(cpnum=_Universe->whichPlayerStarship(un))>=0&&un!=NULL) {
    WaitListEntry entry;
    if (waitList.find(clt->callsign)!=waitList.end()) {
      return;
    }
    Cockpit *cp = _Universe->AccessCockpit(cpnum);
    std::string netbuf;
    std::string fn=dststr;
    if (getSaveStringLength(cpnum,"jump_from")<1) {
	  pushSaveString(cpnum, "jump_from", _Universe->activeStarSystem()->getFileName());
    }else {
      putSaveString(cpnum, "jump_from", 0, _Universe->activeStarSystem()->getFileName());
    }
    std::string savestr;
	std::string csvstr;
	SaveNetUtil::GetSaveStrings( cpnum, savestr, csvstr, true);
	/*
	savestr = cp->savegame->WriteSaveGame (fn.c_str(),un->LocalPosition(),cp->credits,cp->unitfilename,-1,FactionUtil::GetFactionName(un->faction), false);
	csvstr = un->WriteUnitString();
	*/
    std::string::size_type where=savestr.find("^");
    addSimpleChar(netbuf,ACCT_SAVE_LOGOUT);
    
    if (where!=std::string::npos) {
      if (dststr!=savestr.substr(0,where)) {
        savestr=dststr+savestr.substr(where);
      }
    }
    addSimpleString(netbuf, clt->callsign );
    addSimpleString(netbuf, clt->passwd );
    addSimpleString(netbuf,savestr);
    addSimpleString(netbuf,csvstr );
    clt->jumpok=1;

	entry.type = WaitListEntry::JUMPING;
	entry.t = clt;
	entry.tcp = true;
	waitList[clt->callsign] = entry;
	clt->loginstate = Client::LOGGEDIN;
	
    if (acct_sock)
      acct_sock->sendstr(netbuf);

   }else {
    // do something intelligent for NPCs
    
  }
  
  /*
  if (0) {
	Packet p2;
	NetBuffer netbuf;
	string file_content;
	ClientPtr clt = this->getClientFromSerial( serial);
        if (!clt) {
          cerr<<"Client "<< serial<<" Trying to jump but cannot retrieve client struct \n";
        }
	// Send a CMD_JUMP to tell the client if the jump is allowed
	netbuf.addString( clt->jumpfile );
	netbuf.addSerial( serial);
	netbuf.addSerial( jumpserial);

	// DO NOT DO THAT : the client itself will ask to be removed and also to be added when it is allowed to do so
	// And remove the player from its old starsystem and set it out of game
	//this->removeClient( clt );
	// Have to set new starsystem here
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	cp->savegame->SetStarSystem( clt->jumpfile);
	StarSystem* sts = GetLoadedStarSystem( clt->jumpfile.c_str());
	unsigned short zoneid = _Universe->StarSystemIndex( sts);
	netbuf.addShort( zoneid);

	// Test whether the jump was accepted or not by server
	if( ok)
	{
		// If jumpfile is empty the hash was correct
		if( clt->jumpokOLD==1 )
			p2.bc_create( CMD_JUMP, serial,
                          netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1164) );
		// New system file HASH is wrong tell the client with serial != player serial so he can ask for a new download
		else if( clt->jumpokOLD==2)
			p2.bc_create( CMD_JUMP, serial+1,
                          netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1164) );
		zonemgr->broadcast( clt, &p2, true );
	}
*/

	// Should broadcast JUMP so other client display jump anim too ?
}

// NETFIXME: AI Units won't be removed when docking... this may be desired but may be confusing for clients.

void	NetServer::sendDockAuthorize( ObjSerial serial, ObjSerial utdw_serial, int docknum, unsigned short zone)
{
	// Set client not ingame while docked
	ClientPtr clt = this->getClientFromSerial( serial);
	if (!clt) {
		cerr << "Client " << serial << " for dock autorization is NULL." << endl;
		return;
	}
	Unit * un=clt->game_unit.GetUnit();
	if (!un)
		return;
//	StarSystem * currentsys = un->getStarSystem();
	// Remove the unit from the system list
//	currentsys->RemoveUnit( un );

	NetBuffer netbuf;
	Packet p;
	// Send a CMD_DOCK with serial, an ObjSerial = unit_to_dock_with_serial and an int = docking port num
	netbuf.addSerial( utdw_serial);
	netbuf.addInt32( 0); //docknum);
	p.bc_create( CMD_DOCK, serial,
                 netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(118) );
	zonemgr->broadcast( zone, serial, &p, true );

	// Whoops... Make sure to de-ingame the client *AFTER* the CMD_DOCK has been broadcast!
	clt->ingame = false;
	// Set timestamps to 0 so we won't have prediction problem when undocking
	clt->clearLatestTimestamp();
}

void	NetServer::sendDockDeny( ObjSerial serial, unsigned short zone)
{
	// In fact do nothing
}

void	NetServer::sendForcePosition( ClientPtr clt )
{
	Unit *un = clt->game_unit.GetUnit();
	if (!un)
		return;
	
	ClientState cs (un);

	Packet p;
	NetBuffer netbuf;
	if (clt->netversion<4951) {
		netbuf.addQVector(cs.getPosition());
	} else {
		netbuf.addClientState(cs);
	}
	
	p.send(CMD_POSUPDATE, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
		   NULL, clt->tcp_sock, __FILE__, PSEUDO__LINE__(457));
}

void	NetServer::sendUnDock( ObjSerial serial, ObjSerial utdwserial, unsigned short zone)
{
	// Set client ingame
	ClientPtr clt = this->getClientFromSerial( serial);
	if (!clt) {
		cerr << "Client " << serial << " for dock autorization is NULL." << endl;
		return;
	}
	Unit *un = clt->game_unit.GetUnit();
	if (!un)
		return;
	clt->ingame = true;
	// Add the unit back into the system list
//	StarSystem * currentsys = un->getStarSystem();
//	currentsys->AddUnit( un );
	
	// SEND A CMD_UNDOCK TO OTHER CLIENTS IN THE ZONE with utdw serial
	NetBuffer netbuf;
	Packet p;
	netbuf.addSerial( utdwserial);
	p.bc_create( CMD_UNDOCK, serial,
                 netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(134) );
	zonemgr->broadcast( zone, serial, &p, true );

	static bool autolanding_enable=XMLSupport::parse_bool(vs_config->getVariable("physics","AutoLandingEnable","false"));
	if (!autolanding_enable) {
		sendForcePosition(clt);
	}
}

