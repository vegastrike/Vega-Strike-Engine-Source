//#include <netinet/in.h>
//#include "gfxlib.h"
#include "networking/netbuffer.h"
#include "universe_generic.h"
#include "universe_util.h"
//#include "universe_util_generic.h" //Use universe_util_generic.h instead
#include "star_system_generic.h"
#include "cmd/unit_generic.h"
#include "gfx/cockpit_generic.h"
#include "packet.h"
#include "savenet_util.h"
//#include "netserver.h"
#include "zonemgr.h"
#include "vs_globals.h"
#include "endianness.h"
#include <assert.h>

ZoneMgr::ZoneMgr()
{
}

/************************************************************************************************/
/**** addZone                                                                               *****/
/************************************************************************************************/

StarSystem *	ZoneMgr::addZone( string starsys)
{
	cout<<">>> ADDING A NEW ZONE = "<<starsys<<" - # OF ZONES = "<<_Universe->star_system.size()<<endl;
	list<Client *> lst;
	StarSystem * sts=NULL;
	// Generate the StarSystem
	string starsysfile = starsys+".system";
	sts = _Universe->GenerateStarSystem (starsysfile.c_str(),"",Vector(0,0,0));
	// Add it in the star_system vector
	//_Universe->star_system.push_back( sts);
	//_Universe->pushActiveStarSystem( sts);
	// Add an empty list of clients to the zone_list vector
	zone_list.push_back( lst);
	// Add zero as number of clients in zone since we increment in ZoneMgr::addClient()
	zone_clients.push_back( 0);
	cout<<"<<< NEW ZONE ADDED - # OF ZONES = "<<_Universe->star_system.size()<<endl;
	return sts;
}

/************************************************************************************************/
/**** GetZone                                                                               *****/
/************************************************************************************************/

// Return the client list that are in the zone n° serial
list<Client *>	ZoneMgr::GetZone( int serial)
{
	return zone_list[serial];
}

// Adds a client to the zone n° serial
/*
void	ZoneMgr::addClient( Client * clt, int zone)
{
	zone_list[zone].push_back( clt);
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
		cout<<"Trying to remove on an empty list !!"<<endl;
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

StarSystem *	ZoneMgr::addClient( Client * clt, string starsys, unsigned short & num_zone)
{
	// Remove the client from old starsystem if needed and add it in the new one
	/*
	string oldstarsys = clt->save.GetOldStarSystem();
	*/
	StarSystem * sts=NULL;
	StarSystem * ret=NULL;
	//Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	//string starsys = cp->savegame->GetStarSystem();
	// TO BE DONE IN JUMP HANDLING !!!
	/*
	if( starsys!=oldstarsys)
	{
		// Remove the player from the old starsystem
		sts = _Universe->getStarSystem( oldstarsys);
		sts->RemoveUnit( clt->game_unit);

		// SOMEDAY TEST IF THE STARSYSTEM WE WANT TO GO IN IS REACHABLE FROM THE OLD ONE
	}
	*/
	if( !(ret = sts = _Universe->getStarSystem( starsys+".system")))
	{
		// Add a network zone (StarSystem equivalent) and create the new StarSystem
		// StarSystem is not loaded so we generate it
		cout<<"--== STAR SYSTEM NOT FOUND - GENERATING ==--"<<endl;
		sts = this->addZone( starsys);
		// It also mean that there is nobody in that system so no need to send update
		// Return false since the starsystem didn't contain any client
		num_zone = _Universe->star_system.size()-1;
	}
	else
	// Get the index of the existing star_system as it represents the zone number
		num_zone = _Universe->StarSystemIndex( sts);

	cout<<">> ADDING CLIENT IN ZONE # "<<num_zone<<endl;
	// Adds the client in the zone
	zone_list[num_zone].push_back( clt);
	clt->zone = num_zone;
	zone_clients[num_zone]++;

	// Compute a safe entrance point -> DONE WHEN LOGIN ACCEPTED
	//QVector safevec;
	//safevec = UniverseUtil::SafeEntrancePoint( clt->current_state.getPosition());
	//clt->current_state.setPosition( safevec);
	sts->AddUnit( clt->game_unit.GetUnit());
	return ret;
}

/************************************************************************************************/
/**** removeClient                                                                          *****/
/************************************************************************************************/

// Remove a client from its zone
void	ZoneMgr::removeClient( Client * clt)
{
	StarSystem * sts;
	Unit * un = clt->game_unit.GetUnit();
	if( zone_list[clt->zone].empty())
	{
		cout<<"Trying to remove on an empty list !!"<<endl;
		exit( 1);
	}

	zone_list[clt->zone].remove( clt);
	zone_clients[clt->zone]--;
	sts = _Universe->star_system[clt->zone];
	sts->RemoveUnit( un);
	// SHIP MAY NOT HAVE BEEN KILLED BUT JUST CHANGED TO ANOTHER STAR SYSTEM -> NO KILL
	//un->Kill();
}

/************************************************************************************************/
/**** broadcast : broadcast a packet in a zone                                              *****/
/************************************************************************************************/

// Broadcast a packet to a client's zone clients
void	ZoneMgr::broadcast( Client * clt, Packet * pckt )
{
    if( clt == NULL )
    {
        cout<<"Trying to send update without client" << endl;
        return;
    }
    if( clt->zone < 0 || clt->zone > zone_list.size() )
    {
        cout<<"Trying to send update to nonexistant zone " << clt->zone << endl;
        return;
    }

    // cout<<"Sending update to "<<(zone_list[clt->zone].size()-1)<<" clients"<<endl;
	for( LI i=zone_list[clt->zone].begin(); i!=zone_list[clt->zone].end(); i++)
	{
		// Broadcast to other clients
		if( clt->serial!= (*i)->serial)
		{
			cout<<"BROADCASTING "<<pckt->getCommand()<<" to client n° "<<(*i)->serial;
			cout<<endl;
			pckt->setNetwork( &(*i)->cltadr, (*i)->sock);
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
    // cout<<"Sending update to "<<(zone_list[clt->zone].size()-1)<<" clients"<<endl;
	for( LI i=zone_list[zone].begin(); i!=zone_list[zone].end(); i++)
	{
		// Broadcast to all clients including the one who did a request
		//if( serial!= (*i)->serial)
		//{
			cout<<"Sending update to client n° "<<(*i)->serial;
			cout<<endl;
			pckt->setNetwork( &(*i)->cltadr, (*i)->sock);
			pckt->bc_send( );
		//}
	}
}

/************************************************************************************************/
/**** broadcastSnapshots                                                                    *****/
/************************************************************************************************/

// Broadcast all positions
void	ZoneMgr::broadcastSnapshots( )
{
	ClientState cstmp;
	char buffer[MAXBUFFER];
	int i=0, j=0, p=0;
	LI k, l;
	LUI m;
	NetBuffer netbuf;

	//cout<<"Sending snapshot for ";
	//int h_length = Packet::getHeaderLength();
	// Loop for all systems/zones
	for( i=0; i<zone_list.size(); i++)
	{
		// Check if system is non-empty
		if( zone_clients[i]>0)
		{
			// It allows to check (for a given client) if other clients are far away (so we can only
			// send position, not orientation and stuff) and if other clients are visible to the given
			// client.
			// -->> Reduce bandwidth usage but increase CPU usage
			int	offset = 0, nbclients = 0, nbunits=0;
			ObjSerial sertmp;
			Packet pckt;

			// Loop for all the zone's clients
			for( k=zone_list[i].begin(); k!=zone_list[i].end(); k++)
			{
			/************************* START CLIENTS BROADCAST ***************************/
				// If we don't want to send a client its own info set nbclients to zone_clients-1 for memory saving (ok little)
				nbclients = zone_clients[i]-1;
				netbuf.Reset();
				for( j=0, p=0, l=zone_list[i].begin(); l!=zone_list[i].end(); l++)
				{
					// Check if we are on the same client and that the client has moved !
					if( l!=k && !((*l)->current_state.getPosition()==(*l)->old_state.getPosition() && (*l)->current_state.getOrientation()==(*l)->old_state.getOrientation()))
					{
						// Client pointed by 'k' can see client pointed by 'l'
						// For now only check if the 'l' client is in front of the ship and not behind
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
								// Put the current client state in
								netbuf.addClientState( (*l)->current_state);
								// Increment the number of clients we send full info about
								j++;
							}
							// Here find a condition for which sending only position would be enough
							else if( 1 /* ratio>=1 far but still visible */)
							{
								// Mark as position update only
								netbuf.addChar( CMD_POSUPDATE);
								// Add the client serial
								netbuf.addShort( (*l)->serial);
								netbuf.addVector( (*l)->current_state.getPosition());
								// Increment the number of clients we send limited info about
								p++;
							}
						}
					}
					// Else : always send back to clients their own info or just ignore ?
					// Ignore for now
				}
			/************************* END CLIENTS BROADCAST ***************************/
			/************************* START UNITS BROADCAST ***************************/
				nbunits = zone_units[i];
				//netbuf.Reset();
				for( j=0, p=0, m=zone_unitlist[i].begin(); m!=zone_unitlist[i].end(); m++)
				{
					// Check if we are on the same client and that the client has moved !
					if( !((*m)->prev_physical_state.position==(*m)->curr_physical_state.position && (*m)->prev_physical_state.orientation==(*m)->curr_physical_state.orientation))
					{
						// Client pointed by 'k' can see client pointed by 'l'
						// For now only check if the 'l' client is in front of the ship and not behind
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
								// Put the current client state in
								netbuf.addClientState( ClientState( (*m)->GetSerial(), (*m)->curr_physical_state, (*m)->Velocity, (*m)->ResolveForces (identity_transformation,identity_matrix), 0));
								// Increment the number of clients we send full info about
								j++;
							}
							// Here find a condition for which sending only position would be enough
							else if( 1 /* ratio>=1 far but still visible */)
							{
								// Mark as position update only
								netbuf.addChar( CMD_POSUPDATE);
								// Add the client serial
								netbuf.addShort( (*m)->GetSerial());
								netbuf.addQVector( (*m)->curr_physical_state.position);
								// Increment the number of clients we send limited info about
								p++;
							}
						}
					}
					// Else : always send back to clients their own info or just ignore ?
					// Ignore for now
				}
			/************************* END UNITS BROADCAST ***************************/
				// Send snapshot to client k
				if(netbuf.getDataLength()>0)
				{
					//cout<<"\tsend update for "<<(p+j)<<" clients"<<endl;
					pckt.send( CMD_SNAPSHOT, nbclients+nbunits, netbuf.getData(), netbuf.getDataLength(), SENDANDFORGET, &((*k)->cltadr), (*k)->sock, __FILE__,	
#ifndef _WIN32
					__LINE__
#else
					302
#endif
					);
				}
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
	int i=0, j=0, p=0, it=0;
	LI k, l;
	LUI m;
	NetBuffer netbuf;

	//cout<<"Sending snapshot for ";
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
			int	offset = 0, nbclients = 0, nbunits=0, damsize=0;
			ObjSerial sertmp;
			Packet pckt;
			Unit * un;

			// Loop for all the zone's clients
			for( k=zone_list[i].begin(); k!=zone_list[i].end(); k++)
			{
			/************************* START CLIENTS BROADCAST ***************************/
				nbclients = zone_clients[i];
				netbuf.Reset();
				for( j=0, p=0, l=zone_list[i].begin(); l!=zone_list[i].end(); l++)
				{
					// Check if there is damages on that client
					un = (*l)->game_unit.GetUnit();
					unsigned short damages = un->damages;
					if( damages)
					{
						// Add the client serial
						netbuf.addSerial( (*l)->serial);
						// Add the damage flag
						damsize = sizeof( damages);
						if( damsize==sizeof( char))
							netbuf.addChar( damages);
						else if( damsize == sizeof( unsigned short))
							netbuf.addShort( damages);
						else if( damsize == sizeof( unsigned int))
							netbuf.addInt32( damages);
						// Put the altered stucts after the damage enum flag
						if( damages & Unit::SHIELD_DAMAGED)
						{
							netbuf.addShield( un->shield);
						}
						if( damages & Unit::SHIELD_DAMAGED)
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
							netbuf.addShort( un->MaxEnergyData());
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
				}
			/************************* END CLIENTS BROADCAST ***************************/
			/************************* START UNITS BROADCAST ***************************/
				nbunits = zone_units[i];
				//netbuf.Reset();
				for( j=0, p=0, m=zone_unitlist[i].begin(); m!=zone_unitlist[i].end(); m++)
				{
				}
				// Send snapshot to client k
				if( netbuf.getDataLength() > 0)
				{
					pckt.send( CMD_SNAPDAMAGE, nbclients+nbunits, netbuf.getData(), netbuf.getDataLength(), SENDANDFORGET, &((*k)->cltadr), (*k)->sock, __FILE__,	
#ifndef _WIN32
					__LINE__
#else
					302
#endif
					);
				}
			}
		}
	}
}

/************************************************************************************************/
/**** sendZoneClients                                                                       *****/
/************************************************************************************************/

// Send one by one a CMD_ADDLCIENT to the client for every ship in the star system we enter
void	ZoneMgr::sendZoneClients( Client * clt)
{
	LI k;
	int nbclients=0;
	Packet packet2;
	string savestr, xmlstr;
	NetBuffer netbuf;

	// Loop through client in the same zone to send their current_state and save and xml to "clt"
	for( k=zone_list[clt->zone].begin(); k!=zone_list[clt->zone].end(); k++)
	{
		// Test if *k is the same as clt in which case we don't need to send info
		if( clt!=(*k))
		{
			SaveNetUtil::GetSaveStrings( (*k), savestr, xmlstr);
			unsigned int savelen = savestr.length();
			unsigned int xmllen = xmlstr.length();
			// Add the ClientState at the beginning of the buffer
			netbuf.addClientState( (*k)->current_state);
			// Add the save and xml strings
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			packet2.send( CMD_ENTERCLIENT, clt->serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, 
#ifndef _WIN32
				__LINE__
#else
				336
#endif
				);
			nbclients++;
		}
	}
	cout<<"\t>>> SENT INFO ABOUT "<<nbclients<<" OTHER SHIPS TO CLIENT SERIAL "<<clt->serial<<endl;
}

/************************************************************************************************/
/**** getZoneClients                                                                        *****/
/************************************************************************************************/

// Fills buffer with descriptions of clients in the same zone as our client
// Called after the client has been added in the zone so that it can get his
// own information/save from the server
int		ZoneMgr::getZoneClients( Client * clt, char * bufzone)
{
	LI k;
	int state_size;
	unsigned short nbt, nb;
	state_size = sizeof( ClientState);
	nbt = zone_clients[clt->zone];
	NetBuffer netbuf;

	cout<<"ZONE "<<clt->zone<<" - "<<nbt<<" clients"<<endl;
	netbuf.addShort( nbt);
	for( k=zone_list[clt->zone].begin(); k!=zone_list[clt->zone].end(); k++)
	{
		cout<<"SENDING : ";
		netbuf.addClientState( (*k)->current_state);
		(*k)->current_state.display();
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
