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
#include "networking/netserver.h"
#include "vs_random.h"

extern StarSystem * GetLoadedStarSystem( const char * system);

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

string			ZoneMgr::getSystem( string & name)
{
	return Systems.get( name);
}

StarSystem *	ZoneMgr::addZone( string starsys)
{
	StarSystem * sts=NULL;
	string sysfile = starsys+".system";
	if( (sts = GetLoadedStarSystem( sysfile.c_str() ))) {
		cerr<<"--== STAR SYSTEM " << starsys << " ALREADY EXISTS ==--"<<endl;
		return sts;
	}
	// Add a network zone (StarSystem equivalent) and create the new StarSystem
	// StarSystem is not loaded so we generate it
	COUT<<">>> ADDING A NEW ZONE = "<<starsys<<" - ZONE # = "<<_Universe->star_system.size()<<endl;
	COUT<<"--== STAR SYSTEM NOT FOUND - GENERATING ==--"<<endl;
	//list<Unit *> ulst;
	// Generate the StarSystem
	_Universe->netLock(true);
	string starsysfile = starsys+".system";
	UniverseUtil::ComputeSystemSerials( starsysfile);
	//sts = new StarSystem( starsysfile.c_str(), Vector(0,0,0));
	//_Universe->Generate2( sts);
	sts = _Universe->GenerateStarSystem (starsysfile.c_str(),"",Vector(0,0,0));
	bool newSystem=true;
	unsigned int i;
	for (i=0;i<_Universe->star_system.size();i++) {
		if (_Universe->star_system[i]==sts) {
			newSystem=false;
			break;
		}
	}
	while (newSystem||i>=zone_list.size()) {
		newSystem=false;
		ClientWeakList* lst = new ClientWeakList;
		zone_list.push_back( lst);
		//zone_unitlist.push_back( ulst);
		// Add zero as number of clients in zone since we increment in ZoneMgr::addClient()
		zone_clients.push_back( 0);
		//zone_units.push_back( 0);
		COUT<<"<<< NEW ZONE ADDED - # OF ZONES = "<<_Universe->star_system.size()<<endl;
	}
	sts->SetZone( _Universe->StarSystemIndex( sts));
	_Universe->netLock(false);
	return sts;
}

/************************************************************************************************/
/**** GetZone                                                                               *****/
/************************************************************************************************/

// Return the client list that are in the zone # serial
ClientWeakList* ZoneMgr::GetZone( int serial )
{
	return zone_list[serial];
}

// Adds a client to the zone # serial
/*
void	ZoneMgr::addClient( ClientWeakPtr clt, int zone)
{
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL )
        zone_list[zone] = lst = new list<Client*>;

	lst->push_back( clt );
	zone_clients[zone]++;
	// Now we add the unit in that starsystem
	sts->AddUnit( clt->game_unit);
}
*/

/************************************************************************************************/
/**** addUnit                                                                               *****/
/************************************************************************************************/

/*
void	ZoneMgr::addUnit( Unit * un, int zone)
{
	zone_unitlist[zone].push_back( un);
	zone_units[zone]++;
}
*/

/************************************************************************************************/
/**** removeUnit                                                                            *****/
/************************************************************************************************/

/*
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
*/

// Returns NULL if no corresponding Unit was found
Unit *	ZoneMgr::getUnit( ObjSerial unserial, unsigned short zone)
{
	Unit * un = NULL;

	// Clients not ingame are removed from the drawList so it is ok not to test that
	for(un_iter iter = (_Universe->star_system[zone]->getUnitList()).createIterator();un = *iter;++iter){
		if( un->GetSerial() == unserial)
			break;
	}

	return un;
}

/************************************************************************************************/
/**** addClient                                                                             *****/
/************************************************************************************************/

StarSystem *	ZoneMgr::addClient( ClientWeakPtr cltw, string starsys, unsigned short & num_zone)
{
	// Remove the client from old starsystem if needed and add it in the new one
	StarSystem * sts=NULL;

	sts = this->addZone( starsys);
	
	// Get the index of the existing star_system as it represents the zone number
	num_zone = _Universe->StarSystemIndex( sts);

	COUT<<">> ADDING CLIENT IN ZONE # "<<num_zone<<endl;
	// Adds the client in the zone

    ClientWeakList* lst = zone_list[num_zone];
    if( lst == NULL )
        zone_list[num_zone] = lst = new ClientWeakList;
	lst->push_back( cltw );
    ClientPtr clt(cltw);
	zone_clients[num_zone]++;
	cerr<<zone_clients[num_zone]<<" clients now in zone "<<num_zone<<endl;

	//QVector safevec;
	Unit *addun = clt->game_unit.GetUnit();
	if (addun)
		sts->AddUnit( addun );
	else
		cerr << "dead client attempted to be added to system: refusing\n";
	return sts;
}

/************************************************************************************************/
/**** removeClient                                                                          *****/
/************************************************************************************************/

// Remove a client from its zone
void	ZoneMgr::removeClient( ClientPtr clt )
{
	StarSystem * sts;
	Unit * un = clt->game_unit.GetUnit();
	unsigned int zonenum = 0;
	if (un)
		zonenum = un->activeStarSystem->GetZone();
	for (; zonenum < zone_list.size(); ++zonenum) {
		bool found=false;
		ClientWeakList* lst = zone_list[zonenum];
		
		for (ClientWeakList::iterator q = lst->begin();
			 q!=lst->end();) {
				ClientWeakPtr cwp = *q;
				ClientWeakPtr ocwp (clt);
				if ((!(cwp<ocwp))&&!(ocwp<cwp)) {
					q=lst->erase(q);
					found=true;
					break;
				} else {
					++q;
				}
		}
		if (found)
			break;
	}
	if (zonenum>=zone_list.size()) {
		cerr<<"Client "<<clt->callsign<<" not found in any zone when attempting to remove it"<<endl;
		return;
	}
	zone_clients[zonenum]--;
	cerr<<zone_clients[zonenum]<<" clients left in zone "<<zonenum<<endl;
	if (!un)
		return;
	sts = _Universe->star_system[zonenum];
        if (un->GetHull()<0)
          un->Kill(true,true);
        else
          sts->RemoveUnit( un);
	// SHIP MAY NOT HAVE BEEN KILLED BUT JUST CHANGED TO ANOTHER STAR SYSTEM -> NO KILL
	//un->Kill();
}

/************************************************************************************************/
/**** broadcast : broadcast a packet in a zone                                              *****/
/************************************************************************************************/

// Broadcast a packet to a client's zone clients
void ZoneMgr::broadcast( ClientWeakPtr fromcltw, Packet * pckt, bool isTcp  )
{
    if( fromcltw.expired() )
    {
        cerr<<"Trying to send update without client" << pckt->getCommand() << endl;
        return;
    }
    ClientPtr fromclt( fromcltw );
	Unit * un = fromclt->game_unit.GetUnit();
	if (!un) {
		cerr<<"Trying to broadcast information with dead client unit" << pckt->getCommand() << endl;
		return;
	}
	unsigned short zonenum = un->getStarSystem()->GetZone();
    if( zonenum >= zone_list.size() )
    {
        cerr<<"Trying to send update to nonexistant zone " << zonenum << pckt->getCommand() << endl;
        return;
    }

    // cout<<"Sending update to "<<(zone_list[zonenum].size()-1)<<" clients"<<endl;
    ClientWeakList* lst = zone_list[zonenum];
    if( lst == NULL )
    {
        cerr<<"Trying to send update to nonexistant zone " << zonenum << pckt->getCommand() << endl;
        return;
    }

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt(*i);
		Unit * un2 = clt->game_unit.GetUnit();
        // Broadcast to other clients
        if( (isTcp || clt->ingame) && ((un2==NULL) || (un->GetSerial() != un2->GetSerial())))
        {
            COUT << "BROADCASTING " << pckt->getCommand()
                 << " to client #";
			if (un2)
				cout<<un2->GetSerial();
			else
				cout<<"dead";
			cout << endl;
			if (isTcp) {
				pckt->bc_send( clt->cltadr, clt->tcp_sock);
			} else {
				pckt->bc_send( clt->cltudpadr, *clt->lossy_socket);
			}
        }
    }
}

/************************************************************************************************/
/**** broadcast : broadcast a packet in a zone                                              *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
void	ZoneMgr::broadcast( int zone, ObjSerial serial, Packet * pckt, bool isTcp )
{
	if (zone >= zone_list.size()) {
		return;
	}
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		// Broadcast to all clients including the one who did a request
		// Allow packets to non-ingame clients to get lost if requested UDP.
		if( (isTcp || clt->ingame) /*&& un->GetSerial() != un2->GetSerial()*/ )
		{
//			COUT<<endl;
			if (isTcp) {
				pckt->bc_send( clt->cltadr, clt->tcp_sock);
			} else {
				pckt->bc_send( clt->cltudpadr, *clt->lossy_socket);
			}
		}
	}
}

// Broadcast a packet to a zone clients with its serial as argument but not to the originating client
void	ZoneMgr::broadcastNoSelf( int zone, ObjSerial serial, Packet * pckt, bool isTcp )
{
	if (zone >= zone_list.size()) return;
    ClientWeakList* lst = zone_list[zone];
    if( lst == NULL ) return;

	for( CWLI i=lst->begin(); i!=lst->end(); i++)
	{
        if( (*i).expired() ) continue;

        ClientPtr clt( *i );
		Unit *broadcastTo = clt->game_unit.GetUnit();
		// Broadcast to all clients including the one who did a request
		if( (isTcp || clt->ingame) && ((!broadcastTo) || broadcastTo->GetSerial()!=serial))
		{
			if (isTcp) {
				pckt->bc_send( clt->cltadr, clt->tcp_sock);
			} else {
				pckt->bc_send( clt->cltudpadr, *clt->lossy_socket);
			}
		}
	}
}

/************************************************************************************************/
/**** broadcastSample : broadcast sound sample to players in the zone and on same frequency *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
// NETFIXME: Should this be always TCP?
void	ZoneMgr::broadcastSample( int zone, ObjSerial serial, Packet * pckt, float frequency )
{
	if (zone >= zone_list.size()) return;
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
		if( clt->ingame && clt->comm_freq!=-1 && clt->portaudio && clt->comm_freq==frequency &&
			((!un)||(un->GetSerial() != serial)))
		{
			pckt->bc_send( clt->cltadr, clt->tcp_sock);
		}
	}
}

/************************************************************************************************/
/**** broadcastText : broadcast a text message to players in the zone and on same frequency *****/
/************************************************************************************************/

// Broadcast a packet to a zone clients with its serial as argument
// Always TCP.
void	ZoneMgr::broadcastText( int zone, ObjSerial serial, Packet * pckt, float frequency )
{
	if (zone >= zone_list.size()) return;
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
		if( clt->ingame && clt->comm_freq!=-1 && ((!un)||(un->GetSerial() != serial)) )
		{
			pckt->bc_send( clt->cltadr, clt->tcp_sock);
		}
	}
}

/************************************************************************************************/
/**** broadcastSnapshots                                                                    *****/
/************************************************************************************************/
#include "lin_time.h"
// Broadcast all positions
// This function sends interpolated and predicted positions based on the "semi-ping" between the sender clients and the server
// the receiver client will only have to interpolate and predict on his own "semi-ping" value
// Always UDP.
// NETFIXME:  May be too big for UDP if there are too many ships.  We may want to split these up into reasonable sizes.
void	ZoneMgr::broadcastSnapshots( bool update_planets)
{
	unsigned int i=0;
	CWLI k;

	for( i=0; i<_Universe->star_system.size(); i++)
	{

	}
	//COUT<<"Sending snapshot for ";
	//int h_length = Packet::getHeaderLength();
	// Loop for all systems/zones
	for( i=0; i<zone_list.size(); i++)
	{
		int totalunits=0;
		// Check if system contains player(s)
		if( zone_clients[i]>0)
		{
//			COUT << "BROADCAST SNAPSHOTS = "<<zone_clients[i]<<" clients in zone "<<i<<" time now: "<<queryTime()<<"; frame time: "<<getNewTime() << endl;
			// Loop for all the zone's clients
			for( k=zone_list[i]->begin(); k!=zone_list[i]->end(); k++)
			{
				totalunits=0;
                ClientPtr cltk( *k );
				// If that client is ingame we send to it position info
				if( cltk->ingame==true)
				{
				    int       nbunits=0;
			        Packet    pckt;
                    NetBuffer netbuf;

//                    COUT << "CLEAN NETBUF" << endl;

					// This also means clients will receive info about themselves
                    // which they should ignore or take into account sometimes
                    // (according to their ping value)
					Unit* unit;

					// Add the client we send snapshot to its own deltatime (semi-ping)
					netbuf.addFloat( cltk->getDeltatime() );
//                    COUT << "   *** deltatime " << cltk->getDeltatime() << endl;
					// Clients not ingame are removed from the drawList so it is ok not to test that
					for(un_iter iter = (_Universe->star_system[i]->getUnitList()).createIterator();unit = *iter;++iter){
                                          ++totalunits;
                                          if (unit->GetSerial()!=0) {
						// Only send unit that ate UNITPTR and PLANETPTR+NEBULAPTR if update_planets
						if( (unit->isUnit()==UNITPTR || unit->isUnit()==ASTEROIDPTR || unit->isUnit()==MISSILEPTR) || ((unit->isUnit()==PLANETPTR || unit->isUnit()==NEBULAPTR) && update_planets) )
						{
							ClientState cs( unit);

							/* TEST if it is a client and then set deltatime */
							//ClientPtr cltl;
							//if( (cltl = Server->getClientFromSerial( unit->GetSerial())))
							//{
								//cs.setDelay( cltl->getDeltatime() );
								// This should be moved out of the 'if' when download manager is working
							//}
							bool added = addPosition( cltk, netbuf, unit, cs);
                                                        if( added )
							    ++nbunits;
						}
                                }
					}

					// Send snapshot to client k
					if(nbunits>0)
					{
						//COUT<<"\tsend update for "<<(p+j)<<" clients"<<endl;
						pckt.send( CMD_SNAPSHOT, /*nbclients+*/nbunits,
                                   netbuf.getData(), netbuf.getDataLength(),
                                   SENDANDFORGET, &(cltk->cltudpadr), *cltk->lossy_socket,
                                   __FILE__, PSEUDO__LINE__(337) );
					}
				}
                                {
                                  Unit * unit;
                                  if ((unit=cltk->game_unit.GetUnit())!=NULL){
                                    Unit*targ=unit->Target();
                                    if (targ) {
                                      double range=unit->GetComputerData().radar.maxrange;
                                      unit->GetComputerData().radar.maxrange*=1.5;//generous
                                      if (!unit->InRange(targ,false)) {
                                        unit->Target(NULL);                                 
                                      }
                                      unit->GetComputerData().radar.maxrange=range;
                                    }
                                  }
                                }
                        }
                }
		{
			Unit * unit;

			// Clients not ingame are removed from the drawList so it is ok not to test that
			for(un_iter iter = (_Universe->star_system[i]->getUnitList()).createIterator();unit = *iter;++iter){
				unit->damages = Unit::NO_DAMAGE;

				if (vsrandom.genrand_int31()%(totalunits*10+1) == 1) {
					unit->damages = 0xffff;
				}
			}
		}
	}
}
bool Nearby(ClientPtr clt, Unit * un) {
  Unit * parent=clt->game_unit.GetUnit();
  if (parent) {
    if (un==parent) return true;
    double mm;
    if (!parent->InRange(un,mm,false,false,true)) {
      return false;
    }
    static double maxrange=XMLSupport::parse_float(vs_config->getVariable("server","max_send_range","1e21"));
    if (mm>maxrange)
      return false;
  }else return false;
  return true;
}
bool ZoneMgr::addPosition( ClientPtr client, NetBuffer & netbuf, Unit * un, ClientState & un_cs)
{
	// This test may be wrong for server side units -> may cause more traffic than needed
	if( 1 /* !(un->old_state.getPosition()==un->curr_physical_state.position) ||
             !(un->old_state.getOrientation()==un->curr_physical_state.orientation) */ )
	{
		// Unit 'un' can see Unit 'iter'
		// For now only check if the 'iter' client is in front of Unit 'un')
		if( 1 /*(distance = this->isVisible( source_orient, source_pos, target_pos)) > 0*/)
		{
			
			// Test if client 'l' is far away from client 'k' = test radius/distance<=X
			// So we can send only position
			// Here distance should never be 0
			//ratio = radius/distance;
			if( un->damages||Nearby(client, un) /* ratio > XX client not too far */)
			{
				unsigned char type = ZoneMgr::FullUpdate;
				if (un->damages) {
					type |= ZoneMgr::DamageUpdate;
				}
//                COUT << "   *** FullUpdate ser=" << un->GetSerial() << " cs=" << un_cs << endl;
				// Mark as position+orientation+velocity update
				netbuf.addChar( type );
				netbuf.addShort( un->GetSerial());
				// Put the current client state in
				netbuf.addClientState( un_cs);
				// Throw in some other cheap but useful info.
				netbuf.addFloat (un->energy);
				// Increment the number of clients we send full info about
				if (un->damages) {
					addDamage( netbuf, un );
				}
			}
			// Here find a condition for which sending only position would be enough
			else if( 0 /* ratio>=1 far but still visible */)
			{
//                COUT << "   *** PosUpdate ser=" << un->GetSerial()
//                     << " pos=" << un_cs.getPosition().i
//                     << "," << un_cs.getPosition().j
//                     << "," << un_cs.getPosition().k << endl;
				// Mark as position update only
				netbuf.addChar( ZoneMgr::PosUpdate );
				// Add the client serial
				netbuf.addShort( un->GetSerial());
				netbuf.addQVector( un_cs.getPosition());
				// Increment the number of clients we send limited info about
			}
            else
            {
                static int i=0;
		if (i++%16384==0)
		    COUT << "Client counted but not sent because of ratio: " << un->name<<endl;
                return false;
            }
		}
        else
        {
            COUT << "Client counted but not sent because of distance!" << endl;
            return false;
        }
	}
    else
    {
        COUT << "Client counted but not sent because of position/orientation test!" << endl;
        return false;
    }
    return true;
}

/************************************************************************************************/
/**** broadcastDamage                                                                       *****/
/************************************************************************************************/

// Broadcast all damages
// NETFIXME:  May be too big for UDP.
void	ZoneMgr::broadcastDamage( )
{
	unsigned int i=0;
	CWLI k;
	NetBuffer netbuf;

	//COUT<<"Sending snapshot for ";
	//int h_length = Packet::getHeaderLength();
	// Loop for all systems/zones
	for( i=0; i<zone_list.size(); i++)
	{
		int totalunits=0; 
		// Check if system is non-empty
		if( zone_clients[i]>0)
		{
			/************* Second method : send independently to each client a buffer of its zone  ***************/
			// It allows to check (for a given client) if other clients are far away (so we can only
			// send position, not orientation and stuff) and if other clients are visible to the given
			// client.
			//int	nbclients = 0;
			Packet pckt;

			//cerr<<"BROADCAST DAMAGE = "<<zone_clients[i]<<" clients in zone "<<i<<endl;
			// Loop for all the zone's clients
			for( k=zone_list[i]->begin(); k!=zone_list[i]->end(); k++)
			{
                if( (*k).expired() ) continue;
				int nbunits = 0;
				totalunits = 0;
                ClientPtr cp( *k );
				if( cp->ingame )
				{
					Unit * unit;

					// Clients not ingame are removed from the drawList so it is ok not to test that
					for(un_iter iter = (_Universe->star_system[i]->getUnitList()).createIterator();unit = *iter;++iter){
                                          if (unit->GetSerial()!=0) {
						if( unit->damages)
						{
							// Add the client serial
							netbuf.addSerial( unit->GetSerial());
							addDamage( netbuf, unit);
							++nbunits;
						}
						++totalunits;

                                          }
					}
					// NETFIXME: Should damage updates be UDP or TCP?
					// Send snapshot to client k
					if( netbuf.getDataLength() > 0)
					{
						pckt.send( CMD_SNAPDAMAGE, /*nbclients+*/nbunits,
                                   netbuf.getData(), netbuf.getDataLength(),
                                   SENDANDFORGET, &(cp->cltudpadr), *cp->lossy_socket,
                                   __FILE__, PSEUDO__LINE__(442) );
					}
				}
			}
		}
		{
			Unit * unit;

			// Clients not ingame are removed from the drawList so it is ok not to test that
			for(un_iter iter = (_Universe->star_system[i]->getUnitList()).createIterator();unit = *iter;++iter){
				unit->damages = Unit::NO_DAMAGE;
				if (vsrandom.genrand_int31()%(totalunits*10+1) == 1) {
					unit->damages = 0xffff&(~Unit::COMPUTER_DAMAGED);
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
		cout << "Sent damage " <<damages<<" for unit "<<un->GetSerial()<<" ("<<un->name<<")"<<endl;
		// Put the altered stucts after the damage enum flag
		if( damages & Unit::SHIELD_DAMAGED)
		{
			netbuf.addShield( un->shield);
		}
		if( damages & Unit::ARMOR_DAMAGED)
		{
			netbuf.addArmor( un->armor);
			netbuf.addFloat( un->hull);
		}
		if( damages & Unit::COMPUTER_DAMAGED)
		{
			netbuf.addChar( un->computer.itts);
			netbuf.addChar( un->computer.radar.iff);
			netbuf.addFloat( un->limits.retro);
			netbuf.addFloat( un->computer.radar.maxcone);
			netbuf.addFloat( un->computer.radar.lockcone);
			netbuf.addFloat( un->computer.radar.trackingcone);
			netbuf.addFloat( un->computer.radar.maxrange);
			char c = 1+UnitImages::NUMGAUGES+MAXVDUS;
			netbuf.addChar(c);
			for( it = 0; it<c; it++)
				netbuf.addFloat( un->image->cockpit_damage[it]);
		}
		if( damages & Unit::MOUNT_DAMAGED)
		{
			netbuf.addShort( un->image->ecm);
			netbuf.addShort( un->mounts.size());
			for( it=0; it<un->mounts.size(); it++)
			{
				netbuf.addChar( (char) un->mounts[it].status);

				netbuf.addInt32( un->mounts[it].ammo);
				netbuf.addFloat( un->mounts[it].time_to_lock);
				netbuf.addShort( un->mounts[it].size);
			}
		}
		if( damages & Unit::CARGOFUEL_DAMAGED)
		{
			netbuf.addFloat( un->FuelData());
			netbuf.addFloat( un->AfterburnData());
			netbuf.addFloat( un->image->CargoVolume);
			netbuf.addFloat( un->image->UpgradeVolume);
//			for( it=0; it<un->image->cargo.size(); it++)
//				netbuf.addInt32( un->image->cargo[it].quantity);
		}
		if( damages & Unit::JUMP_DAMAGED)
		{
			netbuf.addChar( un->shield.leak);
			netbuf.addFloat( un->shield.recharge);
			netbuf.addFloat( un->EnergyRechargeData());
			netbuf.addFloat( un->MaxEnergyData());
			netbuf.addFloat( un->jump.energy); //  NETFIXME: Add insys energy too?
			netbuf.addChar( un->jump.damage);
			netbuf.addChar( un->image->repair_droid);
		}
		if( damages & Unit::CLOAK_DAMAGED)
		{
			netbuf.addInt32( un->cloaking);
			netbuf.addFloat( un->image->cloakenergy);
			netbuf.addInt32( un->cloakmin);
			netbuf.addInt32( un->image->cloakrate);
		}
		if( damages & Unit::LIMITS_DAMAGED)
		{
			netbuf.addFloat( un->computer.max_pitch_down);
			netbuf.addFloat( un->computer.max_pitch_up);
			netbuf.addFloat( un->computer.max_yaw_left);
			netbuf.addFloat( un->computer.max_yaw_right);
			netbuf.addFloat( un->computer.max_roll_left);
			netbuf.addFloat( un->computer.max_roll_right);
			netbuf.addFloat( un->limits.roll);
			netbuf.addFloat( un->limits.yaw);
			netbuf.addFloat( un->limits.pitch);
			netbuf.addFloat( un->limits.lateral);
		}
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
		//cout<<"\t\t\tNumber of units :\t"<<zone_units[i]<<endl;
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
		//memunit = zone_units[i]*sizeof( class Unit);
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
