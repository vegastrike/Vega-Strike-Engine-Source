
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
void DoEnterExitAni(QVector pos, float size, bool enter) {
  static std::string enterclientani=vs_config->getVariable("graphics","enter_client_ani","warp.ani");
  static std::string exitclientani=vs_config->getVariable("graphics","enter_client_ani","warp.ani");
  std::string tmp = enter?enterclientani:exitclientani;
  if (tmp.length()) {
    static float scale=XMLSupport::parse_float(vs_config->getVariable("graphics","client_anim_scale","3"));
    UniverseUtil::playAnimation(tmp,pos,size*scale);
  }

}
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
		cout << " not adding unit with serial number "<< cltserial<<" named " <<shouldbenull->name.get()<<" to system .";
		return;//already exists
	}
	if( !isLocalSerial( cltserial)) {

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
			if (savestr.length()==0){
				if (this->lastsave.size()&&this->lastsave[0].length()&&
					this->callsign==cltname) {
					savestr=this->lastsave[0];
				}
			}
			SaveGame save ("");
			save.ParseSaveGame( "", starsys, "", pos, update, creds, savedships, 0, savestr, false);
                  
			PLAYER_FACTION_STRING= save.GetPlayerFaction();
                  
			// CREATES THE UNIT... GET SAVE AND XML FROM SERVER
			// Use the first ship if there are more than one -> we don't handle multiple ships for now
			// We name the flightgroup with the player name
			if (savedships.empty()) {
				cerr<<"savedships is empty!!!"<<endl;
				cerr<<"SAVEGAME: "<<endl<<savestr<<"-------"<<endl;
				cerr<<"SHIPCSV: "<<endl<<xmlstr<<"-------"<<endl;
				return;
			} else {
				cerr<<"Found saveship[0] = "<<savedships[0]<<endl;
			}
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
		DoEnterExitAni(trans.position,un->rSize(),true);
		if (clt) {
			clt->last_packet=un->old_state;
			clt->prediction->InitInterpolation(un, un->old_state, 0, this->deltatime);
				
//			save.SetPlayerLocation(un->curr_physical_state.position);
			clt->name = cltname;
			clt->callsign=cltname;
			string msg = clt->callsign+" entered the system";
			UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
		}
	}
}

ClientPtr NetClient::AddClientObject( Unit *un, ObjSerial cltserial)
{
	if (!cltserial)
		cltserial=un->GetSerial();

	COUT << " >>> ENTERING CLIENT =( serial #"
		<< cltserial << " )= --------------------------------------" << endl;

	ClientPtr clt = Clients.get(cltserial);
	if( clt)
	{
		// Client may exist if it jumped from a starsystem to another of if killed and respawned
		COUT<<"Existing client #"<<cltserial<<endl;
	}
	else if (!cltserial)
	{
		COUT<<"Local client with serial 0: "<<un->name<<", "<<un->getFullname()<<endl;
		return clt;
	}
	else
	{
		clt = Clients.insert( cltserial, new Client );
		nbclients++;
		COUT<<"New client #"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
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
	VsnetDownload::Client::Buffer buf( *this->clt_tcp_sock, tbuf, VSFileSystem::ZoneBuffer);
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
	/*
        std::set<ObjSerial> enteredSerials;
        for (unsigned int i=0;i<_Universe->numPlayers();++i) {
          Unit*un=_Universe->AccessCockpit(i)->GetParent();
          if(un) enteredSerials.insert(un->GetSerial());
        }
	*/
	// Loop until the end of the buffer
	int offset=netbuf.getOffset();
	while( (subcmd=netbuf.getChar())!=ZoneMgr::End)
	{
                int noffset=netbuf.getOffset();
                if (noffset==offset) {
                  COUT << "ERROR Premature end of AddObjects buffer "<<std::hex<<std::string(netbuf.getData(),netbuf.getSize()) << std::endl;

                  break;
                }
                offset=noffset;// to make sure we aren't at end of truncated buf
		Unit * newunit;
		ObjSerial serial;
		switch( subcmd)
		{
			case ZoneMgr::AddClient :
				serial = netbuf.getSerial();
//				enteredSerials.insert(serial);
				this->enterClient( netbuf, serial);
				break;
			case ZoneMgr::AddUnit :
				newunit = UnitFactory::parseUnitBuffer(netbuf);
				AddClientObject(newunit);
//				enteredSerials.insert(newunit->GetSerial());
				break;
			case ZoneMgr::AddNebula :
				newunit = (Unit*)UnitFactory::parseNebulaBuffer(netbuf);
				AddClientObject(newunit);
//				enteredSerials.insert(newunit->GetSerial());
				break;
			case ZoneMgr::AddPlanet :
				newunit = (Unit*)UnitFactory::parsePlanetBuffer(netbuf);
				AddClientObject(newunit);
//				enteredSerials.insert(newunit->GetSerial());
				break;
			case ZoneMgr::AddAsteroid :
				newunit = (Unit*)UnitFactory::parseAsteroidBuffer(netbuf);
				AddClientObject(newunit);
//				enteredSerials.insert(newunit->GetSerial());
				break;
			case ZoneMgr::AddMissile :
				newunit = (Unit*)UnitFactory::parseMissileBuffer(netbuf);
				AddClientObject(newunit);
//				enteredSerials.insert(newunit->GetSerial());
				break;
			default :
				cerr<<"WARNING : Unknown sub "<<(int)subcmd<< " command in AddObjects"<<endl;
				break;
		}
	}
	// NETFIXME: What is the point of killing off all non-networked units all the time?
	/*
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
	*/
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
                DoEnterExitAni(un->Position(),un->rSize(),false);
		// Removes the unit from starsystem, destroys it and delete client
		_Universe->activeStarSystem()->RemoveUnit( un );
	}
	nbclients--;
	Clients.remove(cltserial);
	if (un) {
		un->Kill();
	}
	COUT<<"Leaving client #"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
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
	NetBuffer netbuf;
	static bool debugPos = XMLSupport::parse_bool(vs_config->getVariable("network", "debug_position_interpolation", "false"));

	// Send the client state
	if (debugPos) COUT<<"Sending ClientState == ";
	if (debugPos) (*cs).display();
	netbuf.addSerial( cs->getSerial());
	netbuf.addClientState( (*cs));
        static bool aim_assist = XMLSupport::parse_bool(vs_config->getVariable("network","aim_assist","true"));
        Unit * targ;
        if ((targ=un->Target())!=NULL&&aim_assist&&un->Target()->GetSerial()!=0/*networked unit*/) {
          netbuf.addSerial(targ->GetSerial());
          netbuf.addVector((targ->Position()-cs->getPosition()).Cast());
          netbuf.addVector(targ->Velocity);
        }
	send ( CMD_POSUPDATE, netbuf, SENDANDFORGET, __FILE__, __LINE__ );
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
	if( latest_timestamp <= int_ts)
	{
        if (debugPos) COUT << "   *** SNAPSHOT is not late - evaluating" << endl;

		this->latest_timestamp = int_ts;
		this->deltatime        = delta_t;

		// Loop throught received snapshot
        int i = 0;
        int j = 0;
        int k = 0;
        int offset=netbuf.getOffset();
		while( (i+j+k)<numUnits )
		{
            ObjSerial       sernum = 0;
            unsigned char   cmd;
	        bool			localplayer = false;
	        ClientPtr		clt;
	        Unit *			un = NULL;

			// Get the ZoneMgr::SnapshotSubCommand from buffer
			cmd    = netbuf.getChar();
                        int noffset=netbuf.getOffset();
                        if (noffset==offset) {
                          COUT << "ERROR Premature end of Snapshot buffer "<<std::hex<<std::string(netbuf.getData(),netbuf.getSize()) << std::endl;
                          break;//if buffer is already used up...
                        }
                        offset=noffset;


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
						cs.setUnitState(un);
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
			} else
				k++;
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

#define SETNOTNULL(un, param, val) if (un) param=val; else val

void NetClient::receiveUnitDamage( NetBuffer &netbuf, Unit *un ) {
	int it=0;
	unsigned short damages;
	damages = netbuf.getShort();

	if (!un) {
		cerr<< "Received Damage Update for null unit"<<endl;
	} else {
		cout << "Received damage " <<damages<<" for unit "<<un->GetSerial()<<" ("<<un->name<<")"<<endl;
	}
	
	if( damages & Unit::SHIELD_DAMAGED)
	{
		SETNOTNULL(un,un->shield,netbuf.getShield());
	}
	if( damages & Unit::ARMOR_DAMAGED)
	{
		SETNOTNULL(un,un->armor, netbuf.getArmor());
		SETNOTNULL(un,un->hull, netbuf.getFloat());
	}
	if( damages & Unit::COMPUTER_DAMAGED)
	{
		SETNOTNULL(un,un->computer.itts, netbuf.getChar());
		SETNOTNULL(un,un->computer.radar.iff, netbuf.getChar());
		SETNOTNULL(un,un->limits.retro, netbuf.getFloat());
		SETNOTNULL(un,un->computer.radar.maxcone, netbuf.getFloat());
		SETNOTNULL(un,un->computer.radar.lockcone, netbuf.getFloat());
		SETNOTNULL(un,un->computer.radar.trackingcone, netbuf.getFloat());
		SETNOTNULL(un,un->computer.radar.maxrange, netbuf.getFloat());
		int numvdus=(int)netbuf.getChar();
		for( it = 0; it<numvdus; it++) {
			float dam=netbuf.getFloat8();
			if (un && it<1+UnitImages::NUMGAUGES+MAXVDUS) {
				un->image->cockpit_damage[it] = dam;
			}
		}
	}
	if( damages & Unit::MOUNT_DAMAGED)
	{
		SETNOTNULL(un,un->image->ecm, netbuf.getShort());
		int nummounts=(int)netbuf.getShort();
		for( it=0; it<nummounts; it++)
		{
			Mount::STATUS tmpstatus=( Mount::STATUS) netbuf.getChar();
			int ammo = netbuf.getInt32();
			float ttl = netbuf.getFloat();
			unsigned short size = netbuf.getShort();
			if(un && it<un->mounts.size()) {
				if (_Universe->isPlayerStarship(un)==NULL||tmpstatus==Mount::UNCHOSEN||tmpstatus==Mount::DESTROYED) {
					un->mounts[it].status = tmpstatus;
					//don't reset my mount status on me... that's my 
				}
				un->mounts[it].ammo = ammo;
				un->mounts[it].time_to_lock = ttl;
				un->mounts[it].size = size;
			}
		}
	}
	if( damages & Unit::CARGOFUEL_DAMAGED)
	{
		if (un) un->SetFuel( netbuf.getFloat()); else netbuf.getFloat();
		if (un) un->SetAfterBurn(netbuf.getFloat()); else netbuf.getFloat();
		SETNOTNULL(un,un->image->CargoVolume, netbuf.getFloat());
		SETNOTNULL(un,un->image->UpgradeVolume, netbuf.getFloat());
		// NRTFIXME: cargo unimplented.
//		for( it=0; it<un->image->cargo.size(); it++)
//			un->image->cargo[it].quantity = netbuf.getInt32();
	}
	if( damages & Unit::JUMP_DAMAGED)
	{
		SETNOTNULL(un,un->shield.leak, netbuf.getChar());
		SETNOTNULL(un,un->shield.recharge, netbuf.getFloat());
		if (un) un->SetEnergyRecharge( netbuf.getFloat()); else netbuf.getFloat();
		if (un) un->SetMaxEnergy( netbuf.getFloat()); else netbuf.getFloat();
		SETNOTNULL(un,un->jump.energy, netbuf.getFloat());
		SETNOTNULL(un,un->jump.damage, netbuf.getChar());
		SETNOTNULL(un,un->image->repair_droid, netbuf.getChar());
	}
	if( damages & Unit::CLOAK_DAMAGED)
	{
		SETNOTNULL(un,un->cloaking, netbuf.getInt32());
		SETNOTNULL(un,un->image->cloakenergy, netbuf.getFloat());
		SETNOTNULL(un,un->cloakmin, netbuf.getInt32());
		SETNOTNULL(un,un->image->cloakrate, netbuf.getInt32());
	}
	if( damages & Unit::LIMITS_DAMAGED)
	{
		SETNOTNULL(un,un->computer.max_pitch_down, netbuf.getFloat( ));
		SETNOTNULL(un,un->computer.max_pitch_up, netbuf.getFloat( ));
		SETNOTNULL(un,un->computer.max_yaw_left, netbuf.getFloat( ));
		SETNOTNULL(un,un->computer.max_yaw_right, netbuf.getFloat( ));
		SETNOTNULL(un,un->computer.max_roll_left, netbuf.getFloat( ));
		SETNOTNULL(un,un->computer.max_roll_right, netbuf.getFloat( ));
		SETNOTNULL(un,un->limits.roll, netbuf.getFloat( ));
		SETNOTNULL(un,un->limits.yaw, netbuf.getFloat( ));
		SETNOTNULL(un,un->limits.pitch, netbuf.getFloat( ));
		SETNOTNULL(un,un->limits.lateral, netbuf.getFloat( ));
	}
}

/*************************************************************/
/**** Says we are ready and going in game                 ****/
/*************************************************************/

void	NetClient::inGame()
{
	NetBuffer netbuf;
    char      flags = 0;
	Unit    * un = this->game_unit.GetUnit();
	if (!un) {
		cout<<"Trying to ingame dead unit";
	}

	//ClientState cs( this->serial, un->curr_physical_state, un->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !! NOT NEEDED ANYMORE -> THE SERVER ALREADY KNOWS
	//netbuf.addClientState( cs);
	send ( CMD_ADDCLIENT, netbuf, SENDRELIABLE, __FILE__, __LINE__ );
	un->SetSerial( this->serial);
	COUT << "Sending ingame with serial #" << this->serial << endl;
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

