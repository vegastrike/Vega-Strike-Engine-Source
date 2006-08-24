
#include "lin_time.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/client.h"
#include "networking/netclient.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/zonemgr.h"
#include "networking/lowlevel/packet.h"
#include "networking/lowlevel/vsnet_clientstate.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "networking/prediction.h"
#include "networking/fileutil.h"
#include "networking/lowlevel/vsnet_notify.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"

/*************************************************************/
/**** Adds an entering client in the actual zone          ****/
/*************************************************************/

void	NetClient::enterClient( NetBuffer &netbuf, ObjSerial cltserial )
{
	// Should receive the name
	string cltname = netbuf.getString();
	string savestr = netbuf.getString();
	string xmlstr = netbuf.getString();
	Transformation trans = netbuf.getTransformation();
	// If not a local player, add it in our array
        Unit * shouldbenull=UniverseUtil::GetUnitFromSerial(cltserial);
          //NETFIXME could be slow--but alas
        if (NULL!=shouldbenull) {
          cout << " not adding unit with serial number "<< cltserial<<" named " <<shouldbenull->name<<" to system .";
          return;//already exists
        }
	if( !isLocalSerial( cltserial))
	{

		// We will ignore - starsys as if a client enters he is in the same system
		//                - pos since we received a ClientState
		//                - creds as we don't care about other players' credits for now
		string starsys;
		QVector pos;
		float creds;
		bool update=true;
		vector<string> savedships;
                string PLAYER_FACTION_STRING;
                string * savegamestr=NULL;
                if (cltname.length()) {
		// Parse the save buffer
                  save.ParseSaveGame( "", starsys, "", pos, update, creds, savedships, 0, savestr, false);
                  
                  PLAYER_FACTION_STRING= save.GetPlayerFaction();
                  
                  // CREATES THE UNIT... GET SAVE AND XML FROM SERVER
                  // Use the first ship if there are more than one -> we don't handle multiple ships for now
                  // We name the flightgroup with the player name
                  cerr<<"Found saveship[0] = "<<savedships[0]<<endl;
                  cerr<<"NEW PLAYER POSITION : x="<<pos.i<<",y="<<pos.j<<"z="<<pos.k<<endl;
                  
                  cerr<<"SAFE PLATER POSITION: x="<<pos.i<<",y="<<pos.j<<"z="<<pos.k<<endl;
                  savegamestr=&xmlstr;
		}else {
                  std::string::size_type wherepipe=xmlstr.find("|");
                  PLAYER_FACTION_STRING=((wherepipe!=string::npos)?xmlstr.substr(0,wherepipe):xmlstr);
                  
                  if (savestr=="Pilot"||savestr=="pilot")
                    savestr="eject";
                  else if (savestr.find(".cargo")==string::npos&&PLAYER_FACTION_STRING=="upgrades") {
                    savestr="generic_cargo";
                  }
                  savedships.push_back(savestr);
                  if (wherepipe!=string::npos) {
                    cltname=xmlstr.substr(wherepipe+1);
                  }
                  if (wherepipe==string::npos||cltname.length()==0){
                    cltname = "Object_"+XMLSupport::tostring(cltserial);
                  }
                }
		
		Unit * un = UnitFactory::createUnit( savedships[0].c_str(),
							 false,
							 FactionUtil::GetFactionIndex( PLAYER_FACTION_STRING),
							 string(""),
							 Flightgroup::newFlightgroup ( cltname,savedships[0],PLAYER_FACTION_STRING,"default",1,1,"","",mission),
							 0, savegamestr);
		ClientPtr clt = this->AddClientObject( un, cltserial);

		// Assign new coordinates to client
		un->SetPosition( trans.position );
		un->curr_physical_state=trans;
		un->BackupState();
		clt->last_packet=un->old_state;
		clt->prediction->InitInterpolation(un, un->old_state, 0, this->deltatime);

		save.SetPlayerLocation(un->curr_physical_state.position);
		clt->name = cltname;
                clt->callsign=cltname;
		string msg = clt->callsign+" entered the system";
		UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
	}
}

ClientPtr NetClient::AddClientObject( Unit *un, ObjSerial cltserial)
{
	ClientPtr clt;

	clt = Clients.get(cltserial);
	if( clt)
	{
		// Client may exist if it jumped from a starsystem to another of if killed and respawned
		COUT<<"Existing client n°"<<cltserial<<endl;
	}
	else
	{
		clt = Clients.insert( cltserial, new Client );
		nbclients++;
		COUT<<"New client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	}
	if( !isLocalSerial( cltserial))
	{
		clt->game_unit.SetUnit( un);
		// Set all weapons to inactive
		vector <Mount>
			::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
		for (;i!=un->mounts.end();++i)
			(*i).status=Mount::INACTIVE;
		un->SetNetworkMode( true);
		un->SetSerial( cltserial);
		//COUT<<"Addclient 4"<<endl;
		un->SetPosition( QVector(0,0,0) );
		un->curr_physical_state=Transformation();
		un->BackupState();
		clt->last_packet=un->old_state;
//		clt->prediction->InitInterpolation(un, un->old_state, 0, this->deltatime);

		_Universe->activeStarSystem()->AddUnit( un);
	
	} else {
		Unit *myun = this->game_unit.GetUnit();
		Unit *cltun = clt->game_unit.GetUnit();
		if( cltun==NULL || cltserial!=cltun->GetSerial())
		{
			clt->game_unit.SetUnit( un?un:getNetworkUnit( cltserial));
		}
		if( myun==NULL || cltserial!=myun->GetSerial())
		{
			this->game_unit.SetUnit( un?un:getNetworkUnit( cltserial));
		}
                
	}
	return clt;
}

/*************************************************************/
/**** Ask the server for the entering zone info           ****/
/*************************************************************/

void	NetClient::downloadZoneInfo()
{
	char tbuf[1024];
	sprintf( tbuf, "%d", this->zone);
    COUT << __PRETTY_FUNCTION__ << " zone: " << tbuf << endl;
	VsnetDownload::Client::Buffer buf( this->clt_tcp_sock, tbuf, VSFileSystem::ZoneBuffer);
	_downloadManagerClient->addItem( &buf);
	timeval timeout={10,0};
	while( !buf.done())
	{
		if (recvMsg( NULL, &timeout )<=0) {
			//NETFIXME: What if timeout or error occurs?
			break;
		}
	}
	const char * tmp = (char *) buf.getBuffer().get();
	NetBuffer netbuf( tmp, buf.getSize());
	this->AddObjects( netbuf);
}

void	NetClient::AddObjects( NetBuffer & netbuf)
{
	char subcmd;
        std::set<ObjSerial> enteredSerials;
        for (unsigned int i=0;i<_Universe->numPlayers();++i) {
          Unit*un=_Universe->AccessCockpit(i)->GetParent();
          if(un) enteredSerials.insert(un->GetSerial());
        }
	// Loop until the end of the buffer
	while( (subcmd=netbuf.getChar())!=ZoneMgr::End)
	{
		switch( subcmd)
		{
			case ZoneMgr::AddClient :
			{
                          
                          ObjSerial serial = netbuf.getSerial();
                          enteredSerials.insert(serial);
                          this->enterClient( netbuf, serial);
			}
			break;
			default :
				cerr<<"WARNING : Unknown sub command in AddObjects"<<endl;
		}
	}
        Unit *un;
        for (un_iter it = UniverseUtil::getUnitList();
             un=(*it);
             ) {
          ++it;
          if (enteredSerials.find(un->GetSerial())==enteredSerials.end()) {            
            un->Kill();//doesnt belong here..not an allowed serial
            //NETFIXME could result in star system being killed off one by one--need to differentiate that
          }
        }

}

/*************************************************************/
/**** Removes an exiting client of the actual zone        ****/
/*************************************************************/

void	NetClient::removeClient( const Packet* packet )
{
	ObjSerial	cltserial = packet->getSerial();

	COUT<<" & HTONS(Serial) = "<<cltserial<<endl;
	ClientPtr clt;
	if( !(clt=Clients.get(cltserial)))
	{
		cerr<<"Error, client does not exists !!"<<endl;
		return;
//		exit( 1);
	}
	Unit * un = clt->game_unit.GetUnit();
	if (un) {
		// Removes the unit from starsystem, destroys it and delete client
		_Universe->activeStarSystem()->RemoveUnit( un );
	}
	nbclients--;
	Clients.remove(cltserial);
	if (un) {
		un->Kill();
	}
	COUT<<"Leaving client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	string msg = clt->callsign+" left the system";
	UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
}

/*************************************************************/
/**** Send an update to server                            ****/
/*************************************************************/

void	NetClient::sendPosition( const ClientState* cs )
{
// NETFIXME: POSUPDATE's need to happen much more often, and should send info more onften about closer units than farther ones.



	Unit *un = this->game_unit.GetUnit();
	if (!un)
		return;
	// Serial in ClientState is updated in UpdatePhysics code at ClientState creation (with pos, veloc...)
	Packet pckt;
	NetBuffer netbuf;
	static bool debugPos = XMLSupport::parse_bool(vs_config->getVariable("network", "debug_position_interpolation", "false"));

	// Send the client state
	if (debugPos) COUT<<"Sending ClientState == ";
	if (debugPos) (*cs).display();
	netbuf.addSerial( cs->getSerial());
	netbuf.addClientState( (*cs));
	pckt.send( CMD_POSUPDATE, un->GetSerial(),
               netbuf.getData(), netbuf.getDataLength(),
               SENDANDFORGET, NULL, *this->lossy_socket,
               __FILE__, PSEUDO__LINE__(218) );
}

/**************************************************************/
/**** Update another client position                       ****/
/**************************************************************/

void NetClient::receivePositions( unsigned int numUnits, unsigned int int_ts, NetBuffer& netbuf, double delta_t )
{
	// Computes deltatime only when receiving a snapshot since we interpolate positions between 2 snapshots
	// We don't want to consider a late snapshot
//	cout << "netSnapshot ";
	static bool debugPos = XMLSupport::parse_bool(vs_config->getVariable("network", "debug_position_interpolation", "false"));
	if( latest_timestamp < int_ts)
	{
        if (debugPos) COUT << "   *** SNAPSHOT is not late - evaluating" << endl;

		this->latest_timestamp = int_ts;
		this->deltatime        = delta_t;

		// Loop throught received snapshot
        int i = 0;
        int j = 0;
		while( (i+j)<numUnits )
		{
            ObjSerial       sernum = 0;
            unsigned char   cmd;
	        bool			localplayer = false;
	        ClientPtr		clt;
	        Unit *			un = NULL;

			// Get the ZoneMgr::SnapshotSubCommand from buffer
			cmd    = netbuf.getChar();

			// Get the serial number of current element
			sernum = netbuf.getShort();

			// First test if it is us
			if( sernum == this->serial)
            {
				localplayer = true;
            }
			if( !(clt = Clients.get(sernum)))
			{
			    // Test if it is a client or a unit
				if( !(un = UniverseUtil::GetUnitFromSerial( sernum)))
				{
					COUT<<"WARNING : No client, no unit found for this serial ("<<sernum<<")"<<endl;
				}
			}
			// Test if local player
			else
            {
				un = clt->game_unit.GetUnit();
				if (un)
					localplayer = _Universe->isPlayerStarship(un)?true:false;
            }

			if( cmd & ZoneMgr::FullUpdate )
			{
                if (debugPos) COUT << "   *** SubCommand is FullUpdate ser=" << sernum << endl;

	            ClientState	cs;
				float newEnergy;

				// Do what needed with update
				if (debugPos) COUT<<"Received ZoneMgr::FullUpdate ";
				// Tell we received the ClientState so we can convert byte order from network to host
				cs = netbuf.getClientState();
				newEnergy = netbuf.getFloat();
                if (debugPos) COUT << "   *** cs=" << cs << endl;

// NETFIXME: Why not set local player? It can't hurt...
				
				if( un )
				{
					if (!localplayer) {
						// Get our "semi-ping" from server
						// We received delay in ms so we convert it into seconds
						// Backup old state
						un->BackupState();
						// Update concerned client with predicted position directly in network client list
						un->curr_physical_state.position = cs.getPosition();
						un->curr_physical_state.orientation = cs.getOrientation();
						un->Velocity = cs.getVelocity();
						if (clt) {
							if(clt->last_packet.getPosition()==QVector(0,0,0)) {
								// Position previously uninitialized...
								un->BackupState();
								clt->last_packet=cs;
							}
							clt->setLatestTimestamp(int_ts);
							clt->elapsed_since_packet = 0;
							assert( clt->prediction );
							clt->prediction->InitInterpolation( un,
								clt->last_packet,
								clt->getDeltatime(),
								clt->getNextDeltatime());
							clt->last_packet=cs;
							un->curr_physical_state = clt->prediction->Interpolate( un, 0);
						}
					
						QVector predpos = un->curr_physical_state.position;
						if (debugPos) cerr<<"Predicted location : x="<<predpos.i<<",y="<<predpos.j<<",z="<<predpos.k<<endl;
					}
					// Everyone gets energy updates.
					un->energy = newEnergy;
				}
				i++;
			}
			else if( cmd & ZoneMgr::PosUpdate )
			{
                if (debugPos) COUT << "   *** SubCommand is PosUpdate ser=" << sernum << endl;
				QVector pos = netbuf.getQVector();
                if (debugPos) COUT << "   *** pos=" << pos.i << "," << pos.j << "," << pos.k << endl;
				if( un && (!localplayer) )
				{
					// Backup old state
					un->BackupState();
					// Set the new received position in curr_physical_state
					un->curr_physical_state.position = pos;
					if (clt) {
						if(clt->last_packet.getPosition()==QVector(0,0,0)) {
							// Position previously uninitialized...
							un->BackupState();
							clt->last_packet.setPosition(pos);
						}
						clt->setLatestTimestamp(int_ts);
						clt->elapsed_since_packet = 0;
						assert( clt->prediction );
						clt->prediction->InitInterpolation( un,
								clt->last_packet,
								clt->getDeltatime(),
								clt->getNextDeltatime());
						clt->last_packet=un->old_state;
						clt->last_packet.setPosition(pos);
						un->curr_physical_state.position = clt->prediction->InterpolatePosition( un, 0);
					}
				}
				else if( localplayer)
                {
					if (debugPos) cerr<<" IGNORING LOCAL PLAYER"<<endl;
                }
				j++;
			}
			if ( cmd & ZoneMgr::DamageUpdate ) {
				cout << "Received damage info for client "<< serial << endl;
				receiveUnitDamage( netbuf, un );
			}
            
			// NETFIXME: Not an exit condition.  Just print a warning message and ignore the rest of the packet.
			// NETFIXME: we should include a length field for each sub-packet so that we can safely ignore a part of the packet.
            //    COUT << "   *** SubCommand is neither FullUpdate nor PosUpdate" << endl;
			//	return;
//                     << "   *** TERMINATING ***" << endl;
//                VSExit( 1 );
		}
	}
    else
    {
        COUT << "   *** SNAPSHOT is late - ignoring" << endl;
    }
}

void NetClient::receiveUnitDamage( NetBuffer &netbuf, Unit *un ) {
	size_t it=0;
        if (!un) {
          cerr<< "Received Damage Update for null unit"<<endl;
          return;
        }
	unsigned short damages;
	damages = netbuf.getShort();

	if( damages & Unit::SHIELD_DAMAGED)
	{
		un->shield = netbuf.getShield();
	}
	if( damages & Unit::ARMOR_DAMAGED)
	{
		un->armor = netbuf.getArmor();
	}
	if( damages & Unit::COMPUTER_DAMAGED)
	{
		un->computer.itts = netbuf.getChar();
		un->computer.radar.iff = netbuf.getChar();
		un->limits.retro = netbuf.getFloat();
		un->computer.radar.maxcone = netbuf.getFloat();
		un->computer.radar.lockcone = netbuf.getFloat();
		un->computer.radar.trackingcone = netbuf.getFloat();
		un->computer.radar.maxrange = netbuf.getFloat();
		for( it = 0; it<1+UnitImages::NUMGAUGES+MAXVDUS; it++)
			un->image->cockpit_damage[it] = netbuf.getFloat();
	}
	if( damages & Unit::MOUNT_DAMAGED)
	{
		un->image->ecm = netbuf.getShort();
		for( it=0; it<un->mounts.size(); it++)
		{
                  Mount::STATUS tmpstatus=( Mount::STATUS) netbuf.getChar();
                  if (_Universe->isPlayerStarship(un)==NULL||tmpstatus==Mount::UNCHOSEN||tmpstatus==Mount::DESTROYED) {
                    un->mounts[it].status = tmpstatus;
                        //don't reset my mount status on me... that's my 
                  }                  
                  un->mounts[it].ammo = netbuf.getInt32();
                  un->mounts[it].time_to_lock = netbuf.getFloat();
                  un->mounts[it].size = netbuf.getShort();
		}
	}
	if( damages & Unit::CARGOFUEL_DAMAGED)
	{
		un->SetFuel( netbuf.getFloat());
		un->SetAfterBurn(netbuf.getFloat());
		un->image->CargoVolume = netbuf.getFloat();
		un->image->UpgradeVolume = netbuf.getFloat();
		// NRTFIXME: cargo unimplented.
//		for( it=0; it<un->image->cargo.size(); it++)
//			un->image->cargo[it].quantity = netbuf.getInt32();
	}
	if( damages & Unit::JUMP_DAMAGED)
	{
		un->shield.leak = netbuf.getChar();
		un->shield.recharge = netbuf.getFloat();
		un->SetEnergyRecharge( netbuf.getFloat());
		un->SetMaxEnergy( netbuf.getFloat());
		un->jump.energy = netbuf.getFloat();
		un->jump.damage = netbuf.getChar();
		un->image->repair_droid = netbuf.getChar();
	}
	if( damages & Unit::CLOAK_DAMAGED)
	{
		un->cloaking = netbuf.getInt32();
		un->image->cloakenergy = netbuf.getFloat();
		un->cloakmin = netbuf.getInt32();
	}
	if( damages & Unit::LIMITS_DAMAGED)
	{
		un->computer.max_pitch_down = netbuf.getFloat( );
		un->computer.max_pitch_up = netbuf.getFloat( );
		un->computer.max_yaw_left = netbuf.getFloat( );
		un->computer.max_yaw_right = netbuf.getFloat( );
		un->computer.max_roll_left = netbuf.getFloat( );
		un->computer.max_roll_right = netbuf.getFloat( );
		un->limits.roll = netbuf.getFloat( );
		un->limits.yaw = netbuf.getFloat( );
		un->limits.pitch = netbuf.getFloat( );
		un->limits.lateral = netbuf.getFloat( );
	}
}

/*************************************************************/
/**** Says we are ready and going in game                 ****/
/*************************************************************/

void	NetClient::inGame()
{
	Packet    packet2;
	NetBuffer netbuf;
    char      flags = 0;
	Unit    * un = this->game_unit.GetUnit();

	//ClientState cs( this->serial, un->curr_physical_state, un->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !! NOT NEEDED ANYMORE -> THE SERVER ALREADY KNOWS
	//netbuf.addClientState( cs);
	packet2.send( CMD_ADDCLIENT, this->serial,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->clt_tcp_sock,
                  __FILE__, PSEUDO__LINE__(392) );
	un->SetSerial( this->serial);
	COUT << "Sending ingame with serial n°" << this->serial << endl;
	this->ingame = true;
	cerr<<"STARTING LOCATION : x="<<un->Position().i<<",y="<<un->Position().j<<",z="<<un->Position().k<<endl;
}

/*************************************************************/
/**** Says we are still alive                             ****/
/*************************************************************/

void NetClient::sendAlive()
{
    /* WE NEED PING PACKET EVEN IN TCP MODE : THAT ALLOWS US TO COMPUTE A "SEMI-PING" TIME BETWEEN SERVER AND CLIENTS
	if( clt_sock.isTcp() == false )
    {
	*/
        Unit * un = this->game_unit.GetUnit();
        if ( !un )
            return;
        Packet p;
        p.send( CMD_PING, un->GetSerial(),
                (char *)NULL, 0,
                SENDANDFORGET, NULL, *this->lossy_socket,
                __FILE__, PSEUDO__LINE__(414) );
    //}
}

