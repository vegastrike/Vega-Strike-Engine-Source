#include "netserver.h"
#include "vsnet_debug.h"
#include "netbuffer.h"
#include "universe_util.h"
#include "savenet_util.h"

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

ClientPtr NetServer::addNewClient( SOCKETALT sock, bool is_tcp )
{
    ClientPtr newclt( new Client( sock, is_tcp ) );
    // New client -> now registering it in the active client list "Clients"
    // Store the associated socket

    allClients.push_back( newclt);

    COUT << " - Actual number of clients : " << allClients.size() << endl;

    return newclt;
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::addClient( ClientPtr clt, char flags )
{
	Unit * un = clt->game_unit.GetUnit();
	COUT<<">>> SEND ENTERCLIENT =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	Packet packet2;
	string savestr, xmlstr;
	NetBuffer netbuf;
	StarSystem * sts;
	StarSystem * st2;

	QVector safevec;
	Cockpit * cp = _Universe->isPlayerStarship( un);
	string starsys = cp->savegame->GetStarSystem();

	unsigned short zoneid;
	// If we return an existing starsystem we broadcast our info to others
	sts=zonemgr->addClient( clt, starsys, zoneid);

	st2 = _Universe->getStarSystem( starsys+".system");

	// On server side this is not done in Cockpit::SetParent()
	cp->activeStarSystem = st2;
	// Cannot use sts pointer since it may be NULL if the system was just created
	safevec = UniverseUtil::SafeStarSystemEntrancePoint( st2, cp->savegame->GetPlayerLocation(), clt->game_unit.GetUnit()->radial_size);
	COUT<<"\tposition : x="<<safevec.i<<" y="<<safevec.j<<" z="<<safevec.k<<endl;
	cp->savegame->SetPlayerLocation( safevec);
	// UPDATE THE CLIENT Unit's state
	un->SetPosition( safevec);

	if( sts)
	{
		// Send info about other ships in the system to "clt"
		zonemgr->sendZoneClients( clt);

		// Send savebuffers and name
		netbuf.addString( clt->callsign);
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr);
		netbuf.addString( savestr);
		netbuf.addString( xmlstr);
		// Put the save buffer after the ClientState
		packet2.bc_create( CMD_ENTERCLIENT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1311));
		COUT<<"<<< SEND ENTERCLIENT("<<un->GetSerial()<<") TO OTHER CLIENT IN THE ZONE------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2 ); // , &NetworkToClient );
		COUT<<"Serial : "<<un->GetSerial()<<endl;
	}
	// In all case set the zone and send the client the zone which it is in
	COUT<<">>> SEND ADDED YOU =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	un->SetZone( zoneid);
	clt->ingame   = true;
    clt->sock.allowCompress( false );
    if( canCompress() && ( flags & CMD_CAN_COMPRESS ) )
    {
        clt->sock.allowCompress( true );
    }
    else
    {
        flags &= ~CMD_CAN_COMPRESS;
    }
	Packet pp;
	netbuf.Reset();
    netbuf.addChar( flags );
	netbuf.addShort( zoneid);
	netbuf.addString( _Universe->current_stardate.GetFullCurrentStarDate());
	un->BackupState();
	// Add initial position to make sure the client is starting from where we tell him
	netbuf.addQVector( safevec);
	pp.send( CMD_ADDEDYOU, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1325) );

	COUT<<"ADDED client n "<<un->GetSerial()<<" in ZONE "<<clt->zone<<" at STARDATE "<<_Universe->current_stardate.GetFullCurrentStarDate()<<endl;
	//delete cltsbuf;
	//COUT<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
}

/***************************************************************/
/**** Removes a client and notify other clients                */
/***************************************************************/

void	NetServer::removeClient( ClientPtr clt)
{
	Packet packet2;
	Unit * un = clt->game_unit.GetUnit();
	clt->ingame = false;
	// Remove the client from its current starsystem
	zonemgr->removeClient( clt);
	// Broadcast to other players
	packet2.bc_create( CMD_EXITCLIENT, un->GetSerial(), NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1311));
	zonemgr->broadcast( clt, &packet2 );
}

/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/

void	NetServer::posUpdate( ClientPtr clt)
{
	NetBuffer netbuf( packet.getData(), packet.getDataLength());
	Unit * un = clt->game_unit.GetUnit();
	ObjSerial clt_serial = netbuf.getSerial();

	if( clt_serial != un->GetSerial())
	{
		cerr<<"!!! ERROR : Received an update from a serial that differs with the client we found !!!"<<endl;
		exit(1);
	}
	ClientState cs;
	// Set old position
	un->BackupState();
	// Update client position in client list : should be enough like it is below
	cs = netbuf.getClientState();
	un->curr_physical_state.position = cs.getPosition();
	un->curr_physical_state.orientation = cs.getOrientation();
	un->Velocity = cs.getVelocity();
	// deltatime has already been updated when the packet was received
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	cp->savegame->SetPlayerLocation( un->curr_physical_state.position);
	snapchanged = 1;
}

