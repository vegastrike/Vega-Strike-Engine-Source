/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  NetServer - Network Server Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#include <time.h>
#include <math.h>
#if !defined( _WIN32) || defined( __CYGWIN__)
	#include <unistd.h>
#else
	#include <io.h>
#endif

#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "cmd/weapon_xml.h"
#include "cmd/bolt.h"
#include "gfx/cockpit_generic.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "client.h"
#include "packet.h"
#include "lin_time.h"
#include "netserver.h"
#include "vsnet_serversocket.h"
#include "vsnet_debug.h"
#include "savenet_util.h"
#include "vs_path.h"
#include "networking/netbuffer.h"
#include "networking/vsnet_dloadmgr.h"
#include "cmd/ai/script.h"
#include "cmd/ai/order.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/fireall.h"
#include "cmd/ai/flybywire.h"
#include "cmd/role_bitmask.h"
#include "gfxlib_struct.h"
#include "md5.h"

double	clienttimeout;
double	logintimeout;
int		acct_con;
double  DAMAGE_ATOM;
double	PLANET_ATOM;
double	SAVE_ATOM;

#define MAXINPUT 1024
char	input_buffer[MAXINPUT];
int		nbchars;

string	universe_file;

/**************************************************************/
/**** Constructor / Destructor                             ****/
/**************************************************************/

NetServer::NetServer()
{
	this->nbclients = 0;
	this->nbaccts = 0;
	this->keeprun = 1;
	this->acctserver = 0;
	this->srvtimeout.tv_sec = 0;
	this->srvtimeout.tv_usec = 0;
	/***** number of zones should be determined as server loads zones files *****/
	zonemgr = new ZoneMgr();
	UpdateTime();
	srand( (unsigned int) getNewTime());
	// Here 500 could be something else between 1 and 0xFFFF
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
	globalsave = new SaveGame("");

    _downloadManagerServer.reset( new VsnetDownload::Server::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloadManagerServer );
}

NetServer::~NetServer()
{
	delete zonemgr;
	delete globalsave;
}

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
			exit(1);
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
		Unit * un = UnitFactory::createUnit( PLAYER_SHIPNAME.c_str(),
                             false,
                             FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
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
		unsigned char * mdigest = new unsigned char[MD5_DIGEST_SIZE];
		static string univ = vs_config->getVariable("data","universe_path","universe");
		string reluniv = univ+"/"+universe_file;
		md5Compute( reluniv, mdigest);
		// Add the galaxy filename with relative path to datadir
		netbuf.addString( reluniv);
		netbuf.addBuffer( mdigest, MD5_DIGEST_SIZE);

		// Add the initial star system filename + md5 too
		static string sys = vs_config->getVariable("data","sectors","sectors");
		string relsys = sys+"/"+cp->savegame->GetStarSystem()+".system";
		md5Compute( relsys, mdigest);
		cerr<<"SYSTEM MD5 = "<<mdigest<<endl;
		netbuf.addString( relsys);
		netbuf.addBuffer( mdigest, MD5_DIGEST_SIZE);
		delete mdigest;
		packet2.send( LOGIN_ACCEPT, cltserial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(241) );
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
	packet2.send( LOGIN_ERROR, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(255) );
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
	packet2.send( LOGIN_UNAVAIL, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(268) );
	COUT<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;
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
	packet2.send( LOGIN_ALREADY, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, PSEUDO__LINE__(283) );
	COUT<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void	NetServer::startMsg()
{
	cout<<endl<<"Vegastrike Server version "<<GAMESERVER_VERSION<<endl;
	cout<<"Written by Stephane Vaxelaire"<<endl<<endl;
}

/**************************************************************/
/**** Handles new connections in TCP_MODE                  ****/
/**************************************************************/

ClientPtr NetServer::newConnection_udp( const AddressIP& ipadr )
{
    COUT << " enter " << "NetServer::newConnection_udp" << endl;

    SOCKETALT sock( udpNetwork->get_fd(), SOCKETALT::UDP, ipadr, _sock_set );

    ClientPtr ret = addNewClient( sock, false );
    nbclients++;

    return ret;
}

ClientPtr NetServer::newConnection_tcp( )
{
    ClientPtr ret;

    // Get new connections if there are - do nothing in standard UDP mode
    bool valid = false;
    do
    {
        SOCKETALT sock = tcpNetwork->acceptNewConn( );
        valid = sock.valid();
        if( valid )
        {
            ret = addNewClient( sock, true );
            nbclients++;
        }
    }
    while( valid );
    return ret;
}

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
/**** Start the server loop                                ****/
/**************************************************************/

void	NetServer::start(int argc, char **argv)
{
	string strperiod, strtimeout, strlogintimeout, stracct, strnetatom;
	int periodrecon;
	keeprun = 1;
	double	savetime=0;
	double  reconnect_time = 0;
	double	curtime=0;
	double	snaptime=0;
	double	planettime=0;
	double	damagetime=0;
	acct_con = 1;
	nbchars = 0;
	memset( input_buffer, 0, MAXINPUT);
	Packet p2;

    _sock_set.start( );

	startMsg();

	CONFIGFILE = new char[42];
	strcpy( CONFIGFILE, "vegaserver.config");
	cout<<"Loading server config...";
	initpaths();
	// Here we say we want to only handle activity in all starsystems
	run_only_player_starsystem=false;
	//vs_config = new VegaConfig( SERVERCONFIGFILE);
	cout<<" config loaded"<<endl;
	// Save period in seconds
	strperiod = vs_config->getVariable( "server", "saveperiod", "7200");
	SAVE_ATOM = atoi( strperiod.c_str());
	string strperiodrecon = vs_config->getVariable( "server", "reconnectperiod", "60");
	periodrecon = atoi( strperiodrecon.c_str());
	strtimeout = vs_config->getVariable( "server", "clienttimeout", "20");
	clienttimeout = atoi( strtimeout.c_str());
	strlogintimeout = vs_config->getVariable( "server", "logintimeout", "60");
	logintimeout = atoi( strlogintimeout.c_str());

	strnetatom = vs_config->getVariable( "network", "network_atom", "0.2");
	NETWORK_ATOM = (double) atoi( strnetatom.c_str());
	strnetatom = vs_config->getVariable( "network", "damage_atom", "1");
	DAMAGE_ATOM = (double) atoi( strnetatom.c_str());
	strnetatom = vs_config->getVariable( "network", "planet_atom", "10");
	PLANET_ATOM = (double) atoi( strnetatom.c_str());

	InitTime();
	UpdateTime();
	savetime = 0;
	reconnect_time = getNewTime()+periodrecon;

	string tmp;
	char srvip[256];
	unsigned short tmpport = ACCT_PORT;
	stracct = vs_config->getVariable( "server", "useaccountserver", "");
	acctserver = ( stracct=="true");

	// Create and bind sockets
	COUT << "Initializing TCP server ..." << endl;
	tcpNetwork = NetUITCP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()), _sock_set );
    if( tcpNetwork == NULL )
    {
        COUT << "Couldn't create TCP server - quitting" << endl;
        exit( -100 );
    }

	COUT << "Initializing UDP server ..." << endl;
	udpNetwork = NetUIUDP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()), _sock_set );
    if( udpNetwork == NULL )
    {
        COUT << "Couldn't create UDP server - quitting" << endl;
        exit( -100 );
    }

	COUT << "done." << endl;

	if( !acctserver)
	{
		// Read data files ;)
		cout<<"Loading accounts data... ";
		LoadAccounts( "accounts.xml");
		// Gets hashtable accounts elements and put them in vector Cltacct
		Cltacct = getAllAccounts();
		cout<<Cltacct.size()<<" accounts loaded."<<endl;
	}
	else
	{
		cout<<"Initializing connection to account server..."<<endl;
		if( vs_config->getVariable( "network", "accountsrvip", "")=="")
		{
			cout<<"Account server IP not specified, exiting"<<endl;
			cleanup();
		}
		memset( srvip, 0, 256);
		memcpy( srvip, (vs_config->getVariable( "network", "accountsrvip", "")).c_str(), 256);
		if( vs_config->getVariable( "network", "accountsrvport", "")=="")
			tmpport = ACCT_PORT;
		else
			tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
		acct_sock = NetUITCP::createSocket( srvip, tmpport, _sock_set );
		if( !acct_sock.valid())
		{
			cerr<<"Cannot connect to account server... quitting"<<endl;
			cleanup();
		}
		COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create the _Universe telling it we are on server side
	universe_file = vs_config->getVariable ("server","galaxy","milky_way.xml");
	_Universe = new Universe( argc, argv, universe_file.c_str(), true);
	string strmission = vs_config->getVariable( "server", "missionfile", "networking.mission");
	mission = new Mission( strmission.c_str());
	mission->initMission( false);

	string strstardate = vs_config->getVariable( "server", "initial_stardate", "2000.00:00");
	_Universe->current_stardate.Init( strstardate);
	cout<<"Starting STARDATE : "<<_Universe->current_stardate.GetFullCurrentStarDate()<<endl;

	// Loads dynamic universe
	string dynpath = datadir+"/dynaverse.dat";
	FILE * fp = fopen( dynpath.c_str(), "rb");
	if( !fp)
	{
		cerr<<"!!! ERROR : opening dynamic universe file " << dynpath.c_str() << " !!!"<<endl;
	}
	else
	{
		fseek( fp, 0, SEEK_END);
		int dynsize = ftell( fp);
		fseek( fp, 0, SEEK_SET);
		char * dynaverse = new char[dynsize+1];
		dynaverse[dynsize] = 0;
		int nbread;
		if( (nbread = fread( dynaverse, sizeof( char), dynsize, fp)) != dynsize)
		{
			cerr<<"!!! ERROR : read "<<nbread<<" bytes, there were "<<dynsize<<" to read !!!"<<endl;
			exit(1);
		}

		globalsave->ReadSavedPackets( dynaverse);
	}

	// Server loop
	while( keeprun)
	{
		// int       nb;

		UpdateTime();

		// Check a key press
		//keyset.setReadAlwaysTrue( 0);
		//this->checkKey( keyset);

		// Check received communications
		newConnection_tcp( );
		checkMsg( _sock_set );
		if( acctserver && acct_con )
		{
			// Listen for account server answers
			checkAcctMsg( _sock_set );
			// And send to it the login request we received
			// Then send clients confirmations or errors
		}
		curtime = getNewTime();
		if( acctserver && !acct_con && (curtime - reconnect_time)>periodrecon)
		{
			NetBuffer netbuf;
			curtime += periodrecon;
			// We previously lost connection to account server
			// We try to reconnect
			acct_sock = NetUITCP::createSocket( srvip, tmpport, _sock_set );
			if( acct_sock.valid())
			{
				int nbclients_here = allClients.size();
				LI i;
				int j=0;
				COUT <<">>> Reconnected accountserver on socket "<<acct_sock<<" done."<<endl;
				// Send a list of ingame clients
				// Build a buffer with number of clients and client serials
				int listlen = allClients.size()*sizeof(ObjSerial);
				char * buflist = new char[listlen];
				// Put first the number of clients
				//netbuf.addShort( nbclients);
				for( j=0, i = allClients.begin(); i!=allClients.end(); i++, j++)
				{
					// Add the current client's serial to the buffer
					netbuf.addSerial((*i)->game_unit.GetUnit()->GetSerial());
				}
				// Passing NULL to AddressIP arg because between servers -> only TCP
				// Use the serial packet's field to send the number of clients
				if( p2.send( CMD_RESYNCACCOUNTS, nbclients_here, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(524) ) < 0 )
				{
					perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
					COUT<<"SOCKET was : "<<acct_sock<<endl;
					cleanup();
				}
				delete buflist;
			}
			else
				cerr<<">>> Reconnection to account server failed."<<endl;
		}

		// See if we have some timed out clients and disconnect them
		this->checkTimedoutClients_udp();

		// Remove all clients to be disconnected
		LI j;
		for ( j=discList.begin(); j!=discList.end(); j++)
		{
			disconnect( (*j), __FILE__, PSEUDO__LINE__(543) );
		}
		discList.clear();

		// Remove all clients that logged out
		for ( j=logoutList.begin(); j!=logoutList.end(); j++)
		{
			this->logout( (*j));
		}
		logoutList.clear();

		/****************************** VS STUFF TO DO ************************************/
		// UPDATE STAR SYSTEM -> TO INTEGRATE WITH NETWORKING
		// PROCESS JUMPS -> MAKE UNITS CHANGE THEIR STAR SYSTEM
		  unsigned int i;
		  // static float nonactivesystemtime = XMLSupport::parse_float (vs_config->getVariable ("physics","InactiveSystemTime",".3"));
		  static unsigned int numrunningsystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
		  // float systime=nonactivesystemtime;
		  
		  for (i=0;i<_Universe->star_system.size()&&i<numrunningsystems;i++) {
			//_Universe->star_system[i]->Update((i==0)?1:systime/i,true);
		  }
		  StarSystem::ProcessPendingJumps();
		/****************************** VS STUFF TO DO ************************************/

		snaptime += GetElapsedTime();
		planettime += GetElapsedTime();
		if( snapchanged && snaptime>NETWORK_ATOM)
		{
			//COUT<<"SENDING SNAPSHOT ----------"<<end;
			// If planet time we send planet and nebula info
			if( planettime>PLANET_ATOM)
			{
				zonemgr->broadcastSnapshots( true);
				planettime = 0;
			}
			// Otherwise we just send ships/bases... info
			else
				zonemgr->broadcastSnapshots( false);
			snapchanged = 0;
			snaptime = 0;
		}
#ifndef NET_SHIELD_SYSTEM_1
		damagetime += GetElapsedTime();
		// Time to send shield and damage info
		if( damagetime>DAMAGE_ATOM)
		{
			zonemgr->broadcastDamage();
			damagetime = 0;
		}
#endif

		// Check for automatic server status save time (in seconds)
		//curtime = getNewTime();
		savetime += GetElapsedTime();
		//if( curtime - savetime > period*60)
		if( savetime>SAVE_ATOM)
		{
			// Not implemented
			cout<<">>> Saving server status... ";
			this->save();
			savetime = 0;
			cout<<"done."<<endl;
		}


		_sock_set.waste_time(0,10000);
	}

	delete CONFIGFILE;
	delete vs_config;
	this->closeAllSockets();
}

/**************************************************************/
/**** Check keyboard interaction                           ****/
/**************************************************************/


void	NetServer::checkKey( SocketSet & sets)
{
#if 0
	int		memory_use=0;
	char	c;

	if( sets.select( 0, 0) )
	{
		if( read( 0, &c, 1)==-1)
			cerr<<"Error reading char on std input "<<endl;
		if( c != 0x0a)
		{
			input_buffer[nbchars] = c;
			nbchars++;
		}
		else
		{
			if( !strncmp( input_buffer, "quit", 4) || !strncmp( input_buffer, "QUIT", 4))
			{
				cleanup();
			}
			else if( !strncmp( input_buffer, "stats", 4) || !strncmp( input_buffer, "STATS", 4))
			{
				// Display server stats
				cout<<endl;
				cout<<"-----------------------------------------------"<<endl;
				cout<<"| Server stats                                |"<<endl;
				cout<<"-----------------------------------------------"<<endl<<endl;
				cout<<"\tNumber of loaded and active star systems :\t"<<_Universe->star_system.size()<<endl;
				cout<<"\tNumber of players in all star systems :\t\t"<<allClients.size()<<endl;
				cout<<"\t\tClients : "<<allClients.size()<<endl;
				cout<<"\tNumber of clients waiting for authorization :\t"<<waitList.size()<<endl<<endl;;
				zonemgr->displayStats();
				cout<<"-----------------------------------------------"<<endl;
				cout<<"| End stats                                   |"<<endl;
				cout<<"-----------------------------------------------"<<endl<<endl;
			}
			else if( !strncmp( input_buffer, "mem", 3) || !strncmp( input_buffer, "MEM", 3) || input_buffer[0]=='m' && nbchars==1)
			{
				// Display memory usage
				cout<<endl;
				cout<<"-----------------------------------------------"<<endl;
				cout<<"| Server memory usage                         |"<<endl;
				cout<<"-----------------------------------------------"<<endl<<endl;
				memory_use = sizeof( ServerSocket)*2 + sizeof( class Packet)*2 + sizeof( class SaveGame) + sizeof( class ZoneMgr);
				memory_use += sizeof( int)*5 + sizeof( SOCKETALT) + sizeof( struct timeval);
				// List of clients
				memory_use += sizeof( Client *)*allClients.size() + discList.size()*sizeof( Client *) + waitList.size()*sizeof( struct WaitListEntry);
				cout<<"\tSize of NetServer variables :\t"<<(memory_use/1024)<<" KB ("<<memory_use<<" bytes)"<<endl;
				memory_use += zonemgr->displayMemory();
				cout<<"\t========== TOTAL MEMORY USAGE = "<<(memory_use/1024)<<" KB ("<<memory_use<<" bytes) ==========="<<endl<<endl;
			}

			nbchars = 0;
			memset( input_buffer, 0, MAXINPUT);
		}
	}
#endif
}


/**************************************************************/
/**** Check account server activity                        ****/
/**************************************************************/

void	NetServer::checkAcctMsg( SocketSet& sets )
{
	int len=0;
	AddressIP	ipadr, ip2;
	ClientPtr   clt;
	unsigned char cmd=0;

	// Watch account server socket
	// Get the number of active clients
	if( acct_sock.isActive( ))
	{
		//COUT<<"Net activity !"<<endl;
		// Receive packet and process according to command

		PacketMem mem;
		if( (len=acct_sock.recvbuf( mem, &ip2 ))>0 )
		{
			// Maybe copy that in a "else" condition too if when it fails we have to disconnect a client

			// Here we get the latest client which asked for a login
			// Since coms between game servers and account server are TCP the order of request/answers
			// should be ok and we can use a "queue" for waiting clients
			if( waitList.size()==0)
			{
				cerr<<"Error : trying to remove client on empty waitList"<<" - len="<<len<<endl;
				exit( 1);
			}
            WaitListEntry entry( waitList.front() );
			if( entry.tcp )
			{
			    clt = entry.t;
			    COUT << "Got response for TCP client" << endl;
			}
			else
			{
			    ipadr = entry.u;
				COUT << "Got response for client IP : " << ipadr << endl;
			}
			waitList.pop();

			Packet p( mem );
			packeta = p;
			switch( packeta.getCommand())
			{
				case LOGIN_NEW :
					COUT << ">>> NEW LOGIN =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
					// We received a login authorization for a new account (no ship created)
					this->sendLoginAccept( clt, ipadr, 1);
					COUT << "<<< NEW LOGIN ----------------------------------------------------------------"<<endl;
				break;
				case LOGIN_ACCEPT :
					// Login is ok
					COUT<<">>> LOGIN ACCEPTED =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
					sendLoginAccept( clt, ipadr, 0);
					COUT<<"<<< LOGIN ACCEPTED -----------------------------------------------------------"<<endl;
				break;
				case LOGIN_ERROR :
					COUT<<">>> LOGIN ERROR =( DENIED )= --------------------------------------"<<endl;
					// Login error -> disconnect
					this->sendLoginError( clt, ipadr);
					COUT<<"<<< LOGIN ERROR ---------------------------------------------------"<<endl;
				break;
				case LOGIN_ALREADY :
					COUT<<">>> LOGIN ALREADY =( ALREADY LOGGED IN -> serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
					// Client already logged in -> disconnect
					this->sendLoginAlready( clt, ipadr);
					COUT<<"<<< LOGIN ALREADY --------------------------------------------------------------"<<endl;
				break;
				default:
					COUT<<">>> UNKNOWN COMMAND =( "<<hex<<cmd<<" )= --------------------------------------"<<endl;
			}
		}
		else
		{
			cerr<<"Connection to account server lost !!"<<endl;
			acct_sock.disconnect( __PRETTY_FUNCTION__, false );
			acct_con = 0;
		}
	}
}

/**************************************************************/
/**** Check which clients are sending data to the server   ****/
/**************************************************************/

void	NetServer::checkMsg( SocketSet& sets )
#ifdef VSNET_DEBUG
{
    ostringstream ostr;
    bool          printit = false;
    ostr << "Checking activity on sockets, TCP=";
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        Client* cl = *i;
        if( cl->isTcp() )
        {
		    if( cl->sock.isActive( ) )
		    {
                ostr << cl->sock.get_fd() << "+ ";
                printit = true;
			    this->recvMsg_tcp( cl );
		    }
        }
	}
    ostr << " ";
	if( udpNetwork->isActive( ) )
	{
        ostr << "UDP=" << udpNetwork->get_fd() << "+" << ends;
	    recvMsg_udp( );
        printit = true;
	}
    ostr << ends;
    if( printit ) COUT << ostr.str() << endl;
}
#else
{
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
        if( cl->isTcp() )
        {
		    if( cl->sock.isActive( ) )
		    {
			    this->recvMsg_tcp( cl );
		    }
        }
	}
	if( udpNetwork->isActive( ) )
	{
	    recvMsg_udp( );
	}
}
#endif

/**************************************************************/
/**** Disconnects timed out clients                        ****/
/**************************************************************/

void	NetServer::checkTimedoutClients_udp()
{
	/********* Method 1 : compare latest_timestamp to current time and see if > CLIENTTIMEOUT */
	double curtime = (unsigned int) getNewTime();
	double deltatmp = 0;
	for (LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
        if( cl->isUdp() )
        {
		    deltatmp = (fabs(curtime - cl->latest_timeout));
		    if( cl->latest_timeout!=0)
		    {
			    //COUT<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			    // Here considering a delta > 0xFFFFFFFX where X should be at least something like 0.9
			    // This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			    // side -> when timestamp has grown enough to became bigger than what an u_int can store
			    if( cl->ingame && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.9) )
			    {
	                Unit * un;
				    un = cl->game_unit.GetUnit();
				    cerr<<"ACTIVITY TIMEOUT for client number "<<un->GetSerial()<<endl;
				    COUT<<"\t\tCurrent time : "<<curtime<<endl;
				    COUT<<"\t\tLatest timeout : "<<(cl->latest_timeout)<<endl;
				    COUT<<"t\tDifference : "<<deltatmp<<endl;
                    cl->_disconnectReason = "UDP timeout";
				    discList.push_back( cl );
			    }
		    }
        }
	}
}

/**************************************************************/
/**** Receive a message from a client                      ****/
/**************************************************************/

void	NetServer::recvMsg_tcp( ClientPtr clt )
{
    char	command;
    AddressIP	ipadr;
    // int nbpackets = 0;

    assert( clt );

    SOCKETALT sockclt( clt->sock );
	PacketMem mem;

    int recvbytes = sockclt.recvbuf( mem, &ipadr );

    if( recvbytes < 0 )
    {
	    cerr << ", disconnecting(error)" << endl;
        clt->_disconnectReason = "TCP error";
        discList.push_back( clt );
    }
    else if( recvbytes == 0 )
    {
	    cerr << ", disconnecting(eof)" << endl;
        clt->_disconnectReason = "TCP peer closed";
        discList.push_back( clt );
    }
    else
    {
        Packet packet( mem );
		packet.setNetwork( &ipadr, sockclt );
		command = packet.getCommand( );
        if( clt )
			this->updateTimestamps( clt, packet);

#ifdef VSNET_DEBUG
		COUT << "Created a packet with command " << displayCmd(Cmd(command)) << endl;
	    mem.dump( cerr, 3 );
#endif

		// In TCP we always process
        this->processPacket( clt, command, ipadr, packet );
    }
}

void NetServer::recvMsg_udp( )
{
    SOCKETALT sockclt( udpNetwork->get_fd(), SOCKETALT::UDP, udpNetwork->get_adr(), _sock_set );
    ClientPtr clt;
    AddressIP ipadr;
	bool process = true;

	PacketMem mem;
    int    ret;
    ret = sockclt.recvbuf( mem, &ipadr );
    if( ret > 0 )
    {
        Packet packet( mem );
	    packet.setNetwork( &ipadr, sockclt );

        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

        COUT << "Received from serial : " << nserial << endl;

        // Find the corresponding client
        ClientPtr tmp;
		bool      found = false;
        for( LI i=allClients.begin(); i!=allClients.end(); i++)
        {
            tmp = (*i);
            if( tmp->isUdp() && tmp->game_unit.GetUnit()->GetSerial() == nserial)
            {
                clt = tmp;
                found = 1;
				COUT << " found client " << *(clt.get()) << endl;
				break;
            }
        }
        if( !found && command!=CMD_LOGIN)
        {
            COUT << "Error : non-login message received from unknown client !" << endl;
            // Maybe send an error packet handled by the client
            return;
        }

        // Check if the client's IP is still the same (a very little and unaccurate in some cases protection
		// against spoofing client serial#)
        if (clt && (ipadr!=clt->cltadr))
        {
	    	assert( command != CMD_LOGIN ); // clt should be 0 because ObjSerial was 0

            COUT << "Error : IP changed for client # " << clt->game_unit.GetUnit()->GetSerial() << endl;
            clt->_disconnectReason = "possible IP spoofing";
            discList.push_back( clt );
	    	/* It is not entirely impossible for this to happen; it would be nice
			 * to add an additional identity check. For now we consider it an error.
	     	*/
        }
        else
        {
            if( clt )
				process = this->updateTimestamps( clt, packet);

			// Do not process a packet considered to be late and not important (positions and damage ?)
			if( process)
            	this->processPacket( clt, command, ipadr, packet );
        }
    }
}

// Return true if ok, false if we received a late packet
bool	NetServer::updateTimestamps( ClientPtr cltp, Packet & p )
{
        assert( cltp );
        Client* clt = cltp.get();

		bool 	  ret = true;
        double    ts = p.getTimestamp();

		// We know which client it is so we update its timeout
		double curtime = getNewTime();
		clt->latest_timeout = curtime;

		clt->old_timestamp = clt->latest_timestamp;
		// In case the timestamp is superior to the received tmestamp
		if( ts < clt->latest_timestamp) 
		{
			// If ts > 0xFFFFFFF0 (15 seconds before the maxin an u_int) 
			// This is not really a reliable test -> we may still have late packet in that range of timestamps
			if( (clt->isTcp() && ts > 0xFFFFFFF0) || (clt->isUdp() && ts > 0xFFFFFFF0 && !(p.getFlags() & SENDANDFORGET)) )
				clt->deltatime = 0xFFFFFFFF - clt->latest_timestamp + ts;
			// Only check for late packets when sent non reliable because we need others
			else if( clt->isUdp() && p.getFlags() & SENDANDFORGET)
				ret = false;
		}
		else
			clt->deltatime = (ts - clt->latest_timestamp);
		// Only update latest_timestamp if we didn't receive a late packet (always true for TCP)
		if( ret)
			clt->latest_timestamp = ts;

		return ret;
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::processPacket( ClientPtr clt, unsigned char cmd, const AddressIP& ipadr, Packet& p )
{
    packet = p;

    Packet p2;
	NetBuffer netbuf( packet.getData(), packet.getDataLength());
	int mount_num;
	unsigned short zone;
	char mis;
	// Find the unit
	Unit * un = NULL;
	Unit * unclt = NULL;
	ObjSerial target_serial;
	ObjSerial packet_serial = p.getSerial();

    switch( cmd)
    {
		case CMD_LOGIN:
        {
            COUT<<">>> LOGIN REQUEST --------------------------------------"<<endl;
            // Authenticate client
            // Need to give the IP address of incoming message in UDP mode to store it
            // in the Client struct
            if( !acctserver)
			{
                this->authenticate( clt, ipadr, packet );
			}
			else if( !acct_con)
			{
				this->sendLoginUnavailable( clt, ipadr );
			}
            else
            {
				SOCKETALT tmpsock;
				const AddressIP* iptmp;
				if( clt )
				{
					// This must be a TCP client
					WaitListEntry entry;
					entry.tcp = true;
					entry.t   = clt;
					this->waitList.push( entry );
					iptmp = &clt->cltadr;
					tmpsock = clt->sock;
				}
				else
				{
					WaitListEntry entry;
					entry.tcp = false;
					entry.u   = ipadr;
					this->waitList.push( entry );
					iptmp = &ipadr;
					COUT << "Waiting authorization for client IP : " << ipadr << endl;
				}
				// Redirect the login request packet to account server
				COUT << "Redirecting login request to account server on socket " << acct_sock << endl
				<< "*** Packet to copy length : " << packet.getDataLength()<<endl;
				if( p2.send( packet.getCommand(), 0, (char *)packet.getData(), packet.getDataLength(), SENDANDFORGET, iptmp, acct_sock, __FILE__, PSEUDO__LINE__(1031) ) < 0 )
				{
					perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
					COUT<<"SOCKET was : "<<acct_sock<<endl;
					cleanup();
				}
            }
            COUT<<"<<< LOGIN REQUEST --------------------------------------"<<endl;
        }
        break;
		case CMD_INITIATE:
			this->sendLocations( clt );
			break;
		case CMD_ADDCLIENT:
			// Add the client to the game
			COUT<<">>> ADD REQUEST =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
			//COUT<<"Received ADDCLIENT request"<<endl;
			this->addClient( clt, netbuf.getChar() );
			COUT<<"<<< ADD REQUEST --------------------------------------------------------------"<<endl;
			break;
		case CMD_POSUPDATE:
			// Received a position update from a client
			cerr<<">>> POSITION UPDATE =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
			this->posUpdate( clt );
			cerr<<"<<< POSITION UPDATE ---------------------------------------------------------------"<<endl;
			break;
		case CMD_NEWCHAR:
			// Receive the new char and store it
			this->recvNewChar( clt );
			this->sendLocations( clt );
			break;
		case CMD_LETSGO:
			// Add the client to the game in its zone
			//this->addClient( clt );
			break;
		case CMD_PING:
			// Nothing to do here, just receiving the packet is enough
			//COUT<<"Got PING from serial "<<packet.getSerial()<<endl;
			break;
		case CMD_LOGOUT:
			COUT<<">>> LOGOUT REQUEST =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
			// Client wants to quit the game
			logoutList.push_back( clt );
			COUT<<"<<< LOGOUT REQUEST -----------------------------------------------------------------"<<endl;
			break;
// 		case CMD_ACK :
// 			/*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
// 			/*** We must make sure those 2 conditions are enough ***/
// 			COUT<<">>> ACK =( "<<packet.getTimestamp()<<" )= ---------------------------------------------------"<<endl;
// 			// packet.ack( );
// 			break;

		// SHOULD NOT BE USED ANYMORE
		case CMD_ASKFILE :
		{
			/*
			char nbfiles = netbuf.getChar();
			string file;
			for( char i=0; i<nbfiles; i++)
			{
				file = netbuf.getString();
				// Add the file to a queue in the download thread
				// with client serial in order to send him the file later
				//DownloadQueue.add( file, clt->serial);
			}
			*/
		}
		break;
        case CMD_DOWNLOAD :
			COUT<<">>> CMD DOWNLOAD =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
            if( _downloadManagerServer )
            {
                _downloadManagerServer->addCmdDownload( clt->sock, netbuf );
            }
			COUT<<"<<< CMD DOWNLOAD --------------------------------------------------------------"<<endl;
            break;
		case CMD_FIREREQUEST :
			// Here should put a flag on the concerned mount of the concerned Unit to say we want to fire
			// target_serial is in fact the serial of the firing unit (client itself or turret)
			target_serial = netbuf.getSerial();
			mount_num = netbuf.getInt32();
			zone = netbuf.getShort();
			mis = netbuf.getChar();
			// Find the unit
			// Set the concerned mount as ACTIVE and others as INACTIVE
			un = zonemgr->getUnit( target_serial, zone);
			if( un==NULL)
				COUT<<"ERROR --> Received a fire order for non-existing UNIT"<<endl;
			else
			{
				vector <Mount>
					::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
				for (;i!=un->mounts.end();++i)
					(*i).status=Mount::INACTIVE;
					un->mounts[mount_num].status=Mount::ACTIVE;
				// Ask for fire
				if( mis != 0)
					un->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
				else
					un->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
			}
		break;
		case CMD_JUMP :
		{
			vector<string>	adjacent;
			string newsystem = netbuf.getString();
			unsigned char * client_md5;
			unsigned char * md5 = new unsigned char[MD5_DIGEST_SIZE];
			bool found = false;
			NetBuffer	netbuf2;

			StarSystem * sts;
			Cockpit * cp;
			
			un = clt->game_unit.GetUnit();
			cp = _Universe->isPlayerStarship( un);
			if( un==NULL)
				COUT<<"ERROR --> Received a jump request for non-existing UNIT"<<endl;
			else
			{
					// Verify if there really is a jump point to the new starsystem
					adjacent = _Universe->getAdjacentStarSystems( cp->savegame->GetStarSystem()+".system");
					for( unsigned int i=0; !found && i<adjacent.size(); i++)
					{
						if( adjacent[i]==newsystem)
							found = true;
					}
					if( found)
					{
						// Then activate jump drive to say we want to jump
						un->ActivateJumpDrive();
						// The jump reply is sent in Unit::jumpReactToCollision()
						// In the meantime we create the star system if it isn't loaded yet
						// The starsystem maybe loaded for nothing if the client has not enough warp energy to jump
						// but that's no big deal since they all will be loaded finally
						if( !(sts = _Universe->getStarSystem( newsystem+".system")))
							zonemgr->addZone( newsystem+".system");
						cp->savegame->SetStarSystem( newsystem);

						client_md5 = netbuf.getBuffer( MD5_DIGEST_SIZE);
						if( md5CheckFile( newsystem, client_md5) )
							clt->jumpfile = "";
						/*
						{
							// Send an ASKFILE packet with serial == 0 to say file is ok
							p2.send( CMD_ASKFILE, un->GetSerial(), NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
						}
						*/
						else
						{
							// Add that file to download queue with client serial !!
							clt->jumpfile = newsystem;
						}
					}
					else
					{
						// NOTE THAT THIS SHOULD NOT HAPPEN AND THAT WHEN A CLIENT WANTS TO JUMP IN AN UNKNOW
						// (FROM SERVER POINT OF VIEW) SYSTEM -> IT WILL BE VULNERABLE WHILE IT DOES
						// JUMP ANIMATION AND WILL STAY ALIVE IN ITS CURRENT SYSTEM FOR OTHER PLAYER...
						// TOO BAD FOR CHEATERS ;)

						// Set 0 as serial to say client must stay in its current zone/starsystem
						// p2.send( CMD_JUMP, 0, netbuf2.getData(), netbuf2.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
						clt->jumpfile="error";
					}
			}	
			delete md5;
		}
		break;
		case CMD_SCAN :
		{
			// Received a target scan request
			// WE SHOULD FIND A WAY TO CHECK THAT THE CLIENT HAS THE RIGHT SCAN SYSTEM FOR THAT
			target_serial = netbuf.getSerial();
			zone = netbuf.getShort();
			un = zonemgr->getUnit( target_serial, zone);
			// Get the un Unit data and send it in a packet
			// Here we should get what a scanner could get on the target ship
			// Get the unit that asked for target info
			netbuf.Reset();
			unclt = zonemgr->getUnit( packet.getSerial(), zone);
			//float distance = UnitUtil::getDistance( unclt, un);
			// Add armor data
			/*
			netbuf.addShort();
			netbuf.addShort();
			netbuf.addShort();
			netbuf.addShort();
			*/
			// Add shield data
			//netbuf.addFloat();
			// ??
			// Add hull
			//netbuf.addFloat( un->hull);
			// Add distance
			//netbuf.addFloat( distance);
		}
		break;
		/* SHOULD NOT RECEIVE THIS SINCE COMM SESSIONS ARE HANDLED IN A CLIENT-TO-CLIENT WAY
		case CMD_CAMSHOT :
		{
			p2.bc_create( packet.getCommand(), packet.getSerial(), packet.getData(), packet.getDataLength(), SENDANDFORGET, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1281));
			// Send to concerned clients
			zonemgr->broadcast_camshots( clt->zone, clt->serial, &p2);
		}
		*/
		break;
		case CMD_STARTNETCOMM :
		{
			float freq = netbuf.getFloat();
			clt->comm_freq = freq;
			clt->secured = netbuf.getChar();
			clt->webcam = netbuf.getChar();
			clt->portaudio = netbuf.getChar();
			// Broadcast players with same frequency that there is a new one listening to it
			p2.bc_create( packet.getCommand(), packet_serial, packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1293));
			// Send to concerned clients
			zonemgr->broadcast( clt->zone, packet_serial, &p2);
		}
		break;
		case CMD_STOPNETCOMM :
		{
			clt->comm_freq = -1;
			// float freq = netbuf.getFloat();
			// Broadcast players with same frequency that this client is leaving the comm session
			p2.bc_create( packet.getCommand(), packet_serial, packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1302));
			// Send to concerned clients
			zonemgr->broadcast( clt->zone, packet_serial, &p2);
		}
		break;
		case CMD_SOUNDSAMPLE :
		{
			// Broadcast sound sample to the clients in the same zone and the have PortAudio support
			p2.bc_create( packet.getCommand(), packet_serial, packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1341));
			zonemgr->broadcastSample( clt->zone, packet_serial, &p2, clt->comm_freq);

		}
		case CMD_TXTMESSAGE :
		{
			// Broadcast sound sample to the clients in the same zone and the have PortAudio support
			p2.bc_create( packet.getCommand(), packet_serial, packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1341));
			zonemgr->broadcastText( clt->zone, packet_serial, &p2, clt->comm_freq);

		}

		/***************** NOT USED ANYMORE *******************/
		// SHOULD WE HANDLE A BOLT SERIAL TO UPDATE POSITION ON CLIENT SIDE ???
		// I THINK WE CAN LET THE BOLT GO ON ITS WAY ON CLIENT SIDE BUT THE SERVER WILL DECIDE
		// IF SOMEONE HAS BEEN HIT
		case CMD_BOLT :
		case CMD_BALL :
			  // HERE ONLY SET THE CORRESPONDING MOUNT,UNIT COUPLE TO "FIRE"
			  //p2.bc_create( packet.getCommand(), packet.getSerial(), packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__);
			  //zonemgr->broadcast( clt, &p2 ); // , &NetworkToClient );
			break;
		case CMD_PROJECTILE :
		{
			// DO NOT GET INFO FROM NETWORK - WE HAVE ALL THE INFO ON SERVER SIDE !!!
			// SO ONLY DO WHAT IS NEEDED SO THAT IN THE NEXT STARSYSTEM UPDATE THE PROJECTILE IS "FIRED"

			  // THIS IS TO BE DONE IN MOUNT.CPP
			  // Add the projectile in the client's zone
			  //zonemgr->addUnit( temp, clt->zone);

		  // Finally send an ack to the creation of the created projectile in order to create them on all the clients in
		  // the same zone (send the projectile serial)
		  // We can some day add a check to send only to clients that are in a given range to that projectile
		  //p2.bc_create( packet.getCommand(), temp->GetSerial(), packet.getData(), packet.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__);
		  //zonemgr->broadcast( clt, &p2 ); // , &NetworkToClient );
		}
		break;
    	default:
        	COUT << "Unknown command " << Cmd(cmd) << " ! "
             	 << "from client " << clt->game_unit.GetUnit()->GetSerial() << endl;
    }
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
		netbuf.addString( clt->callsign);
		// Send savebuffer after clientstate
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr);
		netbuf.addString( savestr);
		netbuf.addString( xmlstr);
		// Put the save buffer after the ClientState
		packet2.bc_create( CMD_ENTERCLIENT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1311));
		COUT<<"<<< SEND ENTERCLIENT("<<un->GetSerial()<<") TO OTHER CLIENT IN THE ZONE------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2 ); // , &NetworkToClient );
		COUT<<"Serial : "<<un->GetSerial()<<endl;
		// Send info about other ships in the system to "clt"
		zonemgr->sendZoneClients( clt);
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

	ClientState cs;
	// Set old position
	un->prev_physical_state = un->curr_physical_state;
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

/**************************************************************/
/**** Receive the new player info                          ****/
/**************************************************************/

// Designed to receive character info after a new creation on client-side

void	NetServer::recvNewChar( ClientPtr clt)
{
}

/**************************************************************/
/**** Receive the new player info                          ****/
/**************************************************************/

void	NetServer::sendLocations( ClientPtr clt)
{
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

	// Removes the client from its starsystem
	if( clt->ingame )
		this->removeClient( clt );
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
        if( un )
        {
	        COUT << "User " << clt->callsign << " with serial "<<un->GetSerial()<<" disconnected" << endl;
        }
        else
        {
			COUT<<"!!! ERROR : UNIT==NULL !!!"<<endl;
			// exit(1);
        }
	    COUT << "There were " << allClients.size() << " clients - ";
	    allClients.remove( clt );
	}
	else
	{
        if( un != NULL )
        {
		    Packet p1;
	        p1.send( CMD_DISCONNECT, un->GetSerial(), NULL, 0,
		             SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
                     PSEUDO__LINE__(1432) );
	        COUT << "Client " << un->GetSerial() << " disconnected" << endl;
	        COUT << "There were " << allClients.size() << " clients - ";
	        allClients.remove( clt );
        }
        else
        {
            COUT << "Could not get Unit for " << clt->callsign << endl;
			// exit(1);
        }
	}
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

	// Removes the client from its starsystem
	if( clt->ingame)
		this->removeClient( clt );
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
	    COUT <<"Client "<<un->GetSerial()<<" disconnected"<<endl;
	    COUT <<"There was "<< allClients.size() <<" clients - ";
	    allClients.remove( clt );
	}
	else
	{
	    COUT <<"Client "<<un->GetSerial()<<" disconnected"<<endl;
	    COUT <<"There was "<< allClients.size() <<" clients - ";
	    allClients.remove( clt );
	}
	clt.reset( );
	COUT << allClients.size() <<" clients left"<<endl;
	nbclients--;
}

/**************************************************************/
/**** Close all sockets for shutdown                       ****/
/**************************************************************/

void	NetServer::closeAllSockets()
{
	tcpNetwork->disconnect( "Closing sockets", false );
	udpNetwork->disconnect( "Closing sockets", false );
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
        if( cl->isTcp() )
		    cl->sock.disconnect( __PRETTY_FUNCTION__, false );
	}
}

/**************************************************************/
/**** Save the server state                                ****/
/**************************************************************/

// For now it only save units and player saves
void	NetServer::save()
{
	Packet pckt;
	Cockpit * cp;
	Unit * un;
	FILE * fp=NULL;
	string xmlstr, savestr;
	//unsigned int xmllen, savelen, nxmllen, nsavelen;
	NetBuffer netbuf;

	// Save the Dynamic Universe in the data dir for now
	string dynuniv_path = datadir+"dynaverse.dat";
	fp = fopen( dynuniv_path.c_str(), "w+");
	if( !fp)
	{
		cerr<<"Error opening dynamic universe file"<<endl;
		cleanup();
	}
	else
	{
		string dyn_univ = globalsave->WriteDynamicUniverse();
		int wlen = fwrite( dyn_univ.c_str(), sizeof( char), dyn_univ.length(), fp);
		if( wlen != dyn_univ.length())
		{
			cerr<<"Error writing dynamic universe file"<<endl;
			cleanup();
		}
		fclose( fp);
		fp = NULL;
	}

	// Loop through all clients and write saves
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		cp = _Universe->AccessCockpit( i);
		SaveNetUtil::GetSaveStrings( i, savestr, xmlstr);
		// Write the save and xml unit
		FileUtil::WriteSaveFiles( savestr, xmlstr, datadir+"/serversaves", cp->savegame->GetCallsign());
		// SEND THE BUFFERS TO ACCOUNT SERVER
		if( acctserver && acct_con)
		{
			netbuf.Reset();
			bool found = false;
			// Loop through clients to find the one corresponding to the unit (we need its serial)
			ClientPtr clt = getClientFromSerial( cp->GetParent()->GetSerial());
			if( !clt )
			{
				cerr<<"Error client not found in save process !!!!"<<endl;
				exit(1);
			}
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			//buffer = new char[savestr.length() + xmlstr.length() + 2*sizeof( unsigned int)];
			//SaveNetUtil::GetSaveBuffer( savestr, xmlstr, buffer);
			if( pckt.send( CMD_SAVEACCOUNTS, clt->game_unit.GetUnit()->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1678) ) < 0 )
				COUT<<"ERROR sending SAVE to account server"<<endl;
		}
	}
}

// WEAPON STUFF

void	NetServer::BroadcastUnfire( ObjSerial serial, int weapon_index, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;

	//netbuf.addSerial( serial);
	netbuf.addInt32( weapon_index);

	//p.send( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, __LINE__);
	p.bc_create( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1695) );
	zonemgr->broadcast( zone, serial, &p );
}

// In BroadcastFire we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetServer::BroadcastFire( ObjSerial serial, int weapon_index, ObjSerial missile_serial, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;
	// bool found = false;

	netbuf.addInt32( weapon_index);
	netbuf.addSerial( missile_serial);

	p.bc_create( CMD_FIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1710) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendDamages( ObjSerial serial, unsigned short zone, Shield shield, Armor armor, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addFloat( amt);
	netbuf.addFloat( ppercentage);
	netbuf.addFloat( spercentage);
	netbuf.addVector( pnt);
	netbuf.addVector( normal);
	netbuf.addColor( color);
	netbuf.addShield( shield);
	netbuf.addArmor( armor);

	p.bc_create( CMD_DAMAGE, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1729) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendKill( ObjSerial serial, unsigned short zone)
{
	Packet p;
	Unit * un;

	// Find the client in the udp & tcp client lists in order to set it out of the game (not delete it yet)
	ClientPtr clt = this->getClientFromSerial( serial);
	if( !clt )
	{
		COUT<<"Killed a non client Unit = "<<serial<<endl;
		un = zonemgr->getUnit( serial, zone);
		zonemgr->removeUnit( un, zone);
	}
	else
	{
		COUT<<"Killed client serial = "<<serial<<endl;
		zonemgr->removeClient( clt );
	}

	p.bc_create( CMD_KILL, serial, NULL, 0, SENDRELIABLE, NULL, acct_sock, __FILE__, PSEUDO__LINE__(1771) );
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendJump( ObjSerial serial, bool ok)
{
	Packet p2;
	NetBuffer netbuf;
	string file_content;
	ClientPtr clt = this->getClientFromSerial( serial);

	// Send a CMD_JUMP to tell the client if the jump is allowed
	netbuf.addString( clt->jumpfile );

	// And remove the player from its old starsystem and set it out of game
	this->removeClient( clt );
	// Have to set new starsystem here
	// ??????

	// Test whether the jump was accepted or not by server
	if( ok)
	{
		// If jumpfile is empty the md5 was correct
		if( clt->jumpfile=="" )
			p2.send( CMD_JUMP, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
		// New system file MD5 is wrong tell the client with serial != player serial so he can ask for a new download
		else
			p2.send( CMD_JUMP, serial+1, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
	}
	else if( !ok || clt->jumpfile=="error")
		p2.send( CMD_JUMP, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, PSEUDO__LINE__(1164) );
}

