#include "networking/netserver.h"
#include "networking/clientptr.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "vsfilesystem.h"
#include "cmd/unit_factory.h"
#include "networking/fileutil.h"

extern string universe_file;
extern bool verify_path (const vector<string> &path, bool allowmpl=false);
extern void vschdirs (const vector<string> &path);
extern void vscdups (const vector<string> &path);
extern vector<vector <string> > lookforUnit( const char * filename, int faction, bool SubU);

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

void	NetServer::authenticate( ClientPtr clt, AddressIP ipadr, Packet& packet )
{
	Packet	packet2;
	string	callsign;
	string	passwd;
	int		i;
	Account *	elem = NULL;
	NetBuffer netbuf( packet.getData(), packet.getDataLength());

	// Get the callsign/passwd from network
	callsign = netbuf.getString();
	passwd = netbuf.getString();

	i=0;
	int found=0;
	for ( VI j=Cltacct.begin(); j!=Cltacct.end() && !found; j++, i++)
	{
		elem = *j;
		if( !elem->compareName( callsign) && !elem->comparePass( passwd))
			found = 1;
	}
	if( !found)
    {
		sendLoginError( clt, ipadr);
    }
	else
	{
		if( elem->isNew())
			sendLoginAccept( clt, ipadr, 1);
		else
			sendLoginAccept( clt, ipadr, 0);
	}
}

ClientPtr NetServer::getClientFromSerial( ObjSerial serial)
{
	ClientPtr clt;
	bool	found = false;

	for( LI li=allClients.begin(); li!=allClients.end(); li++)
	{
		if( serial == (clt=(*li))->game_unit.GetUnit()->GetSerial())
        {
			found = true;
            break;
        }
		if (!found)
		{
			cerr<<"   WARNING client not found in getClientFromSerial !!!!"<<endl;
			clt.reset();
		}
	}

	return clt;
}

void	NetServer::sendLoginAccept( ClientPtr clt, AddressIP ipadr, int newacct)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;

    string callsign;
    string passwd;

	NetBuffer netbuf( packeta.getData(), packeta.getDataLength());
	ObjSerial cltserial;
	callsign = netbuf.getString();
	passwd = netbuf.getString();
	string serverip = netbuf.getString();
	string serverport = netbuf.getString();

    if( !clt )
	{
	    // This must be UDP mode, because the client would exist otherwise.
	    // In UDP mode, client is created here.
		clt = newConnection_udp( ipadr );
		if( !clt )
		{
		    COUT << "Error creating new client connection"<<endl;
			VSExit(1);
		}
	}

	memcpy( &clt->cltadr, &ipadr, sizeof( AddressIP));
	clt->callsign = callsign;
	clt->passwd = passwd;
	cltserial = getUniqueSerial();

	COUT << "LOGIN REQUEST SUCCESS for <" << callsign << ">" << endl;
	// Verify that client already has a character
	if( newacct)
	{
		COUT << "This account has no ship/char so create one" << endl;
		// Send a command to make the client create a new character/ship
	}
	else
	{
		COUT << ">>> SEND LOGIN ACCEPT =( serial n°" << cltserial << " )= --------------------------------------" << endl;
		// Get the save parts in a string array
		vector<string> saves;
		saves.push_back( netbuf.getString());
		saves.push_back( netbuf.getString());
		// Put the save parts in buffers in order to load them properly
		COUT<<"SAVE="<<saves[0].length()<<" bytes - XML="<<saves[1].length()<<" bytes"<<endl;
		netbuf.Reset();
		string datestr = _Universe->current_stardate.GetFullTrekDate();
		cerr<<"SENDING STARDATE : "<<datestr<<endl;
		netbuf.addString( datestr);
		netbuf.addString( saves[0]);
		netbuf.addString( saves[1]);

		string PLAYER_CALLSIGN( clt->name);
		QVector tmpvec( 0, 0, 0);
		bool update = true;
		float credits;
		vector<string> savedships;
		string str("");
		// Create a cockpit for the player and parse its savegame
		Cockpit * cp = _Universe->createCockpit( PLAYER_CALLSIGN);
		cp->Init ("");
		COUT<<"-> LOADING SAVE FROM NETWORK"<<endl;
		cp->savegame->ParseSaveGame( "", str, "", tmpvec, update, credits, savedships, cltserial, saves[0], false);
		// Generate the system we enter in if needed and add the client in it

		COUT<<"\tcredits = "<<credits<<endl;
		COUT<<"\tfaction = "<<cp->savegame->GetPlayerFaction()<<endl;
		COUT<<"-> SAVE LOADED"<<endl;

		// WARNING : WE DON'T SAVE FACTION NOR FLIGHTGROUP YET
		COUT<<"-> UNIT FACTORY WITH XML"<<endl;
		// We may have to determine which is the current ship of the player if we handle several ships for one player
		string PLAYER_SHIPNAME = savedships[0];
		string PLAYER_FACTION_STRING = cp->savegame->GetPlayerFaction();

        int saved_faction = FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str());
		//vector<vector <string> > path = lookforUnit( savedships[0].c_str(), saved_faction, false);
		bool exist = (VSFileSystem::LookForFile( savedships[0], VSFileSystem::UnitFile)<=VSFileSystem::Ok);
		if( !exist)
		{
			// We can't find the unit saved for player -> send a login error
			this->sendLoginError( clt, ipadr);
			Packet p2;
			// Send the account server a logout info
			Unit * un = clt->game_unit.GetUnit();
			if( p2.send( CMD_LOGOUT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(),
						 SENDRELIABLE, NULL, acct_sock, __FILE__,
						 PSEUDO__LINE__(162) ) < 0 )
			{
				COUT<<"ERROR sending LOGOUT to account server"<<endl;
			}
			cerr<<"WARNING : Unit file ("<<savedships[0]<<") not found for "<<callsign<<endl;
			return;
		}

		Unit * un = UnitFactory::createUnit( PLAYER_SHIPNAME.c_str(),
                             false,
							 saved_faction,
                             string(""),
                             Flightgroup::newFlightgroup (PLAYER_CALLSIGN,PLAYER_SHIPNAME,PLAYER_FACTION_STRING,"default",1,1,"","",mission),
                             0, &saves[1]);
		COUT<<"\tAFTER UNIT FACTORY WITH XML"<<endl;
		clt->game_unit.SetUnit( un);
		// Assign its serial to client*
		un->SetSerial( cltserial);

		// Affect the created unit to the cockpit
		COUT<<"-> UNIT LOADED"<<endl;

		cp->SetParent( un,PLAYER_SHIPNAME.c_str(),"",tmpvec);
		COUT<<"-> COCKPIT AFFECTED TO UNIT"<<endl;

        Packet packet2;
		netbuf.addString( universe_file);
#ifdef CRYPTO
		unsigned char * digest = new unsigned char[FileUtil::Hash.DigestSize()];
		FileUtil::HashFileCompute( universe_file, digest, UniverseFile);
		// Add the galaxy filename with relative path to datadir
		netbuf.addBuffer( digest, FileUtil::Hash.DigestSize());
#endif

		// Add the initial star system filename + hash if crypto++ support too
		string relsys = cp->savegame->GetStarSystem()+".system";
		netbuf.addString( relsys);

		// Generate the starsystem before addclient so that it already contains serials
		zonemgr->addZone( cp->savegame->GetStarSystem());
#ifdef CRYPTO
		FileUtil::HashFileCompute( relsys, digest, SystemFile);
		netbuf.addBuffer( digest, FileUtil::Hash.DigestSize());
		delete digest;
#endif
		/*
		cerr<<endl<<"BEGIN FULL BUFFER -------------------------------------------"<<endl<<endl;
		cerr<<netbuf.getData()<<endl;
		cerr<<endl<<"END FULL BUFFER ---------------------------------------------"<<endl<<endl;
		*/
		packet2.send( LOGIN_ACCEPT, cltserial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(241) );
		COUT<<"SHIP -- "<<savedships[0]<<" -- LOCATION: x="<<tmpvec.i<<",y="<<tmpvec.j<<",z="<<tmpvec.k<<endl;
		COUT<<"<<< SENT LOGIN ACCEPT -----------------------------------------------------------------------"<<endl;
	}
}

void	NetServer::sendLoginError( ClientPtr clt, AddressIP ipadr )
{
	Packet	packet2;
	// Send a login error
	SOCKETALT	sockclt;
	if( clt )
		sockclt = clt->sock;
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ERROR -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_ERROR, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(255) );
	COUT<<"<<< SENT LOGIN ERROR -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginUnavailable( ClientPtr clt, AddressIP ipadr )
{
	Packet	packet2;
	// Send an unavailable login service
	SOCKETALT	sockclt;
	if( clt )
		sockclt = clt->sock;
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN UNAVAILABLE -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_UNAVAIL, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(268) );
	COUT<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;

	discList.push_back( clt);
}

void	NetServer::sendLoginAlready( ClientPtr clt, AddressIP ipadr)
{
	// SHOULD NOT WE FREE THE MEMORY OCCUPIED BY A POSSIBLE CLIENT * ???
	Packet	packet2;
	// Send a login error
	// int		retsend;
	SOCKETALT	sockclt;
	if( clt )
		sockclt = clt->sock;
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ALREADY =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
	packet2.send( LOGIN_ALREADY, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(283) );
	COUT<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
	discList.push_back( clt);
}

