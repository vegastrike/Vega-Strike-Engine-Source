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
#include <unistd.h>
#include <math.h>

#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "cmd/weapon_xml.h"
#include "cmd/bolt.h"
#include "gfx/cockpit_generic.h"
#include "universe_util.h"
//#include "universe_util_generic.h" //Use universe_util_generic.h instead
#include "cmd/unit_factory.h"
#include "client.h"
#include "packet.h"
#include "lin_time.h"
#include "netserver.h"
#include "vsnet_serversocket.h"
#include "savenet_util.h"
#include "vs_path.h"
#include "networking/netbuffer.h"
#include "cmd/ai/script.h"
#include "cmd/ai/order.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/fireall.h"
#include "cmd/ai/flybywire.h"
#include "cmd/role_bitmask.h"
#include "gfxlib_struct.h"

double	clienttimeout;
double	logintimeout;
int		acct_con;
double  DAMAGE_ATOM;
double	PLANET_ATOM;
double	SAVE_ATOM;

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
}

NetServer::~NetServer()
{
	delete zonemgr;
	delete globalsave;
}

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

void	NetServer::authenticate( Client * clt, AddressIP ipadr, Packet& packet )
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
		sendLoginError( clt, ipadr);
	else
	{
		if( elem->isNew())
			sendLoginAccept( clt, ipadr, 1);
		else
			sendLoginAccept( clt, ipadr, 0);
	}
}

void	NetServer::sendLoginAccept( Client * clt, AddressIP ipadr, int newacct)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;

    string callsign;
    string passwd;
    //char * buf = packeta.getData();
    //strcpy( name, buf);
    //strcpy( passwd, buf+NAMELEN);
	NetBuffer netbuf( packeta.getData(), packeta.getDataLength());
	ObjSerial cltserial;
	callsign = netbuf.getString();
	passwd = netbuf.getString();

    if( clt == NULL )
	{
	    // This must be UDP mode, because the client would exist otherwise.
	    // In UDP mode, client is created here.
		clt = newConnection_udp( ipadr );
		if( !clt)
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
		//cout<<"Login recv packet size = "<<packeta.getLength()<<endl;
		// Get the save parts in a string array
		vector<string> saves;
		saves.push_back( netbuf.getString());
		saves.push_back( netbuf.getString());
		// Put the save parts in buffers in order to load them properly
		cout<<"SAVE="<<saves[0].length()<<" bytes - XML="<<saves[1].length()<<" bytes"<<endl;

		string PLAYER_CALLSIGN( clt->name);
		QVector tmpvec( 0, 0, 0);
		bool update = true;
		float credits;
		vector<string> savedships;
		string str("");
		// Create a cockpit for the player and parse its savegame
		Cockpit * cp = _Universe->createCockpit( PLAYER_CALLSIGN);
		cp->Init ("");
		cout<<"-> LOADING SAVE FROM NETWORK"<<endl;
		cp->savegame->ParseSaveGame( "", str, "", tmpvec, update, credits, savedships, cltserial, saves[0], false);
		// Generate the system we enter in if needed and add the client in it

		cout<<"\tcredits = "<<credits<<endl;
		cout<<"\tfaction = "<<cp->savegame->GetPlayerFaction()<<endl;
		cout<<"-> SAVE LOADED"<<endl;

		// WARNING : WE DON'T SAVE FACTION NOR FLIGHTGROUP YET
		cout<<"-> UNIT FACTORY WITH XML"<<endl;
		// We may have to determine which is the current ship of the player if we handle several ships for one player
		string PLAYER_SHIPNAME = savedships[0];
		string PLAYER_FACTION_STRING = cp->savegame->GetPlayerFaction();
		Unit * un = UnitFactory::createUnit( PLAYER_SHIPNAME.c_str(),
                             false,
                             FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
                             string(""),
                             Flightgroup::newFlightgroup (PLAYER_CALLSIGN,PLAYER_SHIPNAME,PLAYER_FACTION_STRING,"default",1,1,"","",mission),
                             0, saves[1]);
		cout<<"\tAFTER UNIT FACTORY WITH XML"<<endl;
		clt->game_unit.SetUnit( un);
		// Assign its serial to client*
		un->SetSerial( cltserial);

		// Affect the created unit to the cockpit
		cout<<"-> UNIT LOADED"<<endl;

		cp->SetParent( un,"","",tmpvec);
		cout<<"-> COCKPIT AFFECTED TO UNIT"<<endl;

        Packet packet2;
		packet2.send( LOGIN_ACCEPT, cltserial, packeta.getData(), packeta.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			213
#endif
			);
		cout<<"<<< SENT LOGIN ACCEPT -----------------------------------------------------------------------"<<endl;
	}
}

void	NetServer::sendLoginError( Client * clt, AddressIP ipadr)
{
	Packet	packet2;
	// Send a login error
	SOCKETALT	sockclt;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	cout<<">>> SEND LOGIN ERROR -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_ERROR, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			233
#endif 
			);
	cout<<"<<< SENT LOGIN ERROR -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginUnavailable( Client * clt, AddressIP ipadr)
{
	Packet	packet2;
	// Send an unavailable login service
	SOCKETALT	sockclt;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	cout<<">>> SEND LOGIN UNAVAILABLE -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_UNAVAIL, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			252
#endif
			);
	cout<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginAlready( Client * clt, AddressIP ipadr)
{
	// SHOULD NOT WE FREE THE MEMORY OCCUPIED BY A POSSIBLE CLIENT * ???
	Packet	packet2;
	// Send a login error
	// int		retsend;
	SOCKETALT	sockclt;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	cout<<">>> SEND LOGIN ALREADY =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
	packet2.send( LOGIN_ALREADY, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			273
#endif
			);
	cout<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
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

Client* NetServer::newConnection_udp( const AddressIP& ipadr )
{
    COUT << " enter " << "NetServer::newConnection_udp" << endl;

    SOCKETALT sock( udpNetwork->get_udp_sock(), SOCKETALT::UDP, ipadr );

    Client* ret = addNewClient( sock, false );
    nbclients++;

    return ret;
}

Client* NetServer::newConnection_tcp( SocketSet& set )
{
    SOCKETALT	sock;
    Client*		ret = NULL;

    // Just ignore the client for now

    // Get new connections if there are - do nothing in standard UDP mode
    if( tcpNetwork->isActive( set ) )
    {
        COUT << " enter " << "newConnection_tcp" << endl;
        sock = tcpNetwork->acceptNewConn( set );
        if( sock.valid() )
        {
            ret = this->addNewClient( sock, true );
            nbclients++;
        }
    }
    return ret;
}

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

Client* NetServer::addNewClient( SOCKETALT sock, bool is_tcp )
{
    Client * newclt = new Client( sock, is_tcp );
    // New client -> now registering it in the active client list "Clients"
    // Store the associated socket

    // Adds the client
	if( is_tcp )
        tcpClients.push_back( newclt);
	else
        udpClients.push_back( newclt);
    //COUT << "Added client with socket n°" << sock;
      cout << " - Actual number of clients : "
         << tcpClients.size() + udpClients.size() << endl;

    return newclt;
}

/**************************************************************/
/**** Start the server loop                                ****/
/**************************************************************/

void	NetServer::start(int argc, char **argv)
{
	string strperiod, strtimeout, strlogintimeout, stracct, strnetatom;
	int period, periodrecon;
	keeprun = 1;
	double	savetime=0;
	double  reconnect_time = 0;
	double	curtime=0;
	double	snaptime=0;
	double	planettime=0;
	double	damagetime=0;
	acct_con = 1;
	Packet p2;

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
	savetime = getNewTime()+period;
	reconnect_time = getNewTime()+periodrecon;

	string tmp;
	char srvip[256];
	unsigned short tmpport;
	stracct = vs_config->getVariable( "server", "useaccountserver", "");
	acctserver = ( stracct=="true");
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
		memcpy( srvip, (vs_config->getVariable( "network", "accountsrvip", "")).c_str(), vs_config->getVariable( "network", "accountsrvip", "").length());
		if( vs_config->getVariable( "network", "accountsrvport", "")=="")
			tmpport = ACCT_PORT;
		else
			tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
		acct_sock = NetUITCP::createSocket( srvip, tmpport );
		if( !acct_sock.valid())
		{
			cout<<"Cannot connect to account server... quitting"<<endl;
			cleanup();
		}
		COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create and bind sockets
	COUT << "Initializing TCP server ..." << endl;
	tcpNetwork = NetUITCP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()) );
	COUT << "Initializing UDP server ..." << endl;
	udpNetwork = NetUIUDP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()) );
	COUT << "done." << endl;

	// Create the _Universe telling it we are on server side
	_Universe = new Universe(argc,argv,vs_config->getVariable ("server","galaxy","milky_way.xml").c_str(), true);
	string strmission = vs_config->getVariable( "server", "missionfile", "networking.mission");
	mission = new Mission( strmission.c_str());
	mission->initMission( false);

	// Server loop
	while( keeprun)
	{
		SocketSet set;
		int       nb;

		UpdateTime();
		// Check a key press
		// this->checkKey();
		// Handle new connections in TCP mode
		tcpNetwork->watchForNewConn( set, 0 );

		// Check received communications
		prepareCheckMsg( set );
		if( acctserver && acct_con) prepareCheckAcctMsg( set );
		nb = set.select( NULL );
		if( nb > 0 )
		{
			newConnection_tcp( set );
			checkMsg( set );
			if( acctserver && acct_con)
			{
				// Listen for account server answers
				checkAcctMsg( set );
				// And send to it the login request we received
				// Then send clients confirmations or errors
			}
		}
		curtime = getNewTime();
		if( acctserver && !acct_con && (curtime - reconnect_time)>periodrecon)
		{
			NetBuffer netbuf;
			reconnect_time += periodrecon;
			// We previously lost connection to account server
			// We try to reconnect
			acct_sock = NetUITCP::createSocket( srvip, tmpport );
			if( acct_sock.valid())
			{
				int nbclients_here = udpClients.size() + tcpClients.size();
				LI i;
				int j=0;
				COUT <<">>> Reconnected accountserver on socket "<<acct_sock<<" done."<<endl;
				// Send a list of ingame clients
				// Build a buffer with number of clients and client serials
				int listlen = (tcpClients.size()+udpClients.size())*sizeof(ObjSerial);
				char * buflist = new char[listlen];
				// Put first the number of clients
				//netbuf.addShort( nbclients);
				for( j=0, i = tcpClients.begin(); i!=tcpClients.end(); i++, j++)
				{
					// Add the current client's serial to the buffer
					netbuf.addSerial((*i)->game_unit.GetUnit()->GetSerial());
				}
				for( i = udpClients.begin(); i!=udpClients.end(); i++, j++)
				{
					// Add the current client's serial to the buffer
					netbuf.addSerial((*i)->game_unit.GetUnit()->GetSerial());
				}
				// Passing NULL to AddressIP arg because between servers -> only TCP
				// Use the serial packet's field to send the number of clients
				if( p2.send( CMD_RESYNCACCOUNTS, nbclients_here, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			520
#endif					
					) < 0 )
				{
					perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
					cout<<"SOCKET was : "<<acct_sock<<endl;
					cleanup();
				}
				delete buflist;
			}
			else
				cout<<">>> Reconnection to account server failed."<<endl;
		}

		// See if we have some timed out clients and disconnect them
		this->checkTimedoutClients_udp();

		// Remove all clients to be disconnected
		LI j;
		for ( j=discList.begin(); j!=discList.end(); j++)
		{
			this->disconnect( (*j), __FILE__,
#ifndef _WIN32
			__LINE__
#else
			545
#endif
			);
		}
		discList.clear();

		// Remove all clients that logged out
		for ( j=logoutList.begin(); j!=logoutList.end(); j++)
		{
			this->logout( (*j));
		}
		logoutList.clear();

		snaptime += GetElapsedTime();
		planettime += GetElapsedTime();
		if( snapchanged && snaptime>NETWORK_ATOM)
		{
			//cout<<"SENDING SNAPSHOT ----------"<<end;
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
		// period * 60 because in minutes in the config file
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

		/****************************** VS STUFF TO DO ************************************/
		// UPDATE STAR SYSTEM -> TO INTEGRATE WITH NETWORKING
		// PROCESS JUMPS -> MAKE UNITS CHANGE THEIR STAR SYSTEM
		  unsigned int i;
		  static float nonactivesystemtime = XMLSupport::parse_float (vs_config->getVariable ("physics","InactiveSystemTime",".3"));
		  static unsigned int numrunningsystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
		  float systime=nonactivesystemtime;
		  
		  for (i=0;i<_Universe->star_system.size()&&i<numrunningsystems;i++) {
			//_Universe->star_system[i]->Update((i==0)?1:systime/i,true);
		  }
		  StarSystem::ProcessPendingJumps();
		/****************************** VS STUFF TO DO ************************************/

		micro_sleep(10000);
	}

	delete CONFIGFILE;
	delete vs_config;
	this->closeAllSockets();
}

/**************************************************************/
/**** Check keyboard interaction                           ****/
/**************************************************************/


void	NetServer::checkKey()
{
/*
	fd_set	fd_keyb;
	int		s;
	char	c;

	FD_ZERO( &fd_keyb);
	FD_SET( 0, &fd_keyb);

	if( (s = select( 1, &fd_keyb, NULL, NULL, &srvtimeout))<0)
		perror( "Error readding standard input ");
	if( s>0)
	{
		if( read( 0, &c, 1)==-1)
			cout<<"Error reading char on std input "<<endl;
		if( c != 0x0a)
		{
			if( c == 'Q' || c == 'q')
				keeprun = 0;
		}
	}
*/
}


/**************************************************************/
/**** Check account server activity                        ****/
/**************************************************************/

void	NetServer::prepareCheckAcctMsg( SocketSet& set )
{
    acct_sock.watch( set );
}

void	NetServer::checkAcctMsg( SocketSet& set )
{
	int len=0;
	AddressIP	ipadr, ip2;
	Client *	clt = NULL;
	unsigned char cmd=0;

	// Watch account server socket
	// Get the number of active clients
	if( acct_sock.isActive( set ))
	{
		//cout<<"Net activity !"<<endl;
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
				cout<<"Error : trying to remove client on empty waitList"<<" - len="<<len<<endl;
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
					cout<<">>> LOGIN ERROR =( DENIED )= --------------------------------------"<<endl;
					// Login error -> disconnect
					this->sendLoginError( clt, ipadr);
					cout<<"<<< LOGIN ERROR ---------------------------------------------------"<<endl;
				break;
				case LOGIN_ALREADY :
					cout<<">>> LOGIN ERROR =( ALREADY LOGGED IN -> serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
					// Client already logged in -> disconnect
					this->sendLoginAlready( clt, ipadr);
					cout<<"<<< LOGIN ERROR --------------------------------------------------------------"<<endl;
				break;
				default:
					cout<<">>> UNKNOWN COMMAND =( "<<hex<<cmd<<" )= --------------------------------------"<<endl;
			}
		}
		else
		{
			cout<<"Connection to account server lost !!"<<endl;
			acct_sock.disconnect( __PRETTY_FUNCTION__, false );
			acct_con = 0;
		}
	}
}

/**************************************************************/
/**** Check which clients are sending data to the server   ****/
/**************************************************************/

void	NetServer::prepareCheckMsg( SocketSet& set )
{
	// First add all clients to be watched
	for( LI i=tcpClients.begin(); i!=tcpClients.end(); i++)
	{
		(*i)->sock.watch( set );
	}
	udpNetwork->watch( set );
}

void	NetServer::checkMsg( SocketSet& set )
{
	for( LI i=tcpClients.begin(); i!=tcpClients.end(); i++)
	{
		if( (*i)->sock.isActive( set ) )
		{
			this->recvMsg_tcp( (*i));
		}
	}
	if( udpNetwork->isActive( set ) )
	{
	    recvMsg_udp( );
	}
}

/**************************************************************/
/**** Disconnects timed out clients                        ****/
/**************************************************************/

void	NetServer::checkTimedoutClients_udp()
{
	/********* Method 1 : compare latest_timestamp to current time and see if > CLIENTTIMEOUT */
	double curtime = (unsigned int) getNewTime();
	double deltatmp = 0;
	Unit * un;
	for (LI i=udpClients.begin(); i!=udpClients.end(); i++)
	{
		deltatmp = (fabs(curtime - (*i)->latest_timeout));
		if( (*i)->latest_timeout!=0)
		{
			//cout<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			// Here considering a delta > 0xFFFFFFFF*X where X should be at least something like 0.9
			// This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			// side -> when timestamp has grown enough to became bigger than what an u_int can store
			if( (*i)->ingame && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.8) )
			{
				un = (*i)->game_unit.GetUnit();
				cout<<"ACTIVITY TIMEOUT for client number "<<un->GetSerial()<<endl;
				cout<<"\t\tCurrent time : "<<curtime<<endl;
				cout<<"\t\tLatest timeout : "<<((*i)->latest_timeout)<<endl;
				cout<<"t\tDifference : "<<deltatmp<<endl;
				discList.push_back( *i);
			}
		}
	}
}

/**************************************************************/
/**** Receive a message from a client                      ****/
/**************************************************************/

void	NetServer::recvMsg_tcp( Client * clt)
{
    char	command;
    AddressIP	ipadr;
    // int nbpackets = 0;
    double ts = 0;

    assert( clt != NULL );

    SOCKETALT sockclt( clt->sock );
	PacketMem mem;

    int recvbytes = sockclt.recvbuf( mem, &ipadr );

    if( recvbytes <= 0 )
    {
	//COUT << "received " << recvbytes << " bytes from " << sockclt;
	  cout << ", disconnecting" << endl;
        discList.push_back( clt);
    }
    else
    {
        Packet packet( mem );
	packet.setNetwork( &ipadr, sockclt );
        ts      = packet.getTimestamp();
	command = packet.getCommand( );
        if( clt!=NULL)
        {
            clt->old_timestamp = clt->latest_timestamp;
            // In case the timestamp reached maximum in an u_int
            if( ts < clt->latest_timestamp) 
                clt->deltatime = 0xFFFFFFFF - clt->latest_timestamp + ts;
            else
                clt->deltatime = (ts - clt->latest_timestamp);
            clt->latest_timestamp = ts;
        }

#ifdef VSNET_DEBUG
	COUT << "Created a packet with command " << displayCmd(Cmd(command)) << endl;
	    mem.dump( cout, 3 );
#endif

        this->processPacket( clt, command, ipadr, packet );
    }
}

void NetServer::recvMsg_udp( )
{
    SOCKETALT sockclt( udpNetwork->get_udp_sock(), SOCKETALT::UDP, udpNetwork->get_adr() );
    Client*   clt = NULL;
    AddressIP ipadr;

	PacketMem mem;
    int    ret;
    ret = sockclt.recvbuf( mem, &ipadr );
    if( ret > 0 )
    {
        Packet packet( mem );
	packet.setNetwork( &ipadr, sockclt );

        double    ts      = packet.getTimestamp();
        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

        COUT << "Received from serial : " << nserial << endl;

        // Find the corresponding client
        Client* tmp   = NULL;
	bool    found = false;
        for( LI i=udpClients.begin(); i!=udpClients.end(); i++)
        {
            tmp = (*i);
            if( tmp->game_unit.GetUnit()->GetSerial() == nserial)
            {
                clt = tmp;
                found = 1;
		COUT << " found client " << *clt << endl;
		break;
            }
        }
        if( !found && command!=CMD_LOGIN)
        {
            COUT << "Error : non-login message received from unknown client !" << endl;
            // Maybe send an error packet handled by the client
            return;
        }

        // Check if the client's IP is still the same (a very little protection
		// against spoofing client serial#)
        if( clt!=NULL && ipadr!=clt->cltadr )
        {
	    assert( command != CMD_LOGIN ); // clt should be 0 because ObjSerial was 0

            COUT << "Error : IP changed for client # " << clt->game_unit.GetUnit()->GetSerial() << endl;
            discList.push_back( clt);
	    	/* It is not entirely impossible for this to happen; it would be nice
			 * to add an additional identity check. For now we consider it an error.
	     */
        }
        else
        {
            if( clt != NULL )
            {
                // We know which client it is so we update its timeout
                double curtime = getNewTime();
                clt->latest_timeout = curtime;

                clt->old_timestamp = clt->latest_timestamp;
                // In case the timestamp reached maximum in an u_int
                if( ts < clt->latest_timestamp) 
                    clt->deltatime = 0xFFFFFFFF - clt->latest_timestamp + ts;
                else
                    clt->deltatime = (ts - clt->latest_timestamp);
                clt->latest_timestamp = ts;
            }

            this->processPacket( clt, command, ipadr, packet );
        }
    }
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::processPacket( Client * clt, unsigned char cmd, const AddressIP& ipadr, Packet& p )
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

    switch( cmd)
    {
		case CMD_LOGIN:
        {
            cout<<">>> LOGIN REQUEST --------------------------------------"<<endl;
            // Authenticate client
            // Need to give the IP address of incoming message in UDP mode to store it
            // in the Client struct
            if( !acctserver)
			{
                this->authenticate( clt, ipadr, packet );
			}
			else if( !acct_con)
			{
				this->sendLoginUnavailable( clt, ipadr);
			}
            else
            {
				SOCKETALT tmpsock;
				const AddressIP* iptmp;
				if( clt != NULL )
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
				if( p2.send( packet.getCommand(), 0, (char *)packet.getData(), packet.getDataLength(), SENDANDFORGET, iptmp, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			982
#endif					
					) < 0 )
				{
					perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
					cout<<"SOCKET was : "<<acct_sock<<endl;
					cleanup();
				}
            }
            cout<<"<<< LOGIN REQUEST --------------------------------------"<<endl;
        }
        break;
    case CMD_INITIATE:
        this->sendLocations( clt);
        break;
    case CMD_ADDCLIENT:
        // Add the client to the game
        cout<<">>> ADD REQUEST =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
        //cout<<"Received ADDCLIENT request"<<endl;
        this->addClient( clt);
        cout<<"<<< ADD REQUEST --------------------------------------------------------------"<<endl;
        break;
    case CMD_POSUPDATE:
        // Received a position update from a client
        cerr<<">>> POSITION UPDATE =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
        this->posUpdate( clt);
        cerr<<"<<< POSITION UPDATE ---------------------------------------------------------------"<<endl;
        break;
    case CMD_NEWCHAR:
        // Receive the new char and store it
        this->recvNewChar( clt);
        this->sendLocations( clt);
        break;
    case CMD_LETSGO:
        // Add the client to zone id 1
        // zonemgr->AddClient( clt, 1);
        break;
    case CMD_PING:
		// Nothing to do here, just receiving the packet is enough
        //cout<<"Got PING from serial "<<packet.getSerial()<<endl;
        break;
    case CMD_LOGOUT:
        cout<<">>> LOGOUT REQUEST =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
        // Client wants to quit the game
        logoutList.push_back( clt);
        cout<<"<<< LOGOUT REQUEST -----------------------------------------------------------------"<<endl;
        break;
    case CMD_ACK :
        /*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
        /*** We must make sure those 2 conditions are enough ***/
        cout<<">>> ACK =( "<<packet.getTimestamp()<<" )= ---------------------------------------------------"<<endl;
        packet.ack( );
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
			cout<<"ERROR --> Received a fire order for non-existing UNIT"<<endl;
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
		string newsystem = netbuf.getString();
		StarSystem * sts;
		Cockpit * cp;
		
		un = clt->game_unit.GetUnit();
		if( un==NULL)
			cout<<"ERROR --> Received a jump request for non-existing UNIT"<<endl;
		else
		{
			// Create the new star system if it isn't loaded yet
			if( !(sts = _Universe->getStarSystem( newsystem+".system")))
				zonemgr->addZone( newsystem+".system");
			if( UnitUtil::JumpTo( un, newsystem))
			{
				// Remove unit/client from its old system
				zonemgr->removeClient( clt);
				// Update its star system in its savegame
				cp = _Universe->isPlayerStarship( un);
				cp->savegame->SetStarSystem( newsystem);
				// Add it in the new one
				this->addClient( clt);
				// Send a CMD_JUMP to client with name of system (and md5 string ?)
				netbuf.Reset();
				netbuf.addString( newsystem);
				p2.send( CMD_JUMP, clt->game_unit.GetUnit()->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
					__LINE__
#else
					1076
#endif
					);
			}
		}	
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
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::addClient( Client * clt)
{
	Unit * un = clt->game_unit.GetUnit();
	cout<<">>> SEND ENTERCLIENT =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
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
	cout<<"\tposition : x="<<safevec.i<<" y="<<safevec.j<<" z="<<safevec.k<<endl;
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
		packet2.bc_create( CMD_ENTERCLIENT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1171
#endif
			);
		cout<<"<<< SEND ENTERCLIENT TO OTHER CLIENT IN THE ZONE------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2 ); // , &NetworkToClient );
		cout<<"Serial : "<<un->GetSerial()<<endl;
		// Send info about other ships in the system to "clt"
		zonemgr->sendZoneClients( clt);
	}
	// In all case set the zone and send the client the zone which it is in
	cout<<">>> SEND ADDED YOU =( serial n°"<<un->GetSerial()<<" )= --------------------------------------"<<endl;
	un->SetZone( zoneid);
	clt->ingame = true;
	Packet pp;
	netbuf.Reset();
	netbuf.addShort( zoneid);
	pp.send( CMD_ADDEDYOU, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
		__LINE__
#else
		1190
#endif
		);

	cout<<"ADDED client n "<<un->GetSerial()<<" in ZONE "<<clt->zone<<endl;
	//delete cltsbuf;
	//cout<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
}

/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/

void	NetServer::posUpdate( Client * clt)
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
	// Put deltatime in the delay part of ClientState so that it is send to other clients later
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	cp->savegame->SetPlayerLocation( un->curr_physical_state.position);
	snapchanged = 1;
}

/**************************************************************/
/**** Receive the new player info                          ****/
/**************************************************************/

// Designed to receive character info after a new creation on client-side

void	NetServer::recvNewChar( Client * clt)
{
}

/**************************************************************/
/**** Receive the new player info                          ****/
/**************************************************************/

void	NetServer::sendLocations( Client * clt)
{
}

/**************************************************************/
/**** Disconnect a client                                  ****/
/**************************************************************/

void	NetServer::disconnect( Client * clt, const char* debug_from_file, int debug_from_line )
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl
         << " *** from " << debug_from_file << ":" << debug_from_line << endl;

	NetBuffer netbuf;
	Unit * un = clt->game_unit.GetUnit();

	if( acctserver)
	{
		// Send a disconnection info to account server
		netbuf.addString( clt->callsign);
		netbuf.addString( clt->passwd);
		Packet p2;
		if( p2.send( CMD_LOGOUT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(),
		             SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1225
#endif					 
					 ) < 0 )
        {
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		}
		//p2.display( "", 0);
	}

	// Removes the client from its starsystem
	if( clt->zone>0)
		zonemgr->removeClient( clt);
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
	    COUT << "Client " << un->GetSerial() << " disconnected" << endl;
	    COUT << "There were "
	         << tcpClients.size()+udpClients.size() << " clients - ";
	    tcpClients.remove( clt);
	}
	else
	{
		Packet p1;
	    p1.send( CMD_DISCONNECT, un->GetSerial(), NULL, 0,
		         SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1253
#endif
				 );
	    COUT << "Client " << un->GetSerial() << " disconnected" << endl;
	    COUT << "There were "
	         << tcpClients.size()+udpClients.size() << " clients - ";
	    udpClients.remove( clt);
	}
	// Broadcast client EXIT zone
	Packet p;
	p.bc_create( CMD_EXITCLIENT, un->GetSerial(), NULL, 0,
	             SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1268
#endif
				 );
	zonemgr->broadcast( clt, &p );
	if( clt != NULL)
	{
		delete clt;
		clt = NULL;
	}
	COUT << tcpClients.size()+udpClients.size() << " clients left" << endl;
	nbclients--;
	//exit( 1);
}

/*** Same as disconnect but do not respond to client since we assume clean exit ***/
void	NetServer::logout( Client * clt)
{
	Packet p, p1, p2;
	NetBuffer netbuf;
	Unit * un = clt->game_unit.GetUnit();

	if( acctserver)
	{
		// Send a disconnection info to account server
		netbuf.addString( clt->callsign);
		netbuf.addString( clt->passwd);
		cout<<"Loggin out "<<clt->name<<":"<<clt->passwd<<endl;
		Packet p2;
		if( p2.send( CMD_LOGOUT, un->GetSerial(), netbuf.getData(), netbuf.getDataLength(),
		             SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1299
#endif
			) < 0 )
        {
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		}
	}

	// Removes the client from its starsystem
	if( clt->zone>0)
		zonemgr->removeClient( clt);
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
	    COUT <<"Client "<<un->GetSerial()<<" disconnected"<<endl;
	    COUT <<"There was "<< udpClients.size()+tcpClients.size() <<" clients - ";
	    tcpClients.remove( clt );
	}
	else
	{
	    COUT <<"Client "<<un->GetSerial()<<" disconnected"<<endl;
	    COUT <<"There was "<< udpClients.size()+tcpClients.size() <<" clients - ";
	    udpClients.remove( clt );
	}
	// Broadcast client EXIT zone
	if( clt->zone>0)
	{
		p.bc_create( CMD_EXITCLIENT, un->GetSerial(), NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1330
#endif
			);
		zonemgr->broadcast( clt, &p );
	}
	if( clt != NULL)
	{
		delete clt;
		clt = NULL;
	}
	COUT << udpClients.size()+tcpClients.size() <<" clients left"<<endl;
	nbclients--;
	//exit( 1);
}

/**************************************************************/
/**** Close all sockets for shutdown                       ****/
/**************************************************************/

void	NetServer::closeAllSockets()
{
	tcpNetwork->disconnect( "Closing sockets" );
	udpNetwork->disconnect( "Closing sockets" );
	for( LI i=tcpClients.begin(); i!=tcpClients.end(); i++)
	{
		(*i)->sock.disconnect( __PRETTY_FUNCTION__, false );
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
		cout<<"Error opening dynamic universe file"<<endl;
		cleanup();
	}
	else
	{
		string dyn_univ = globalsave->WriteDynamicUniverse();
		int wlen = fwrite( dyn_univ.c_str(), sizeof( char), dyn_univ.length(), fp);
		if( wlen != dyn_univ.length())
		{
			cout<<"Error writing dynamic universe file"<<endl;
			cleanup();
		}
		fclose( fp);
		fp = NULL;
	}

	// Loop through all cockpits and write save files
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		SaveNetUtil::GetSaveStrings( i, savestr, xmlstr);
		// Write the save and xml unit
		FileUtil::WriteSaveFiles( savestr, xmlstr, datadir+"/serversaves", cp->savegame->GetCallsign());
		// SEND THE BUFFERS TO ACCOUNT SERVER
		if( acctserver && acct_con)
		{
			bool found = false;
			Client * clt;
			netbuf.Reset();
			// Loop through clients to find the one corresponding to the unit (we need its serial)
			for( LI li=udpClients.begin(); li!=udpClients.end(); li++)
			{
				if( (clt=(*li))->game_unit.GetUnit() == un)
					found = true;
			}
			if( !found)
			{
				for( LI li=tcpClients.begin(); li!=tcpClients.end(); li++)
				{
					if( (clt=(*li))->game_unit.GetUnit() == un)
						found = true;
				}
			}
			if (!found)
			{
				cout<<"Error client not found in save process !!!!"<<endl;
				exit(1);
			}
			netbuf.addString( savestr);
			netbuf.addString( xmlstr);
			//buffer = new char[savestr.length() + xmlstr.length() + 2*sizeof( unsigned int)];
			//SaveNetUtil::GetSaveBuffer( savestr, xmlstr, buffer);
			if( pckt.send( CMD_SAVEACCOUNTS, clt->game_unit.GetUnit()->GetSerial(), netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1434
#endif
			) < 0 )
				cout<<"ERROR sending SAVE to account server"<<endl;
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
	p.bc_create( CMD_UNFIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1457
#endif		
		);
	zonemgr->broadcast( zone, serial, &p );
}

// In BroadcastFire we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetServer::BroadcastFire( ObjSerial serial, int weapon_index, ObjSerial missile_serial, unsigned short zone)
{
	Packet p;
	NetBuffer netbuf;
	bool found = false;

	netbuf.addInt32( weapon_index);
	netbuf.addSerial( missile_serial);

	p.bc_create( CMD_FIREREQUEST, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1478
#endif
		);
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendDamages( ObjSerial serial, unsigned short zone, Unit::Shield shield, Unit::Armor armor, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color)
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

	p.bc_create( CMD_DAMAGE, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1541
#endif
		);
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

void	NetServer::sendKill( ObjSerial serial, unsigned short zone)
{
	Packet p;
	bool found = false;
	Client * clt;
	Unit * un;

	// Find the client in the udp & tcp client lists in order to set it out of the game (not delete it yet)
	for( LI li=udpClients.begin(); !found && li!=udpClients.end(); li++)
	{
		un = (*li)->game_unit.GetUnit();
		if( un->GetSerial() == serial)
		{
			clt = (*li);
			found = true;
		}
	}
	if( !found)
	{
		for( LI li=tcpClients.begin(); !found && li!=tcpClients.end(); li++)
		{
			un = (*li)->game_unit.GetUnit();
			if( un->GetSerial() == serial)
			{
				clt = (*li);
				found = true;
			}
		}
	}
	if( !found)
		cout<<"Killed a non client Unit = "<<serial<<endl;
	else
	{
		cout<<"Killed client serial = "<<serial<<endl;
		clt->ingame = false;
	}

	p.bc_create( CMD_KILL, serial, NULL, 0, SENDRELIABLE, NULL, acct_sock, __FILE__,
#ifndef _WIN32
			__LINE__
#else
			1553
#endif
		);
	// WARNING : WE WILL SEND THE INFO BACK TO THE CLIENT THAT HAS FIRED
	zonemgr->broadcast( zone, serial, &p );
}

