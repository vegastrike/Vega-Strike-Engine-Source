#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "networking/savenet_util.h"
#include "networking/prediction.h"
#include "lin_time.h"

extern QVector DockToSavedBases( int n);
extern StarSystem * GetLoadedStarSystem( const char * system);

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

ClientPtr NetServer::addNewClient( SOCKETALT &sock )
{
    ClientPtr newclt( new Client( sock ) );
    // New client -> now registering it in thx active client list "Clients"
    // Store the associated socket

    allClients.push_back( newclt);

    COUT << " - Actual number of clients : " << allClients.size() << endl;

    return newclt;
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::addClient( ClientPtr clt)
{
	Unit * un = clt->game_unit.GetUnit();
	COUT<<">>> SEND ENTERCLIENT =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	Packet packet2;
	string savestr, xmlstr;
	NetBuffer netbuf;
	StarSystem * sts;
	StarSystem * st2;

	QVector nullVec( 0, 0, 0);
	int player = _Universe->whichPlayerStarship( un);
	cerr<<"ADDING Player number "<<player<<endl;
	Cockpit * cp = _Universe->AccessCockpit(player);
	string starsys = cp->savegame->GetStarSystem();

	unsigned short zoneid;
	// If we return an existing starsystem we broadcast our info to others
	sts=zonemgr->addClient( clt, starsys, zoneid);

	//st2 = _Universe->getStarSystem( starsys);
	string sysfile = starsys+".system";
	st2 = GetLoadedStarSystem( sysfile.c_str());
	if( !st2)
	{
		cerr<<"!!! ERROR : star system '"<<starsys<<"' not found"<<endl;
		VSExit(1);
	}

	// On server side this is not done in Cockpit::SetParent()
	cp->activeStarSystem = st2;
	un->activeStarSystem = st2;
	// Cannot use sts pointer since it may be NULL if the system was just created
	// Try to see if the player is docked on start

	
// NETFIXME: Dock not yet working!

	
	QVector safevec;//( DockToSavedBases( player));
	if( true) //safevec == nullVec)
	{
		safevec = UniverseUtil::SafeStarSystemEntrancePoint( st2, cp->savegame->GetPlayerLocation(), clt->game_unit.GetUnit()->radial_size);
		cerr<<"PLAYER NOT DOCKED - STARTING AT POSITION : x="<<safevec.i<<",y="<<safevec.j<<",z="<<safevec.k<<endl;
		clt->ingame   = true;
	}
	else
		cerr<<"PLAYER DOCKED - STARTING DOCKED AT POSITION : x="<<safevec.i<<",y="<<safevec.j<<",z="<<safevec.k<<endl;
	COUT<<"\tposition : x="<<safevec.i<<" y="<<safevec.j<<" z="<<safevec.k<<endl;
	cp->savegame->SetPlayerLocation( safevec);
	// UPDATE THE CLIENT Unit's state
	un->SetPosition( safevec);

	if( sts)
	{
		// DO NOT DO THAT HERE ANYMORE (VERY BLOCKING ON SERVER SIDE) -> CLIENT ASKS FOR THE NEW SYSTEM UNITS
		// AND DOWNLOADS INFO
		// Send info about other ships in the system to "clt"
		//zonemgr->sendZoneClients( clt);

		// Send savebuffers and name
		netbuf.addString( clt->callsign);
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr);
		netbuf.addString( savestr);
		netbuf.addString( xmlstr);
		netbuf.addTransformation( un->curr_physical_state );
		// Put the save buffer after the ClientState
		packet2.bc_create( CMD_ENTERCLIENT, un->GetSerial(),
                           netbuf.getData(), netbuf.getDataLength(),
                           SENDRELIABLE,
                           __FILE__, PSEUDO__LINE__(1311));
		COUT<<"<<< SEND ENTERCLIENT("<<un->GetSerial()<<") TO OTHER CLIENT IN THE ZONE------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2, true ); // , &NetworkToClient );
		COUT<<"Serial : "<<un->GetSerial()<<endl;
	}
	// In all case set the zone and send the client the zone which it is in
	COUT<<">>> SEND ADDED YOU =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	un->activeStarSystem->SetZone( zoneid);
	Packet pp;
	netbuf.Reset();
	//netbuf.addShort( zoneid);
	//netbuf.addString( _Universe->current_stardate.GetFullTrekDate());
	un->BackupState();
    clt->setLatestTimestamp(packet.getTimestamp( ));
	clt->last_packet=un->old_state;
	clt->prediction->InitInterpolation(un, un->old_state, 0, clt->getNextDeltatime());
	// Add initial position to make sure the client is starting from where we tell him
	netbuf.addTransformation(un->curr_physical_state);
	getZoneInfo(un->activeStarSystem->GetZone(), netbuf);
	pp.send( CMD_ADDEDYOU, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(1325) );

	COUT<<"ADDED client n "<<un->GetSerial()<<" in ZONE "<<un->activeStarSystem->GetZone()<<" at STARDATE "<<_Universe->current_stardate.GetFullTrekDate()<<endl;
	//delete cltsbuf;
	//COUT<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
}

void	NetServer::serverTimeInitUDP( ClientPtr clt, NetBuffer &netbuf)
{
	Packet p2;
	NetBuffer timeBuf;
	// If client sent an unsigned short.
	if (netbuf.getSize()>1) {
		unsigned short clt_port = netbuf.getShort();
		if (clt_port) {
			AddressIP adr (clt->cltadr, clt_port);
			clt->setUDP( &udpNetwork, adr );
		} else {
			clt->setTCP();
		}
	}
	timeBuf.addDouble(queryTime()); // get most "up-to-date" time.
	// NETFIXME: is SENDANDFORGET really UDP? No.  Use the client's lossy_socket.
	p2.send( CMD_SERVERTIME, 0, timeBuf.getData(), timeBuf.getDataLength(), SENDANDFORGET, &clt->cltudpadr, *clt->lossy_socket, __FILE__, PSEUDO__LINE__(691) );
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
	packet2.bc_create( CMD_EXITCLIENT, un->GetSerial(),
                       NULL, 0, SENDRELIABLE,
                       __FILE__, PSEUDO__LINE__(1311));
	zonemgr->broadcast( clt, &packet2, true );
}

/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/

void	NetServer::posUpdate( ClientPtr clt)
{


//NETFIXME: do a sanity check on the position.



	NetBuffer netbuf( packet.getData(), packet.getDataLength());
	Unit * un = clt->game_unit.GetUnit();
	ObjSerial clt_serial = netbuf.getSerial();
//   clt->setLatestTimestamp(packet.getTimestamp( ));
    clt->setLatestTimestamp(packet.getTimestamp( ));
	clt->elapsed_since_packet = 0;
	if( clt_serial != un->GetSerial())
		{
			cerr<<"!!! ERROR : Received an update from a serial that differs with the client we found !!!"<<endl;
//			VSExit(1);
		}
	ClientState cs;
	// Set old position
	un->BackupState();
	// Update client position in client list : should be enough like it is below
	cs = netbuf.getClientState();
	static bool debugPos = XMLSupport::parse_bool(vs_config->getVariable("network", "debug_position_interpolation", "false"));
	if (debugPos) COUT<<"Received ZoneMgr::PosUpdate from client "<<clt_serial<<"   *** cs="<<cs<< endl;
	un->curr_physical_state.position = cs.getPosition();
	un->curr_physical_state.orientation = cs.getOrientation();
	un->Velocity = cs.getVelocity();

	assert( clt->prediction );
	clt->prediction->InitInterpolation( un, clt->last_packet, clt->getDeltatime(), clt->getNextDeltatime());
//	un->curr_physical_state.position = clt->prediction->InterpolatePosition( un, 0);
	clt->last_packet=cs;
	// deltatime has already been updated when the packet was received
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	cp->savegame->SetPlayerLocation( un->curr_physical_state.position);
	snapchanged = 1;
}

/**************************************************************/
/**** Check if the client has the right system file        ****/
/**************************************************************/

void	NetServer::checkSystem( ClientPtr clt)
{
	// HERE SHOULD CONTROL THE CLIENT HAS THE SAME STARSYSTEM FILE OTHERWISE SEND IT TO CLIENT

	Unit * un = clt->game_unit.GetUnit();
	Cockpit * cp = _Universe->isPlayerStarship( un);

	string starsys = cp->savegame->GetStarSystem();
	// getCRC( starsys+".system");
	// 

}

/**************************************************************/
/**** Disconnect a client                                  ****/
/**************************************************************/

void	NetServer::disconnect( ClientPtr clt, const char* debug_from_file, int debug_from_line )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
         << " *** from " << debug_from_file << ":" << debug_from_line << endl
         << " *** disconnecting " << clt->callsign << " because of "
         << clt->_disconnectReason << endl;

	NetBuffer netbuf;
	Unit * un = clt->game_unit.GetUnit();

	if( acctserver )
	{
        if( un != NULL )
        {
		    // Send a disconnection info to account server
		    netbuf.addString( clt->callsign );
		    netbuf.addString( clt->passwd );
		    Packet p2;
		    if( p2.send( CMD_LOGOUT, un->GetSerial(),
                         netbuf.getData(), netbuf.getDataLength(),
		                 SENDRELIABLE, NULL, acct_sock, __FILE__,
                         PSEUDO__LINE__(1395) ) < 0 )
            {
			    COUT<<"ERROR sending LOGOUT to account server"<<endl;
		    }
		    //p2.display( "", 0);
        }
        else
        {
            COUT << "Could not get Unit for " << clt->callsign << endl;
        }
	}

	clt->tcp_sock.disconnect( __PRETTY_FUNCTION__, false );
	if( un )
    {
	    COUT << "User " << clt->callsign << " with serial "<<un->GetSerial()<<" disconnected" << endl;
    }
    else
    {
		COUT<<"!!! ERROR : UNIT==NULL !!!"<<endl;
		// Never should cause server to exit because of a client error.
		// VSExit(1);
    }
	COUT << "There were " << allClients.size() << " clients - ";
	allClients.remove( clt );
	
	// Removes the client from its starsystem
	if( clt->ingame==true )
		this->removeClient( clt );
	// Say true as 2nd arg because we don't want the server to broadcast since player is leaving hte game
	if( un)
		un->Kill( true, true);
	clt.reset();
	COUT << allClients.size() << " clients left" << endl;
	nbclients--;
}

/*** Same as disconnect but do not respond to client since we assume clean exit ***/
void	NetServer::logout( ClientPtr clt )
{
	Packet p, p1, p2;
	NetBuffer netbuf;
	Unit * un = clt->game_unit.GetUnit();

	if( acctserver)
	{
		// Send a disconnection info to account server
		netbuf.addString( clt->callsign);
		netbuf.addString( clt->passwd);
		COUT<<"Loggin out "<<clt->callsign<<":"<<clt->passwd<<endl;
		Packet p2;
		if( p2.send( CMD_LOGOUT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(),
		             SENDRELIABLE, NULL, acct_sock, __FILE__,
                     PSEUDO__LINE__(1555) ) < 0 )
        {
			COUT<<"ERROR sending LOGOUT to account server"<<endl;
		}
	}

	clt->tcp_sock.disconnect( __PRETTY_FUNCTION__, false );
	COUT <<"Client "<<un->GetSerial()<<" disconnected"<<endl;
	COUT <<"There was "<< allClients.size() <<" clients - ";
	allClients.remove( clt );
	// Removes the client from its starsystem
	if( clt->ingame==true)
		this->removeClient( clt );
	// Say true as 2nd arg because we don't want the server to broadcast since player is leaving hte game
	if( un)
		un->Kill( true, true);
	clt.reset( );
	COUT << allClients.size() <<" clients left"<<endl;
	nbclients--;
}

/************************************************************************************************/
/**** getZoneClients : returns a buffer containing zone info                                *****/
/************************************************************************************************/

// Send one by one a CMD_ADDLCIENT to the client for every ship in the star system we enter
void  NetServer::getZoneInfo( unsigned short zoneid, NetBuffer & netbuf)
{
	CWLI k;
	int nbclients=0;
	Packet packet2;
	string savestr, xmlstr;

	// Loop through client in the same zone to send their current_state and save and xml to "clt"

    ClientWeakList* lst = zonemgr->getZoneList(zoneid);
    if( lst == NULL )
    {
	    COUT << "\t>>> WARNING: Did not send info about " << nbclients << " other ships because of empty (inconsistent?) zone" << endl;
        return;
    }

	for( k=lst->begin(); k!=lst->end(); k++)
	{
        if( (*k).expired() ) continue;
        ClientPtr kp( *k );

		// Test if *k is the same as clt in which case we don't need to send info
		if( kp->ingame)
		{
			SaveNetUtil::GetSaveStrings( kp, savestr, xmlstr);
			// Add the ClientState at the beginning of the buffer -> NO THIS IS IN THE SAVE !!
			//netbuf.addClientState( ClientState( kp->game_unit.GetUnit()));
			// Add the callsign and save and xml strings
			netbuf.addChar( ZoneMgr::AddClient);
			netbuf.addSerial( kp->game_unit.GetUnit()->GetSerial());
			netbuf.addString( kp->callsign);
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			netbuf.addTransformation(kp->game_unit.GetUnit()->curr_physical_state);
			nbclients++;
		}
	}
	netbuf.addChar( ZoneMgr::End);
	COUT<<"\t>>> GOT INFO ABOUT "<<nbclients<<" OTHER SHIPS"<<endl;
}
