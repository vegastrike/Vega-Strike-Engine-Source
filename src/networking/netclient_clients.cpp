#include "networking/lowlevel/vsnet_debug.h"
#include "networking/client.h"
#include "networking/netclient.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/packet.h"
#include "networking/lowlevel/vsnet_clientstate.h"
#include "cmd/unit_generic.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "networking/prediction.h"
#include "networking/fileutil.h"

/*************************************************************/
/**** Adds an entering client in the actual zone          ****/
/*************************************************************/

void	NetClient::addClient( const Packet* packet )
{
	ObjSerial cltserial = packet->getSerial();
	ClientPtr clt;
	// NOTE : in splitscreen mode we may receive info about us so we comment the following test
	/*
	if( cltserial==this->serial)
	{
		// Maybe just ignore here
		COUT<<"FATAL ERROR : RECEIVED INFO ABOUT ME !"<<endl;
		exit(1);
	}
	*/

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


	//Packet ptmp = packet;
	NetBuffer netbuf( packet->getData(), packet->getDataLength());
	// Should receive the name
	clt->name = netbuf.getString();
	// If not a local player, add it in our array
	if( !isLocalSerial( cltserial))
	{
		// The save buffer and XML buffer come after the ClientState
		//COUT<<"GETTING SAVES FOR NEW CLIENT"<<endl;
		vector<string> saves;
		saves.push_back( netbuf.getString());
		saves.push_back( netbuf.getString());
		//saves = FileUtil::GetSaveFromBuffer( packet->getData()+sizeof( ClientState));
		//COUT<<"SAVE="<<saves[0].length()<<" bytes - XML="<<saves[1].length()<<" bytes"<<endl;

		// We will ignore - starsys as if a client enters he is in the same system
		//                - pos since we received a ClientState
		//                - creds as we don't care about other players' credits for now
		string starsys;
		QVector pos;
		float creds;
		bool update=true;
		vector<string> savedships;
		// Parse the save buffer
		save.ParseSaveGame( "", starsys, "", pos, update, creds, savedships, 0, saves[0], false);

		// WE DON'T STORE FACTION IN SAVE YET
		string PLAYER_FACTION_STRING( "privateer");

		// CREATES THE UNIT... GET SAVE AND XML FROM SERVER
		// Use the first ship if there are more than one -> we don't handle multiple ships for now
		// We name the flightgroup with the player name
		cerr<<"Found saveship[0] = "<<savedships[0]<<endl;
		cerr<<"NEW PLAYER POSITION : x="<<pos.i<<",y="<<pos.j<<"z="<<pos.k<<endl;
		Unit * un = UnitFactory::createUnit( savedships[0].c_str(),
							 false,
							 FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
							 string(""),
							 Flightgroup::newFlightgroup ( callsign,savedships[0],PLAYER_FACTION_STRING,"default",1,1,"","",mission),
							 0, &saves[1]);
		clt->game_unit.SetUnit( un);
		// Set all weapons to inactive
		vector <Mount>
			::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
		for (;i!=un->mounts.end();++i)
			(*i).status=Mount::INACTIVE;
		un->SetNetworkMode( true);
		un->SetSerial( cltserial);
		//COUT<<"Addclient 4"<<endl;

		// Assign new coordinates to client
		un->SetPosition( save.GetPlayerLocation());
		un->SetSerial( cltserial);

		_Universe->activeStarSystem()->AddUnit( un);
		string msg = clt->callsign+" entered the system";
		UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
	}
	// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
	else if( cltserial!=this->game_unit.GetUnit()->GetSerial())
	{
		clt->game_unit.SetUnit( getNetworkUnit( cltserial));
		assert( clt->game_unit.GetUnit() != NULL);
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
		exit( 1);
	}
	Unit * un = clt->game_unit.GetUnit();

	// Removes the unit from starsystem, destroys it and delete client
	_Universe->activeStarSystem()->RemoveUnit(clt->game_unit.GetUnit());
	nbclients--;
	Clients.remove(cltserial);
	un->Kill();
	COUT<<"Leaving client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	string msg = clt->callsign+" leaved the system";
	UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
}

/*************************************************************/
/**** Send an update to server                            ****/
/*************************************************************/

void	NetClient::sendPosition( const ClientState* cs )
{
	// Serial in ClientState is updated in UpdatePhysics code at ClientState creation (with pos, veloc...)
	Packet pckt;
	NetBuffer netbuf;

	// Send the client state
	COUT<<"Sending ClientState == ";
	(*cs).display();
	netbuf.addSerial( cs->getSerial());
	netbuf.addClientState( (*cs));
	pckt.send( CMD_POSUPDATE, this->game_unit.GetUnit()->GetSerial(),
               netbuf.getData(), netbuf.getDataLength(),
               SENDANDFORGET, NULL, this->clt_sock,
               __FILE__, PSEUDO__LINE__(1197) );
}

/**************************************************************/
/**** Update another client position                       ****/
/**************************************************************/

void	NetClient::receivePosition( const Packet* packet )
{
	// When receiving a snapshot, packet serial is considered as the number of client updates
	ClientState		cs;
	const char*		databuf;
	ObjSerial   	sernum=0;
	int				nbclts=0, i, j;
	unsigned char	cmd;
	ClientPtr		clt;
	Unit *			un = NULL;
	bool			localplayer = false;

	nbclts = packet->getSerial();
	COUT << "Received update for " << nbclts << " clients - LENGTH="
	     << packet->getDataLength() << endl;
	databuf = packet->getData();
	NetBuffer netbuf( packet->getData(), packet->getDataLength());

	// Loop throught received snapshot
	for( i=0, j=0; (i+j)<nbclts;)
	{
		// Get the command from buffer
		cmd = netbuf.getChar();
		// Get the serial number of current element
		sernum = netbuf.getSerial();
		// Test if it is a client or a unit
		if( !(clt = Clients.get(sernum)))
		{
			if( !(un = UniverseUtil::GetUnitFromSerial( sernum)))
			{
				COUT<<"WARNING : No client, no unit found for this serial ("<<sernum<<")"<<endl;
			}
		}
		// Test if local player
		else
			localplayer = _Universe->isPlayerStarship( Clients.get(sernum)->game_unit.GetUnit());

		if( clt && !localplayer || un)
		{
			if( clt)
				un = clt->game_unit.GetUnit();

			if( cmd == CMD_FULLUPDATE)
			{
				// Do what needed with update
				COUT<<"Received FULLSTATE ";
				// Tell we received the ClientState so we can convert byte order from network to host
				//cs.display();
				cs = netbuf.getClientState();
				cs.display();

				// Backup old state
				un->BackupState();
				// Update concerned client with predicted position directly in network client list
				un->curr_physical_state.position = cs.getPosition();
				un->curr_physical_state.orientation = cs.getOrientation();
				prediction->InitInterpolation( clt);
				un->curr_physical_state = prediction->Interpolate( clt);
				un->Velocity = cs.getVelocity();
				QVector predpos = un->curr_physical_state.position;
				cerr<<"Predicted location : x="<<predpos.i<<",y="<<predpos.j<<",z="<<predpos.k<<endl;

				i++;
			}
			else if( cmd == CMD_POSUPDATE)
			{
				// Get the serial #
				sernum = netbuf.getShort();
				clt = Clients.get(sernum);
				//COUT<<"Received POSUPDATE for serial "<<sernum<<" -> ";

				// Backup old state
				un->BackupState();
				// Set the new received position in curr_physical_state
				un->curr_physical_state.position = netbuf.getQVector();
				prediction->InitInterpolation( clt);
				un->curr_physical_state.position = prediction->InterpolatePosition( clt);
				//predict( sernum);
				j++;
			}
		}
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
    if( canCompress() ) flags |= CMD_CAN_COMPRESS;
    netbuf.addChar( flags );

	//ClientState cs( this->serial, this->game_unit.GetUnit()->curr_physical_state, this->game_unit.GetUnit()->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !! NOT NEEDED ANYMORE -> THE SERVER ALREADY KNOWS
	//netbuf.addClientState( cs);
	packet2.send( CMD_ADDCLIENT, this->serial,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->clt_sock,
                  __FILE__, PSEUDO__LINE__(1307) );
	this->game_unit.GetUnit()->SetSerial( this->serial);
	COUT << "Sending ingame with serial n°" << this->serial
         << " " << (canCompress() ? "(compress)" : "(no compress)") <<endl;
	this->ingame = true;
	Unit * un = this->game_unit.GetUnit();
	cerr<<"STARTING LOCATION : x="<<un->Position().i<<",y="<<un->Position().j<<",z="<<un->Position().k<<endl;
}

/*************************************************************/
/**** Says we are still alive                             ****/
/*************************************************************/

void NetClient::sendAlive()
{
    if( clt_sock.isTcp() == false )
    {
        Packet	p;
        p.send( CMD_PING, this->game_unit.GetUnit()->GetSerial(),
                (char *)NULL, 0,
                SENDANDFORGET, NULL, this->clt_sock,
                __FILE__, PSEUDO__LINE__(1325) );
    }
}

