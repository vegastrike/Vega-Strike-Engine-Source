#include "networking/netserver.h"
#include "networking/clientptr.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "vsfilesystem.h"
#include "cmd/unit_factory.h"
#include "networking/fileutil.h"

#include "networking/lowlevel/vsnet_sockethttp.h"
extern string universe_file;
extern bool verify_path (const vector<string> &path, bool allowmpl=false);
extern void vschdirs (const vector<string> &path);
extern void vscdups (const vector<string> &path);
extern vector<vector <string> > lookforUnit( const char * filename, int faction, bool SubU);

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

ClientPtr NetServer::getClientFromSerial( ObjSerial serial)
{
	ClientPtr clt;
	bool	found = false;

	for( LI li=allClients.begin(); li!=allClients.end(); li++)
	{
		clt = (*li);
		Unit *un = clt->game_unit.GetUnit();
		if( un && serial == un->GetSerial())
        {
			found = true;
            break;
        }
	}
	if (!found)
	{
		cerr<<"   WARNING client not found in getClientFromSerial !!!!"<<endl;
		clt.reset();
	}

	return clt;
}

// WARNING: ipadr is NULL since we are getting this packet from acctserver, not the client itself.
bool	NetServer::sendLoginAccept( std::string inetbuf,ClientPtr clt, AddressIP ipadr, int newacct, char flags)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;

    string callsign;
    string passwd;
    NetBuffer netbuf;
	ObjSerial cltserial;
	callsign = getSimpleString(inetbuf);
	passwd = getSimpleString(inetbuf);
	string serverip = getSimpleString(inetbuf);
	string serverport = getSimpleString(inetbuf);
        string savestr=getSimpleString(inetbuf);
        string xmlstr=getSimpleString(inetbuf);
        if (_Universe->star_system.size()) {
          std::string system = _Universe->star_system[0]->getFileName();
          std::string newsystem=savestr.substr(0,savestr.find("^"));
          if (newsystem!=system) {
            sendLoginError( clt, ipadr );
            return false;
          }
          
        }
    if( !clt )
	{
	    // This must be UDP mode, because the client would exist otherwise.
	    // In UDP mode, client is created here.
		clt = newConnection_udp( ipadr ); // WARNING: ipadr is null.  This code is broken anyway.
		if( !clt )
		{
		    COUT << "Error creating new client connection"<<endl;
//			VSExit(1);
		}
	}
//	memcpy( &clt->cltadr, &ipadr, sizeof( AddressIP)); // ipadr is uninitialized... see above.

	clt->callsign = callsign;
	clt->passwd = passwd;

	COUT << "LOGIN REQUEST SUCCESS for <" << callsign << ">" << endl;
	// Verify that client already has a character
	if( newacct)
	{
		COUT << "This account has no ship/char so create one" << endl;
		// Send a command to make the client create a new character/ship
	}
	else
	{
		clt->savegame.resize(0);
		// Get the save parts in a string array
		clt->savegame.push_back( savestr);
		clt->savegame.push_back( xmlstr);
		// Put the save parts in buffers in order to load them properly
		netbuf.Reset();

		string datestr = _Universe->current_stardate.GetFullTrekDate();
		netbuf.addString( datestr);
		netbuf.addString( clt->savegame[0]);
		netbuf.addString( clt->savegame[1]);
		netbuf.addString( universe_file);
#ifdef CRYPTO
		unsigned char * digest = new unsigned char[FileUtil::Hash.DigestSize()];
		FileUtil::HashFileCompute( universe_file, digest, UniverseFile);
		// Add the galaxy filename with relative path to datadir
		netbuf.addShort( FileUtil::Hash.DigestSize() );
		netbuf.addBuffer( digest, FileUtil::Hash.DigestSize());
#else
		netbuf.addShort( 0 );
#endif

		Packet packet2;
		
		// Create a cockpit for the player and parse its savegame
		Cockpit *cp = loadFromSavegame( clt );
		Unit *un = cp->GetParent();
		if (!un) {
			sendLoginError( clt, ipadr );
			return false;
		}
		cltserial = un->GetSerial();

		COUT << ">>> SEND LOGIN ACCEPT =( serial #" << cltserial << " )= --------------------------------------" << endl;
		COUT<<"SAVE="<<clt->savegame[0].length()<<" bytes - XML="<<clt->savegame[1].length()<<" bytes"<<endl;
		cerr<<"SENDING STARDATE : "<<datestr<<endl;
		// Add the initial star system filename + hash if crypto++ support too
		string relsys = cp->savegame->GetStarSystem()+".system";
		netbuf.addString( relsys);
		
		// Generate the starsystem before addclient so that it already contains serials
		StarSystem * sts = zonemgr->addZone( cp->savegame->GetStarSystem());
		
#ifdef CRYPTO
		string sysxml;
		if( (sysxml=zonemgr->getSystem( relsys))!="")
			FileUtil::HashStringCompute( sysxml, digest);
		else
			FileUtil::HashFileCompute( relsys, digest, SystemFile);
		netbuf.addShort( FileUtil::Hash.DigestSize() );
		netbuf.addBuffer( digest, FileUtil::Hash.DigestSize());
		delete digest;
#else
		netbuf.addShort( 0 );
#endif
		
		int zoneid = _Universe->StarSystemIndex(sts);
		netbuf.addShort( zoneid);
	
		packet2.send( LOGIN_ACCEPT, cltserial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(241) );
	}
        return true;
}

Cockpit * NetServer::loadFromSavegame( ClientPtr clt ) {
	ObjSerial cltserial = getUniqueSerial();
	QVector tmpvec( 0, 0, 0);
	bool update = true;
	float credits;
	vector<string> savedships;
	string str("");
	Cockpit *cp = NULL;
	for (int i=0; i<_Universe->numPlayers(); i++) {
		cp = _Universe->AccessCockpit(i);
		if (cp->savegame->GetCallsign() == clt->callsign) {
			// awesome! this player already has a cockpit.
			break;
		} else {
			cp = NULL;
		}
	}
	if (cp == NULL) {
		cp = _Universe->createCockpit( clt->callsign );
		cp->Init ("");
	}
	COUT<<"-> LOADING SAVE FROM NETWORK"<<endl;
	cp->savegame->ParseSaveGame( "", str, "", tmpvec, update, credits, savedships, cltserial, clt->savegame[0], false);
	// Generate the system we enter in if needed and add the client in it

	COUT<<"\tcredits = "<<credits<<endl;
	COUT<<"\tfaction = "<<cp->savegame->GetPlayerFaction()<<endl;
	COUT<<"-> SAVE LOADED"<<endl;

	// WARNING : WE DON'T SAVE FACTION NOR FLIGHTGROUP YET
	COUT<<"-> UNIT FACTORY WITH XML"<<endl;
	// We may have to determine which is the current ship of the player if we handle several ships for one player
	string PLAYER_SHIPNAME = savedships[0];
	string PLAYER_FACTION_STRING = cp->savegame->GetPlayerFaction();

    int saved_faction = FactionUtil::GetFactionIndex( PLAYER_FACTION_STRING);
	//vector<vector <string> > path = lookforUnit( savedships[0].c_str(), saved_faction, false);
	bool exist = true; //(VSFileSystem::LookForFile( savedships[0], VSFileSystem::UnitFile)<=VSFileSystem::Ok);
	static std::string loadfailed ("LOAD_FAILED");
	Unit * un = NULL;
	if (!PLAYER_SHIPNAME.empty()) {
		un = UnitFactory::createUnit( PLAYER_SHIPNAME.c_str(),
                         false,
						 saved_faction,
                         string(""),
                         Flightgroup::newFlightgroup (clt->callsign,PLAYER_SHIPNAME,PLAYER_FACTION_STRING,"default",1,1,"","",mission),
                         0, &clt->savegame[1]);
	}
	if (!un) {
		exist = false;
	} else if (un->name==loadfailed) {
		exist = false;
		un->Kill();
	}
	if( !exist)
	{
		unsigned short serial = cltserial;
		std::string logoutnetbuf;
                addSimpleChar(logoutnetbuf,ACCT_LOGOUT);
		addSimpleString( logoutnetbuf,clt->callsign);
		addSimpleString(logoutnetbuf,clt->passwd);
		// We can't find the unit saved for player -> send a login error
		this->logout( clt );
		Packet p2;
		// Send the account server a logout info
                if (!acct_sock->sendstr(logoutnetbuf)) {
			COUT<<"ERROR sending LOGOUT to account server"<<endl;
		}
		cerr<<"WARNING : Unit file ("<<savedships[0]<<") not found for "<<clt->callsign<<endl;
		return cp;
	}

	COUT<<"\tAFTER UNIT FACTORY WITH XML"<<endl;
	clt->game_unit.SetUnit( un);
	// Assign its serial to client*
	un->SetSerial( cltserial);

	// Affect the created unit to the cockpit
	COUT<<"-> UNIT LOADED"<<endl;

	cp->SetParent( un,PLAYER_SHIPNAME.c_str(),"",tmpvec);
	COUT<<"-> COCKPIT AFFECTED TO UNIT"<<endl;

	COUT<<"SHIP -- "<<savedships[0]<<" -- LOCATION: x="<<tmpvec.i<<",y="<<tmpvec.j<<",z="<<tmpvec.k<<endl;
	COUT<<"<<< SENT LOGIN ACCEPT -----------------------------------------------------------------------"<<endl;
	return cp;
}

void	NetServer::sendLoginError( ClientPtr clt, AddressIP ipadr )
{
	Packet	packet2;
	// Send a login error
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ERROR -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_ERROR, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(255) );
	COUT<<"<<< SENT LOGIN ERROR -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginUnavailable( ClientPtr clt, AddressIP ipadr )
{
	Packet	packet2;
	// Send an unavailable login service
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN UNAVAILABLE -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_UNAVAIL, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(268) );
	COUT<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;

	discList.push_back( clt);
}

void	NetServer::sendLoginAlready( ClientPtr clt, AddressIP ipadr)
{
	// SHOULD NOT WE FREE THE MEMORY OCCUPIED BY A POSSIBLE CLIENT * ???
	Packet	packet2;
	// Send a login error
	// int		retsend;
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ALREADY =( serial #"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
	packet2.send( LOGIN_ALREADY, 0, (char *)NULL, 0, SENDRELIABLE, &ipadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(283) );
	COUT<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
	discList.push_back( clt);
}

