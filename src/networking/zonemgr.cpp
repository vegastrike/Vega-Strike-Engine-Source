#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "universe_generic.h"
#include "universe_util.h"
#include "star_system_generic.h"
#include "cmd/unit_generic.h"
#include "gfx/cockpit_generic.h"
#include "networking/lowlevel/packet.h"
#include "networking/savenet_util.h"
#include "networking/zonemgr.h"
#include "vs_globals.h"
#include "endianness.h"
#include <assert.h>

ZoneMgr::ZoneMgr()
{
}

/************************************************************************************************/
/**** addZone                                                                               *****/
/************************************************************************************************/

void			ZoneMgr::addSystem( string & sysname, string & system)
{
	Systems.insert( sysname, system);
}
	

StarSystem *	ZoneMgr::addZone( string starsys)
{
	COUT<<">>> ADDING A NEW ZONE = "<<starsys<<" - # OF ZONES = "<<_Universe->star_system.size()<<endl;
    ClientWeakList* lst = new ClientWeakList;
	list<Unit *> ulst;
	StarSystem * sts=NULL;
	// Generate the StarSystem
	string starsysfile = starsys+".system";
	UniverseUtil::ComputeSystemSerials( starsys, starsysfile);
	sts = new StarSystem( starsysfile.c_str(), Vector(0,0,0));
	_Universe->Generate2( sts);
	//sts = _Universe->GenerateStarSystem (starsysfile.c_str(),"",Vector(0,0,0));
	zone_list.push_back( lst);
	zone_unitlist.push_back( ulst);
	// Add zero as number of clients in zone since we increment in ZoneMgr::addClient()
	zone_clients.push_back( 0);
	zone_units.push_back( 0);
	COUT<<"<<< NEW ZONE ADDED - # OF ZONES = "<<_Universe->star_system.size()<<endl;
	return sts;
}

/************************************************************************************************/
/**** GetZone                                                                               *****/
/************************************************************************************************/

// Return the client list that are in the zone n° serial
ClientWeakList* ZoneMgr::GetZone( int serial )
{
	return zone_list[serial];
}

// Adds a client to the zone n° serial
/*
void	ZoneMgr::addClient( ClientWeakPtr clt, int zone)
{
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL )
        zone_list[zone] = lst = new list<Client*>;

	lst->push_back( clt );
	zone_clients[zone]++;
	clt->zone = zone;
	// Now we add the unit in that starsystem
	sts->AddUnit( clt->game_unit);
}
*/

/************************************************************************************************/
/**** addUnit                                                                               *****/
/************************************************************************************************/

void	ZoneMgr::addUnit( Unit * un, int zone)
{
	zone_unitlist[zone].push_back( un);
	zone_units[zone]++;
}

/************************************************************************************************/
/**** removeUnit                                                                            *****/
/************************************************************************************************/

void	ZoneMgr::removeUnit( Unit * un, int zone)
{
	if( zone_unitlist[zone].empty())
	{
		cerr<<"Trying to remove unit on an empty list !!"<<endl;
		exit( 1);
	}
	zone_unitlist[zone].remove( un);
	zone_units[zone]--;
}

// Returns NULL if no corresponding Unit was found
Unit *	ZoneMgr::getUnit( ObjSerial unserial, unsigned short zone)
{
	LUI i;
	Unit * un = NULL;
	for( i=zone_unitlist[zone].begin(); i!=zone_unitlist[zone].end(); i++)
	{
		if( (*i)->GetSerial()==unserial)
			un = (*i);
	}

	return un;
}

/************************************************************************************************/
/**** addClient                                                                             *****/
/************************************************************************************************/

StarSystem *	ZoneMgr::addClient( ClientWeakPtr cltw, string starsys, unsigned short & num_zone)
{
	// Remove the client from old starsystem if needed and add it in the new one
	/*
	string oldstarsys = cltw->save.GetOldStarSystem();
	*/
	StarSystem * sts=NULL;
	StarSystem * ret=NULL;
	//Cockpit * cp = _Universe->isPlayerStarship( cltw->game_unit.GetUnit());
	//string starsys = cp->savegame->GetStarSystem();
	// TO BE DONE IN JUMP HANDLING !!!
	/*
	if( starsys!=oldstarsys)
	{
		// Remove the player from the old starsystem
		sts = _Universe->getStarSystem( oldstarsys);
		sts->RemoveUnit( ClientPtr(cltw)->game_unit);

		// SOMEDAY TEST IF THE STARSYSTEM WE WANT TO GO IN IS REACHABLE FROM THE OLD ONE
	}
	*/
	if( !(ret = sts = _Universe->getStarSystem( starsys+".system")))
	{
		// Add a network zone (StarSystem equivalent) and create the new StarSystem
		// StarSystem is not loaded so we generate it
		COUT<<"--== STAR SYSTEM NOT FOUND - GENERATING ==--"<<endl;
		sts = this->addZone( starsys);
		// It also mean that there is nobody in that system so no need to send update
		// Return false since the starsystem didn't contain any client
		num_zone = _Universe->star_system.size()-1;
	}
	else
	// Get the index of the existing star_system as it represents the zone number
		num_zone = _Universe->StarSystemIndex( sts);

	COUT<<">> ADDING CLIENT IN ZONE # "<<num_zone<<endl;
	// Adds the client in the zone

    ClientWeakList* lst = zone_list[num_zone];
    if( lst == NULL )
        zone_list[num_zone] = lst = new ClientWeakList;
	lst->push_back( cltw );
    ClientPtr clt(cltw);
	clt->zone = num_zone;
	zone_clients[num_zone]++;
	cerr<<zone_clients[clt->zone]<<" clients now in zone "<<clt->zone<<endl;

	// Compute a safe entrance point -> DONE WHEN LOGIN ACCEPTED
	//QVector safevec;
	sts->AddUnit( clt->game_unit.GetUnit());
	return ret;
}

/************************************************************************************************/
/**** removeClient                                                                          *****/
/************************************************************************************************/

// Remove a client from its zone
void	ZoneMgr::removeClient( ClientPtr clt )
{
	StarSystem * sts;
	Unit * un = clt->game_unit.GetUnit();
    ClientWeakList* lst = zone_list[clt->zone];

	if( lst == NULL || lst->empty() )
	{
        cerr<<"Trying to remove client on an empty list !!"<<endl;
        exit( 1);
    }

	lst->remove( clt );
	zone_clients[clt->zone]--;
	cerr<<zone_clients[clt->zone]<<" clients left in zone "<<clt->zone<<endl;
	sts = _Universe->star_system[clt->zone];
	sts->RemoveUnit( un);
	// SHIP MAY NOT HAVE BEEN KILLED BUT JUST CHANGED TO ANOTHER STAR SYSTEM -> NO KILL
	//un->Kill();
}

/************************************************************************************************/
/**** broadcast : broadcast a packet in a zone                                              *****/
/************************************************************************************************/

// Broadcast a packet to a client's zone clients
void ZoneMgr::broadcast( ClientWeakPtr cltw, Packet * pckt )
{
    if( cltw.expired() )
    {
        cerr<<"Trying to send update without client" << endl;
        return;
    }
    ClientPtr clt( cltw );
	Unit * un = clt->game_unit.GetUnit();
	Unit * un2 = NULL;
    if( clt->zone > zone_list.size() )
    {
        cerr<<"Trying to send update to nonexistant zone " << clt->zone << endl;
        return;
    }

    // cout<<"Sending update to "<<(zone_list[clt->zone].size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[clt->zone];
    if( lst == NULL )
    {
        cerr<<"Trying to send update to nonexistant zone " << clt->zone << endl;
        return;
    }

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr cpi(*i);
        un2 = cpi->game_unit.GetUnit();
        // Broadcast to other clients
        if( cpi->ingame && un->GetSerial() != un2->GetSerial())
        {
            COUT << "BROADCASTING " << pckt->getCommand()
                 << " to client n° "<<un2->GetSerial() << endl;
            pckt->setNetwork( &cpi->cltadr, cpi->sock);
            pckt->bc_send( );
        }
    }
}

/************************************************************************************************/
/**** broadcast : broadcast a packet in a zone                                              *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
void	ZoneMgr::broadcast( int zone, ObjSerial serial, Packet * pckt )
{
    // COUT<<"Sending update to "<<(zone_list[clt->zone]->size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		// Broadcast to all clients including the one who did a request
		if( clt->ingame /*&& un->GetSerial() != un2->GetSerial()*/ )
		{
			COUT<<"Sending update to client n° "<< clt->game_unit.GetUnit()->GetSerial();
			COUT<<endl;
			pckt->setNetwork( &clt->cltadr, clt->sock);
			pckt->bc_send( );
		}
	}
}

// Broadcast a packet to a zone clients with its serial as argument but not to the originating client
void	ZoneMgr::broadcastNoSelf( int zone, ObjSerial serial, Packet * pckt )
{
    // COUT<<"Sending update to "<<(zone_list[clt->zone]->size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		// Broadcast to all clients including the one who did a request
		if( clt->ingame && clt->game_unit.GetUnit()->GetSerial()!=serial )
		{
			COUT<<"Sending update to client n° "<< clt->game_unit.GetUnit()->GetSerial();
			COUT<<endl;
			pckt->setNetwork( &clt->cltadr, clt->sock);
			pckt->bc_send( );
		}
	}
}

/************************************************************************************************/
/**** broadcastSample : broadcast sound sample to players in the zone and on same frequency *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
void	ZoneMgr::broadcastSample( int zone, ObjSerial serial, Packet * pckt, float frequency )
{
    // COUT<<"Sending update to "<<(zone_list[clt->zone]->size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[zone];
	Unit * un;
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		un = clt->game_unit.GetUnit();
		// Broadcast to all clients excluding the one who did a request and
		// excluding those who are listening on a different frequency, those who aren't communicating
		// and those who don't have PortAudio support
		if( clt->ingame && clt->comm_freq!=-1 && clt->portaudio && clt->comm_freq==frequency && un->GetSerial() != serial )
		{
			COUT<<"Sending sound sample to client n° "<<clt->game_unit.GetUnit()->GetSerial();
			COUT<<endl;
			pckt->setNetwork( &clt->cltadr, clt->sock);
			pckt->bc_send( );
		}
	}
}

/************************************************************************************************/
/**** broadcastText : broadcast a text message to players in the zone and on same frequency *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
void	ZoneMgr::broadcastText( int zone, ObjSerial serial, Packet * pckt, float frequency )
{
    // COUT<<"Sending update to "<<(zone_list[clt->zone]->size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[zone];
	Unit * un;
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		un = clt->game_unit.GetUnit();
		// Broadcast to all clients excluding the one who did a request and
		// excluding those who are listening on a different frequency, those who aren't communicating
		// and those who don't have PortAudio support
		if( clt->ingame && clt->comm_freq!=-1 && un->GetSerial() != serial )
		{
			COUT<<"Sending sound sample to client n° "<<clt->game_unit.GetUnit()->GetSerial();
			COUT<<endl;
			pckt->setNetwork( &clt->cltadr, clt->sock);
			pckt->bc_send( );
		}
	}
}

/************************************************************************************************/
/**** broadcastSnapshots                                                                    *****/
/************************************************************************************************/

// Broadcast all positions
void	ZoneMgr::broadcastSnapshots( bool update_planets)
{
	unsigned int i=0;
	CWLI k, l;
	LUI m;
	NetBuffer netbuf;

	//COUT<<"Sending snapshot for ";
	//int h_length = Packet::getHeaderLength();
	// Loop for all systems/zones
	for( i=0; i<zone_list.size(); i++)
	{
		// Check if system contains player(s)
		if( zone_clients[i]>0)
		{
			int	nbclients = 0, nbunits=0;
			Packet pckt;

			cerr<<"BROADCAST SNAPSHOTS = "<<zone_clients[i]<<" clients in zone "<<i<<endl;
			// Loop for all the zone's clients
			for( k=zone_list[i]->begin(); k!=zone_list[i]->end(); k++)
			{
			/************************* START CLIENTS BROADCAST ***************************/
				// If we don't want to send a client its own info set nbclients to zone_clients-1 for memory saving (ok little)
                if( (*k).expired() ) continue;
                ClientPtr cltk( *k );
				if( cltk->ingame )
				{
					nbclients = zone_clients[i]-1;
					netbuf.Reset();
					for( l=zone_list[i]->begin(); l!=zone_list[i]->end(); l++)
					{
						// Check if we are on the same client and that the client has moved !
                        ClientPtr cltl( *l );
						if( l!=k && cltl->ingame)
						{
							Unit * un = cltl->game_unit.GetUnit();
							// Create a client state with a delta time
							ClientState cs( un);
							// HAVE TO VERIFY WHICH DELTATIME IS TO BE SENT
							cs.setDelay( cltl->deltatime);
							this->addPosition( netbuf, un, cltk->game_unit.GetUnit(), cs);
						}
						// Else : always send back to clients their own info or just ignore ?
						// Ignore for now
					}
			/************************* END CLIENTS BROADCAST ***************************/
			/************************* START UNITS BROADCAST ***************************/
					nbunits = zone_units[i];
					cerr<<"BROADCAST SNAPSHOTS = "<<zone_units[i]<<" units in zone "<<i<<endl;
					//netbuf.Reset();
					for( m=zone_unitlist[i].begin(); m!=zone_unitlist[i].end(); m++)
					{
						// Only send planets and nebulas update when PLANET_ATOM is reached
						if( ((*m)->isUnit()!=PLANETPTR && (*m)->isUnit()!=NEBULAPTR) || update_planets )
						{
							// Create a client state with a delta time too ?? WHICH ONE ???
							ClientState cs( (*m));
							this->addPosition( netbuf, (*m), cltk->game_unit.GetUnit(), cs);
							// Else : always send back to clients their own info or just ignore ?
							// Ignore for now
						}
					}
			/************************* END UNITS BROADCAST ***************************/
					// Send snapshot to client k
					if(netbuf.getDataLength()>0)
					{
						//COUT<<"\tsend update for "<<(p+j)<<" clients"<<endl;
						pckt.send( CMD_SNAPSHOT, nbclients+nbunits,
                                   netbuf.getData(), netbuf.getDataLength(),
                                   SENDANDFORGET, &(cltk->cltadr), cltk->sock,
                                   __FILE__, PSEUDO__LINE__(337) );
					}
				}
			}
		}
	}
}

void	ZoneMgr::addPosition( NetBuffer & netbuf, Unit * un, Unit * clt_unit, ClientState & un_cs)
{
	// This test may be wrong for server side units -> may cause more traffic than needed
	if( !(un->old_state.getPosition()==un->curr_physical_state.position) || !(un->old_state.getOrientation()==un->curr_physical_state.orientation))
	{
		// Unit 'un' can see Unit 'iter'
		// For now only check if the 'iter' client is in front of Unit 'un')
		if( 1 /*(distance = this->isVisible( source_orient, source_pos, target_pos)) > 0*/)
		{
			// Test if client 'l' is far away from client 'k' = test radius/distance<=X
			// So we can send only position
			// Here distance should never be 0
			//ratio = radius/distance;
			if( 1 /* ratio > XX client not too far */)
			{
				// Mark as position+orientation+velocity update
				netbuf.addChar( CMD_FULLUPDATE);
				netbuf.addShort( un->GetSerial());
				// Put the current client state in
				netbuf.addClientState( un_cs);
				un_cs.display();
				// Increment the number of clients we send full info about
			}
			// Here find a condition for which sending only position would be enough
			else if( 0 /* ratio>=1 far but still visible */)
			{
				// Mark as position update only
				netbuf.addChar( CMD_POSUPDATE);
				// Add the client serial
				netbuf.addShort( un->GetSerial());
				netbuf.addQVector( un_cs.getPosition());
				// Increment the number of clients we send limited info about
			}
		}
	}
}

/************************************************************************************************/
/**** broadcastDamage                                                                       *****/
/************************************************************************************************/

// Broadcast all damages
void	ZoneMgr::broadcastDamage( )
{
	unsigned int i=0;
	CWLI k, l;
	LUI m;
	NetBuffer netbuf;

	//COUT<<"Sending snapshot for ";
	//int h_length = Packet::getHeaderLength();
	// Loop for all systems/zones
	for( i=0; i<zone_list.size(); i++)
	{
		// Check if system is non-empty
		if( zone_clients[i]>0)
		{
			/************* Second method : send independently to each client a buffer of its zone  ***************/
			// It allows to check (for a given client) if other clients are far away (so we can only
			// send position, not orientation and stuff) and if other clients are visible to the given
			// client.
			int	nbclients = 0, nbunits=0;
			Packet pckt;
			Unit * un;

			//cerr<<"BROADCAST DAMAGE = "<<zone_clients[i]<<" clients in zone "<<i<<endl;
			// Loop for all the zone's clients
			for( k=zone_list[i]->begin(); k!=zone_list[i]->end(); k++)
			{
                if( (*k).expired() ) continue;
                ClientPtr cp( *k );
				if( cp->ingame )
				{
			/************************* START CLIENTS BROADCAST ***************************/
					nbclients = zone_clients[i];
					netbuf.Reset();
					for( l=zone_list[i]->begin(); l!=zone_list[i]->end(); l++)
					{
                        if( (*l).expired() ) continue;

                        ClientPtr cltl( *l );
						// Check if there is damages on that client
						un = cltl->game_unit.GetUnit();
						if( cltl->ingame && un && un->damages)
							this->addDamage( netbuf, un);
					}
			/************************* END CLIENTS BROADCAST ***************************/
			/************************* START UNITS BROADCAST ***************************/
					//cerr<<"BROADCAST DAMAGE = "<<zone_units[i]<<" units in zone "<<i<<endl;
					nbunits = zone_units[i];
					//netbuf.Reset();
					for( m=zone_unitlist[i].begin(); m!=zone_unitlist[i].end(); m++)
					{
						// Check if there is damages on that unit
						if( (*m)->damages)
							this->addDamage( netbuf, (*m));
					}
					// Send snapshot to client k
					if( netbuf.getDataLength() > 0)
					{
						pckt.send( CMD_SNAPDAMAGE, nbclients+nbunits,
                                   netbuf.getData(), netbuf.getDataLength(),
                                   SENDANDFORGET, &(cp->cltadr), cp->sock,
                                   __FILE__, PSEUDO__LINE__(442) );
					}
				}
			}
		}
	}
}

void	ZoneMgr::addDamage( NetBuffer & netbuf, Unit * un)
{
		unsigned int it = 0;

		// Add the damage flag
		unsigned short damages = un->damages;
		netbuf.addShort( damages);
		un->damages = Unit::NO_DAMAGE;
		// Add the client serial
		netbuf.addSerial( un->GetSerial());
		// Put the altered stucts after the damage enum flag
		if( damages & Unit::SHIELD_DAMAGED)
		{
			netbuf.addShield( un->shield);
		}
		if( damages & Unit::ARMOR_DAMAGED)
		{
			netbuf.addArmor( un->armor);
		}
		if( damages & Unit::COMPUTER_DAMAGED)
		{
			netbuf.addChar( un->computer.itts);
			netbuf.addChar( un->computer.radar.color);
			netbuf.addFloat( un->limits.retro);
			netbuf.addFloat( un->computer.radar.maxcone);
			netbuf.addFloat( un->computer.radar.lockcone);
			netbuf.addFloat( un->computer.radar.trackingcone);
			netbuf.addFloat( un->computer.radar.maxrange);
			for( it = 0; it<1+UnitImages::NUMGAUGES+MAXVDUS; it++)
				netbuf.addFloat( un->image->cockpit_damage[it]);
		}
		if( damages & Unit::MOUNT_DAMAGED)
		{
			netbuf.addShort( un->image->ecm);
			for( it=0; it<un->mounts.size(); it++)
			{
				if( sizeof( Mount::STATUS) == sizeof( char))
					netbuf.addChar( un->mounts[it].status);
				else if( sizeof( Mount::STATUS) == sizeof( unsigned short))
					netbuf.addShort( un->mounts[it].status);
				else if( sizeof( Mount::STATUS) == sizeof( unsigned int))
					netbuf.addInt32( un->mounts[it].status);

				netbuf.addShort( un->mounts[it].ammo);
				netbuf.addFloat( un->mounts[it].time_to_lock);
				netbuf.addShort( un->mounts[it].size);
			}
		}
		if( damages & Unit::CARGOFUEL_DAMAGED)
		{
			netbuf.addFloat( un->FuelData());
			netbuf.addShort( un->AfterburnData());
			netbuf.addFloat( un->image->cargo_volume);
			for( it=0; it<un->image->cargo.size(); it++)
				netbuf.addInt32( un->image->cargo[it].quantity);
		}
		if( damages & Unit::JUMP_DAMAGED)
		{
			netbuf.addChar( un->shield.leak);
			netbuf.addFloat( un->shield.recharge);
			netbuf.addFloat( un->EnergyRechargeData());
			netbuf.addFloat( un->MaxEnergyData());
			netbuf.addShort( un->jump.energy);
			netbuf.addChar( un->jump.damage);
			netbuf.addChar( un->image->repair_droid);
		}
		if( damages & Unit::CLOAK_DAMAGED)
		{
			netbuf.addShort( un->cloaking);
			netbuf.addFloat( un->image->cloakenergy);
			netbuf.addShort( un->cloakmin);
			netbuf.addShield( un->shield);
		}
		if( damages & Unit::LIMITS_DAMAGED)
		{
			netbuf.addFloat( un->computer.max_pitch);
			netbuf.addFloat( un->computer.max_yaw);
			netbuf.addFloat( un->computer.max_roll);
			netbuf.addFloat( un->limits.roll);
			netbuf.addFloat( un->limits.yaw);
			netbuf.addFloat( un->limits.pitch);
			netbuf.addFloat( un->limits.lateral);
		}
}

/************************************************************************************************/
/**** sendZoneClients                                                                       *****/
/************************************************************************************************/

// Send one by one a CMD_ADDLCIENT to the client for every ship in the star system we enter
void ZoneMgr::sendZoneClients( ClientWeakPtr clt )
{
	CWLI k;
	int nbclients=0;
	Packet packet2;
	string savestr, xmlstr;
	NetBuffer netbuf;

	// Loop through client in the same zone to send their current_state and save and xml to "clt"
    if( clt.expired() ) return;
    ClientPtr cp( clt );

    ClientWeakList* lst = zone_list[cp->zone];
    if( lst == NULL )
    {
	    COUT << "\t>>> WARNING: Did not send info about " << nbclients << " other ships to client serial " << cp->game_unit.GetUnit()->GetSerial() << " because of empty (inconsistent?) zone" << endl;
        return;
    }

	for( k=lst->begin(); k!=lst->end(); k++)
	{
        if( (*k).expired() ) continue;
        ClientPtr kp( *k );

		// Test if *k is the same as clt in which case we don't need to send info
		if( cp!=kp && kp->ingame)
		{
			SaveNetUtil::GetSaveStrings( kp, savestr, xmlstr);
			// Add the ClientState at the beginning of the buffer -> NO THIS IS IN THE SAVE !!
			//netbuf.addClientState( ClientState( kp->game_unit.GetUnit()));
			// Add the callsign and save and xml strings
			netbuf.addString( kp->callsign);
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			packet2.send( CMD_ENTERCLIENT, kp->game_unit.GetUnit()->GetSerial(),
                          netbuf.getData(), netbuf.getDataLength(),
                          SENDRELIABLE, &cp->cltadr, cp->sock,
                          __FILE__, PSEUDO__LINE__(579) );
			nbclients++;
		}
	}
	COUT<<"\t>>> SENT INFO ABOUT "<<nbclients<<" OTHER SHIPS TO CLIENT SERIAL "<<cp->game_unit.GetUnit()->GetSerial()<<endl;
}

/************************************************************************************************/
/**** getZoneClients                                                                        *****/
/************************************************************************************************/

// Fills buffer with descriptions of clients in the same zone as our client
// Called after the client has been added in the zone so that it can get his
// own information/save from the server
int		ZoneMgr::getZoneClients( ClientWeakPtr clt, char * bufzone)
{
	CWLI k;
	int state_size;
	unsigned short nbt;
	state_size = sizeof(ClientState);

    if( clt.expired() ) return 0;
    ClientPtr cp( clt );

	nbt = zone_clients[cp->zone];
	NetBuffer netbuf;

	COUT<<"ZONE "<<cp->zone<<" - "<<nbt<<" clients"<<endl;
	netbuf.addShort( nbt);
    assert( zone_list[cp->zone] != NULL );
	for( k=zone_list[cp->zone]->begin(); k!=zone_list[cp->zone]->end(); k++)
	{
		COUT<<"SENDING : ";
        if( (*k).expired() ) continue;
        ClientPtr kp( *k );
		if( kp->ingame)
			netbuf.addClientState( ClientState( kp->game_unit.GetUnit() )  );
	}

	return state_size*nbt;
}

/************************************************************************************************/
/****  isVisible                                                                            *****/
/************************************************************************************************/

double	ZoneMgr::isVisible( Quaternion orient, QVector src_pos, QVector tar_pos)
{
	double	dotp = 0;
	Matrix m;

	orient.to_matrix(m);
	QVector src_tar( m.getR());

	src_tar = tar_pos - src_pos;
	dotp = DotProduct( src_tar, (QVector) orient.v);

	return dotp;
}

/************************************************************************************************/
/****  displayStats                                                                         *****/
/************************************************************************************************/

void	ZoneMgr::displayStats()
{
	unsigned int i;
	cout<<"\tStar system stats"<<endl;
	cout<<"\t-----------------"<<endl;
	for( i=0; i<zone_list.size(); i++)
	{
		cout<<"\t\tStar system "<<i<<" = \"<<_Universe->star_system[i]->getName()"<<"\""<<endl;
		cout<<"\t\t\tNumber of clients :\t"<<zone_clients[i]<<endl;
		cout<<"\t\t\tNumber of units :\t"<<zone_units[i]<<endl;
	}
}

/************************************************************************************************/
/****  displayMemory                                                                        *****/
/************************************************************************************************/

int		ZoneMgr::displayMemory()
{
	unsigned int i;
	int memory_use=0;
	int memclient=0, memunit=0, memvars=0;
	cout<<"\tStar system memory usage (do not count struct pointed by pointer)"<<endl;
	cout<<"\t-----------------------------------------------------------------"<<endl;
	for( i=0; i<zone_list.size(); i++)
	{
		
		cout<<"\t\tStar system "<<i<<" = \"<<_Universe->star_system[i]->getName()"<<"\""<<endl;
		memclient = zone_clients[i]*sizeof( class Client);
		memunit = zone_units[i]*sizeof( class Unit);
		memvars = zone_clients[i]*sizeof( int)*2;
		cout<<"\t\t\tMemory for clients :\t"<<(memclient/1024)<<" KB ("<<memclient<<" bytes)"<<endl;
		cout<<"\t\t\tMemory for units :\t"<<(memunit/1024)<<" KB ("<<memunit<<" bytes)"<<endl;
		cout<<"\t\t\tMemory for variables :\t"<<(memvars/1024)<<" KB ("<<memvars<<" bytes)"<<endl;
		memory_use += (memclient+memunit+memvars);
	}
	return memory_use;
}


std::string ZoneMgr::Systems::insert( std::string sysname, std::string system )
{
    if( sysname != "" && system != "" )
        _map.insert( SystemPair( sysname, system ) );
    return sysname;
}

std::string ZoneMgr::Systems::get( std::string sysname )
{
    SystemIt it = _map.find(sysname);
    if( it == _map.end() ) return string( "");
    return it->second;
}

bool ZoneMgr::Systems::remove( std::string sysname )
{
    size_t s = _map.erase( sysname );
    if( s == 0 ) return false;
    return true;
}


/*** This is a copy of GFXSphereInFrustum from gl_matrix_hack.cpp avoiding
 * linking with a LOT of unecessary stuff
 */

/*
float	ZoneMgr::sphereInFrustum( const Vector &Cnt, float radius)
{
	float frust [6][4];
   int p;
   float d;
   for( p = 0; p < 5; p++ )//does not evaluate for yon
   {
      d = f[p][0] * Cnt.i + f[p][1] * Cnt.j + f[p][2] * Cnt.k + f[p][3];
      if( d <= -radius )
         return 0;
   }
   return d;
}
*/
