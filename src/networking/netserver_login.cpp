#include "networking/netserver.h"
#include "networking/clientptr.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/netbuffer.h"
#include "vsfilesystem.h"
#include "cmd/unit_factory.h"
#include "networking/fileutil.h"
#include "networking/savenet_util.h"

#include "networking/lowlevel/vsnet_sockethttp.h"
extern string universe_file;
extern bool verify_path (const vector<string> &path, bool allowmpl=false);
extern void vschdirs (const vector<string> &path);
extern void vscdups (const vector<string> &path);
extern vector<vector <string> > lookforUnit( const char * filename, int faction, bool SubU);
extern Unit& GetUnitMasterPartList();

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
bool	NetServer::loginAccept( std::string inetbuf,ClientPtr clt, int newacct, char flags)
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
		clt->savegame.resize(0);
		// Get the save parts in a string array
		clt->savegame.push_back( savestr);
		clt->savegame.push_back( xmlstr);
        if (_Universe->star_system.size()) {
          std::string system = _Universe->star_system[0]->getFileName();
          std::string newsystem=savestr.substr(0,savestr.find("^"));
          if (newsystem!=system) {
            sendLoginError( clt );
            return false;
          }
          
        }
    if( !clt )
	{
		/*
	    // This must be UDP mode, because the client would exist otherwise.
	    // In UDP mode, client is created here.
		clt = newConnection_udp(  ); // WARNING: ipadr is null.  This code is broken anyway.
		if( !clt )
		{*/
		    COUT << "Error creating new client connection"<<endl;
			return false;
		/*}*/
	}
//	memcpy( &clt->cltadr, &ipadr, sizeof( AddressIP)); // ipadr is uninitialized... see above.

	clt->callsign = callsign;
	clt->passwd = passwd;
	COUT << "LOGIN REQUEST SUCCESS for <" << callsign << ">" << endl;
	if( newacct)
	{
		COUT << "This account has no ship/char so create one" << "(UNIMPLEMENTED)" <<endl;
		sendLoginError(clt);
		return false;
		// Send a command to make the client create a new character/ship
	}
	else
	{
		Cockpit *cp = loadCockpit(clt);
		if (cp) {
			if (loadFromSavegame(clt, cp)) {
				sendLoginAccept(clt, cp);
				COUT<<"<<< SENT LOGIN ACCEPT -----------------------------------------------------------------------"<<endl;
			}
		}
	}
	return true;
}
void NetServer::sendLoginAccept(ClientPtr clt, Cockpit *cp) {
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;
	// Verify that client already has a character
	NetBuffer netbuf;
	Unit *un = cp->GetParent();
	if (!un) {
		sendLoginError( clt );
		return;
	}
	// Put the save parts in buffers in order to load them properly
	netbuf.Reset();

	string datestr = _Universe->current_stardate.GetFullTrekDate();
	netbuf.addString( datestr);
	netbuf.addString( clt->savegame[0]);
	netbuf.addString( clt->savegame[1]);

	Packet packet2;
		
	// Create a cockpit for the player and parse its savegame
	ObjSerial cltserial = un->GetSerial();

	COUT << ">>> SEND LOGIN ACCEPT =( serial #" << cltserial << " )= --------------------------------------" << endl;
	COUT<<"SAVE="<<clt->savegame[0].length()<<" bytes - XML="<<clt->savegame[1].length()<<" bytes"<<endl;
	cerr<<"SENDING STARDATE : "<<datestr<<endl;
	// Add the initial star system filename + hash if crypto++ support too
	string sysname = cp->savegame->GetStarSystem();
	string relsys = sysname+".system";
	netbuf.addString( relsys);
		
	// Generate the starsystem before addclient so that it already contains serials
	StarSystem * sts = zonemgr->addZone( sysname );
		
#ifdef CRYPTO
	string sysxml;
	if(!(sysxml=zonemgr->getSystem( relsys)).empty())
		FileUtil::HashStringCompute( sysxml, digest);
	else if (!sysname.empty())
		FileUtil::HashFileCompute( relsys, digest, SystemFile);
	netbuf.addShort( FileUtil::Hash.DigestSize() );
	netbuf.addBuffer( digest, FileUtil::Hash.DigestSize());
	delete digest;
#else
	netbuf.addShort( 0 );
#endif
		
	int zoneid = _Universe->StarSystemIndex(sts);
	netbuf.addShort( zoneid);

	// Add system string to packet...
	//Long, I know, but is there any other way to keep all the proper graphics-related data that the server discards?
	//netbuf.addString( zonemgr->getSystem(sysname) );
	
	packet2.send( LOGIN_ACCEPT, cltserial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(241) );
}

static void getShipList(vector<string> &ships) {
	Unit *mpl = &GetUnitMasterPartList();
	for (vector<Cargo>::const_iterator iter = mpl->image->cargo.begin(); iter!=mpl->image->cargo.end(); iter++) {
		if ((*iter).GetCategory().substr(0,10) == "starships/") {
			std::string content = (*iter).GetContent();
			std::string::size_type dot = content.find('.');
			//if (dot==std::string::npos) {
				ships.push_back(content);
			//}
		}
	}
}

void NetServer::chooseShip(ClientPtr clt, Packet &p) {
	if (!clt) return;
	if (clt->callsign.empty()){
		sendLoginError(clt);
		return;
	}
	vector<string> ships;
	getShipList(ships);
	
	NetBuffer netbuf(p.getData(), p.getDataLength());
	unsigned short selection = netbuf.getShort();
	string shipname = netbuf.getString();
	if (selection>=ships.size()) {
		sendLoginError(clt);
		return;
	}
	string fighter = ships[selection];
	Cockpit *cp = loadCockpit(clt);
	if (cp) {
		if (loadFromNewGame(clt, cp, fighter)) {
			sendLoginAccept(clt, cp);
		}
	}
}

void NetServer::localLogin( ClientPtr clt, Packet &p) {
	if (!clt) return;
	
	NetBuffer netbuf(p.getData(), p.getDataLength());
	clt->callsign = netbuf.getString();
	string passwd = netbuf.getString();
	if (!this->server_password.empty() && passwd != this->server_password) {
		this->sendLoginError(clt);
		return;
	}
	for (int i=0; i<_Universe->numPlayers(); i++) {
		Cockpit *cp = _Universe->AccessCockpit(i);
		if (cp->savegame && cp->savegame->GetCallsign() == clt->callsign) {
			COUT << "Cannot login player "<<clt->callsign<<": already exists on this server!";
			sendLoginAlready(clt);
			return;
		}
	}
	netbuf.Reset();
	vector<string> ships;
	getShipList(ships);
	netbuf.addShort(ships.size());
	for (vector<string>::const_iterator iter = ships.begin(); iter!=ships.end(); ++iter) {
		netbuf.addString(*iter);
	}
	Packet p1;
	p1.send(CMD_CHOOSESHIP, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(202));
}

Cockpit * NetServer::loadCockpit(ClientPtr clt) {
	Cockpit *cp = NULL;
	for (int i=0;i<_Universe->numPlayers();i++) {
		cp = _Universe->AccessCockpit(i);
		if (cp->savegame->GetCallsign() == clt->callsign) {
			if (clt->loginstate==Client::CONNECTED) {
				sendLoginAlready(clt);
				return NULL;
			} else {
				// already logged in... it *should* already exist.
				return cp;
			}
		}
	}
	cp = NULL;
	if (!unused_players.empty()) {
		cp = _Universe->AccessCockpit(unused_players.back());
		unused_players.pop();
	}
	if (cp == NULL) {
		cp = _Universe->createCockpit( clt->callsign );
	} else {
		cp->recreate(clt->callsign);
	}
	clt->loginstate = Client::LOGGEDIN;
	return cp;
}

bool NetServer::loadFromNewGame( ClientPtr clt, Cockpit *cp, string fighter ) {
	ObjSerial cltserial = getUniqueSerial();
	string PLAYER_SHIPNAME = fighter;
	Mission *mission = NULL;
	if (active_missions.size()>0) {
		mission = active_missions[0];
	}
	if (!mission) {
		COUT << "Cannot login player without acctserver: No missions available";
		sendLoginError(clt);
		return false;
	}
	
    int saved_faction = 0; // NETFIXME: Send faction over network too!
	cp->savegame->SetSavedCredits(XMLSupport::parse_float(mission->getVariable("credits","0")));
	cp->savegame->SetStarSystem(mission->getVariable("system","Sol/Sol"));
	if (!mission->flightgroups.empty()) {
		cp->savegame->SetPlayerFaction(mission->flightgroups[0]->faction);
	}
	COUT<<"\tcredits = "<<cp->savegame->GetSavedCredits()<<endl;
	COUT<<"\tfaction = "<<cp->savegame->GetPlayerFaction()<<endl;
	COUT<<"-> SAVE LOADED"<<endl;
	cp->credits = cp->savegame->GetSavedCredits();

	// WARNING : WE DON'T SAVE FACTION NOR FLIGHTGROUP YET
	COUT<<"-> UNIT FACTORY WITH XML"<<endl;
	// We may have to determine which is the current ship of the player if we handle several ships for one player
	string PLAYER_FACTION_STRING = cp->savegame->GetPlayerFaction();
    saved_faction = FactionUtil::GetFactionIndex( PLAYER_FACTION_STRING);
	
	bool exist = true; //(VSFileSystem::LookForFile( savedships[0], VSFileSystem::UnitFile)<=VSFileSystem::Ok);
	static std::string loadfailed ("LOAD_FAILED");
	Unit * un = NULL;
	if (!PLAYER_SHIPNAME.empty()) {
		un = UnitFactory::createUnit( PLAYER_SHIPNAME.c_str(),
                         false,
						 saved_faction,
                         string(""),
                         Flightgroup::newFlightgroup (clt->callsign,PLAYER_SHIPNAME,PLAYER_FACTION_STRING,"default",1,1,"","",mission),
                         0);
	}
	if (!un) {
		exist = false;
	} else if (un->name==loadfailed) {
		exist = false;
		un->Kill();
	}
	if( !exist)
	{
		// We can't find the unit saved for player -> send a login error
		this->sendLoginError( clt );
		cerr<<"WARNING : Unit file ("<<PLAYER_SHIPNAME<<") not found for "<<clt->callsign<<endl;
		return false;
	}

	COUT<<"\tAFTER UNIT FACTORY WITH XML"<<endl;
	clt->game_unit.SetUnit( un);
	// Assign its serial to client*
	un->SetSerial( cltserial);

	// Affect the created unit to the cockpit
	COUT<<"-> UNIT LOADED"<<endl;

	QVector tmpvec(0,0,0);
	cp->SetParent( un,PLAYER_SHIPNAME.c_str(),"",tmpvec);
	COUT<<"-> COCKPIT AFFECTED TO UNIT"<<endl;
	{
		string savestr, xmlstr;
		clt->savegame.resize(0);
		// Get the save parts in a string array
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr);
		clt->savegame.push_back( savestr);
		clt->savegame.push_back( xmlstr);
	}
	return true;
}

bool NetServer::loadFromSavegame( ClientPtr clt, Cockpit *cp ) {
	ObjSerial cltserial = getUniqueSerial();
	QVector tmpvec( 0, 0, 0);
	bool update = true;
	float credits;
	vector<string> savedships;
	string str("");

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
	return true;
}

void	NetServer::sendLoginError( ClientPtr clt )
{
	Packet	packet2;
	// Send a login error
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ERROR -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_ERROR, 0, (char *)NULL, 0, SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(255) );
	COUT<<"<<< SENT LOGIN ERROR -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginUnavailable( ClientPtr clt )
{
	Packet	packet2;
	// Send an unavailable login service
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN UNAVAILABLE -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_UNAVAIL, 0, (char *)NULL, 0, SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(268) );
	COUT<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;

	discList.push_back( clt);
}

void	NetServer::sendLoginAlready( ClientPtr clt)
{
	// SHOULD NOT WE FREE THE MEMORY OCCUPIED BY A POSSIBLE CLIENT * ???
	Packet	packet2;
	// Send a login error
	// int		retsend;
	//COUT<<"Creating packet... ";
	COUT<<">>> SEND LOGIN ALREADY =( serial #"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
	packet2.send( LOGIN_ALREADY, 0, (char *)NULL, 0, SENDRELIABLE, &clt->cltadr, clt->tcp_sock, __FILE__, PSEUDO__LINE__(283) );
	COUT<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
	discList.push_back( clt);
}

