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

VegaConfig * vs_config;
double	clienttimeout;
double	logintimeout;
double	NETWORK_ATOM;
int		acct_con;
string	tmpdir;

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
}

NetServer::~NetServer()
{
	delete zonemgr;
}

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

ObjSerial	NetServer::getUniqueSerial()
{
	// MAYBE CHANGE TO SOMETHING MORE "RANDOM"
	serial_seed = (serial_seed+3)%MAXSERIAL;
	return serial_seed;
}

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

void	NetServer::authenticate( Client * clt, AddressIP ipadr, Packet& packet )
{
	Packet	packet2;
	char *	name = new char[NAMELEN+1];
	char *	passwd = new char[NAMELEN+1];
	int		i, lisize;
	Account *	elem = NULL;

	lisize = sizeof( unsigned long) + sizeof( int);
	//buflen = NAMELEN*2 + lisize;
	const char * buf = packet.getData();
	strcpy( name, buf);
	strcpy( passwd, buf+NAMELEN);

	i=0;
	int found=0;
	for ( VI j=Cltacct.begin(); j!=Cltacct.end() && !found; j++, i++)
	{
		elem = *j;
		if( !elem->compareName( name) && !elem->comparePass( passwd))
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

	delete name;
	delete passwd;
}

void	NetServer::sendLoginAccept( Client * clt, AddressIP ipadr, int newacct)
{
    COUT << "enter " << __PRETTY_FUNCTION__ << endl;

    char name[NAMELEN+1];
    char passwd[NAMELEN+1];
    char * buf = packeta.getData();
    strcpy( name, buf);
    strcpy( passwd, buf+NAMELEN);

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
	strcpy( clt->name, buf);
	strcpy( clt->passwd, buf+NAMELEN);

	// Assign its serial to client*
	if( !acctserver)
		clt->serial = getUniqueSerial();
	else
		clt->serial = packeta.getSerial();
	clt->current_state.setSerial( clt->serial);
	//cout<<"Authentication success for serial "<<clt->serial<<endl;
	COUT << "LOGIN REQUEST SUCCESS for <" << name << ">" << endl;
	// Verify that client already has a character
	if( newacct)
	{
		COUT << "This account has no ship/char so create one" << endl;
		// Send a command to make the client create a new character/ship
	}
	else
	{
		COUT << ">>> SEND LOGIN ACCEPT =( serial n°" << clt->serial << " )= --------------------------------------" << endl;
		//cout<<"Login recv packet size = "<<packeta.getLength()<<endl;
		// Get the save parts in the buffer
		vector<string> saves;
		saves = FileUtil::GetSaveFromBuffer( packeta.getData()+2*NAMELEN);
		char * savebuf = new char[saves[1].length()+1];
		memcpy( savebuf, saves[1].c_str(), saves[1].length());
		savebuf[saves[1].length()] = 0;
		char * xmlbuf = new char[saves[0].length()+1];
		memcpy( xmlbuf, saves[0].c_str(), saves[0].length());
		xmlbuf[saves[0].length()] = 0;
		cout<<"XML="<<saves[0].length()<<" bytes - SAVE="<<saves[1].length()<<" bytes"<<endl;
		string PLAYER_CALLSIGN( clt->name);
		QVector tmpvec( 0, 0, 0), safevec;
		bool update = true;
		float credits;
		vector<string> savedships;
		string str("");
		// Create a cockpit for the player and parse its savegame
		Cockpit * cp = _Universe->createCockpit( PLAYER_CALLSIGN);
		cp->Init ("");
		cout<<"-> LOADING SAVE FROM NETWORK"<<endl;
		cp->savegame->ParseSaveGame( "", str, "", tmpvec, update, credits, savedships, clt->serial, savebuf, false);
		// Generate the system we enter in if needed
		zonemgr->addZone( cp->savegame->GetStarSystem());
		safevec = UniverseUtil::SafeEntrancePoint( tmpvec);
		cout<<"\tcredits = "<<credits<<endl;
		cout<<"\tcredits = "<<credits<<endl;
		cout<<"\tposition : x="<<safevec.i<<" y="<<safevec.j<<" z="<<safevec.k<<endl;
		cout<<"-> SAVE LOADED"<<endl;

		// WARNING : WE DON'T SAVE FACTION NOR FLIGHTGROUP YET
		cout<<"-> UNIT FACTORY WITH XML"<<endl;
		// We may have to determine which is the current ship of the player if we handle several ships for one player
		string PLAYER_SHIPNAME = savedships[0];
		// WE DON'T KNOW THE FACTION YET !!! SO I MAKE IT DEFAULT TO "privateer"
		string PLAYER_FACTION_STRING( "privateer");
		Unit * un = UnitFactory::createUnit( PLAYER_CALLSIGN.c_str(),
                             false,
                             FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
                             string(""),
                             Flightgroup::newFlightgroup (PLAYER_CALLSIGN,PLAYER_SHIPNAME,PLAYER_FACTION_STRING,"default",1,1,"","",mission),
                             0, xmlbuf);
		cout<<"\tAFTER UNIT FACTORY WITH XML"<<endl;
		clt->game_unit.SetUnit( un);
		// Setup the clientstates
		clt->old_state.setPosition( cp->savegame->GetPlayerLocation());
		clt->old_state.setSerial( clt->serial);
		clt->current_state.setPosition( cp->savegame->GetPlayerLocation());
		clt->current_state.setSerial( clt->serial);
		// Affect the created unit to the cockpit
		cout<<"-> UNIT LOADED"<<endl;
		cp->SetParent( un,"","",safevec);
		cout<<"-> COCKPIT AFFECTED TO UNIT"<<endl;
		// The Unit will be added in the addClient function

		//string strname( name);
		// Write temp XML file for unit
		//string tmp;
		//tmp = tmpdir+name+".xml";
		//WriteXMLUnit( tmp.c_str(), xml, xml_size);
		// Then load it in the Unit struct
		//LoadXMLUnit( clt->game_unit, tmp.c_str(), NULL);

        Packet packet2;
		packet2.send( LOGIN_ACCEPT, clt->serial, packeta.getData(), packeta.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
		delete savebuf, xmlbuf;
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
	packet2.send( LOGIN_ERROR, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, __LINE__ );
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
	packet2.send( LOGIN_UNAVAIL, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, __LINE__ );
	cout<<"<<< SENT LOGIN UNAVAILABLE -----------------------------------------------------------------------"<<endl;
}

void	NetServer::sendLoginAlready( Client * clt, AddressIP ipadr)
{
	Packet	packet2;
	char name[NAMELEN+1];
	char passwd[NAMELEN+1];
	const char * buf = packet.getData();
	strcpy( name, buf);
	strcpy( passwd, buf+NAMELEN);
	// Send a login error
	// int		retsend;
	SOCKETALT	sockclt;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	cout<<">>> SEND LOGIN ALREADY =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
	packet2.send( LOGIN_ALREADY, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, __LINE__ );
	cout<<"<<< SENT LOGIN ALREADY -----------------------------------------------------------------------"<<endl;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void	NetServer::startMsg()
{
	cout<<endl<<"Vegastrike Server version 0.0.1"<<endl;
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
	acct_con = 1;
	Packet p2;

	startMsg();

	CONFIGFILE = new char[42];
	strcpy( CONFIGFILE, "vegaserver.config");
	cout<<"Loading server config...";
	initpaths();
	//vs_config = new VegaConfig( SERVERCONFIGFILE);
	cout<<" config loaded"<<endl;
	strperiod = vs_config->getVariable( "server", "saveperiod", "");
	if( strperiod=="")
		period = 7200;
	else
		period = atoi( strperiod.c_str());
	string strperiodrecon = vs_config->getVariable( "server", "reconnectperiod", "");
	if( strperiodrecon=="")
		periodrecon = 60;
	else
		periodrecon = atoi( strperiodrecon.c_str());
	tmpdir = datadir + vs_config->getVariable( "server", "tmpdir", "");
	if( strperiod=="")
		tmpdir = datadir + "/tmp/";
	strtimeout = vs_config->getVariable( "server", "clienttimeout", "");
	if( strtimeout=="")
		clienttimeout = 20;
	else
		clienttimeout = atoi( strtimeout.c_str());
	strlogintimeout = vs_config->getVariable( "server", "logintimeout", "");
	if( strlogintimeout=="")
		logintimeout = 60;
	else
		logintimeout = atoi( strlogintimeout.c_str());
	strnetatom = vs_config->getVariable( "network", "network_atom", "");
	if( strnetatom=="")
		NETWORK_ATOM = 0.2;
	else
		NETWORK_ATOM = (double) atoi( strnetatom.c_str());
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
		COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create and bind sockets
	COUT << "Initializing TCP server ..." << endl;
	tcpNetwork = NetUITCP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()) );
	COUT << "Initializing UDP server ..." << endl;
	udpNetwork = NetUIUDP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()) );
	COUT << "done." << endl;

	// Create the _Universe telling it we are on server side
	_Universe = new Universe(argc,argv,vs_config->getVariable ("general","galaxy","milky_way.xml").c_str(), true);

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
				ObjSerial sertmp;
				// Put first the number of clients
				memcpy( buflist, &nbclients, sizeof( unsigned short));
				for( j=0, i = tcpClients.begin(); i!=tcpClients.end(); i++, j++)
				{
					// Add the current client's serial to the buffer
					sertmp = htons( (*i)->serial);
					memcpy( buflist+j*sizeof( ObjSerial), &sertmp, sizeof( ObjSerial));
				}
				for( i = udpClients.begin(); i!=udpClients.end(); i++, j++)
				{
					// Add the current client's serial to the buffer
					sertmp = htons( (*i)->serial);
					memcpy( buflist+j*sizeof( ObjSerial), &sertmp, sizeof( ObjSerial));
				}
				// Passing NULL to AddressIP arg because between servers -> only TCP
				// Use the serial packet's field to send the number of clients
				if( p2.send( CMD_RESYNCACCOUNTS, nbclients_here, buflist, listlen, SENDRELIABLE, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
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
			this->disconnect( (*j), __FILE__, __LINE__ );
		}
		discList.clear();

		// Remove all clients that logged out
		for ( j=logoutList.begin(); j!=logoutList.end(); j++)
		{
			this->logout( (*j));
		}
		logoutList.clear();

		snaptime += GetElapsedTime();
		if( snapchanged && snaptime>NETWORK_ATOM)
		{
			//cout<<"SENDING SNAPSHOT ----------"<<end;
			zonemgr->broadcastSnapshots( ); // &NetworkToClient );
			snapchanged = 0;
		}

		// Check for automatic server status save time (in seconds)
		//curtime = getNewTime();
		if( curtime - savetime > period)
		{
			// Not implemented
			cout<<">>> Saving server status... ";
			this->save();
			savetime += period;
			cout<<"done."<<endl;
		}

		/****************************** VS STUFF TO DO ************************************/
		// UPDATE STAR SYSTEM -> TO INTEGRATE WITH NETWORKING
		// PROCESS JUMPS -> MAKE UNITS CHANGE THEIR STAR SYSTEM
		  //unsigned int i;
		  //StarSystem * lastStarSystem = NULL;
		  //static float nonactivesystemtime = XMLSupport::parse_float (vs_config->getVariable ("physics","InactiveSystemTime",".3"));
		  //static unsigned int numrunningsystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
		  //float systime=nonactivesystemtime;
		  
		  /*
			for (i=0;i<star_system.size()&&i<numrunningsystems;i++) {
			star_system[i]->Update((i==0)?1:systime/i,true);
		  }
		  StarSystem::ProcessPendingJumps();
		  */
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


/* Remake with SDL as well */

void	NetServer::checkKey()
{
/*
#ifndef HAVE_SDL
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
#endif
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
	for (LI i=udpClients.begin(); i!=udpClients.end(); i++)
	{
		deltatmp = (fabs(curtime - (*i)->latest_timeout));
		if( (*i)->latest_timeout!=0)
		{
			//cout<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			// Here considering a delta > 0xFFFFFFFF*X where X should be at least something like 0.9
			// This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			// side -> when timestamp has grown enough to became bigger than what an u_int can store
			if( (*i)->zone>-1 && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.8) )
			{
				cout<<"ACTIVITY TIMEOUT for client number "<<(*i)->serial<<endl;
				cout<<"\t\tCurrent time : "<<curtime<<endl;
				cout<<"\t\tLatest timeout : "<<((*i)->latest_timeout)<<endl;
				cout<<"t\tDifference : "<<deltatmp<<endl;
				discList.push_back( *i);
			}
			else if( !(*i)->zone>-1 && deltatmp > logintimeout)
			{
				cout<<"LOGIN TIMEOUT for client number "<<(*i)->serial<<endl;
				cout<<"\t\tCurrent time : "<<curtime<<endl;
				cout<<"\t\tLatest timeout : "<<((*i)->latest_timeout)<<endl;
				cout<<"t\tDifference : "<<deltatmp<<endl;
				discList.push_back( *i);
			}
			/*
			else
				cout<<"CLIENT "<<(*i)->serial<<" - DELTA="<<deltatmp<<endl;
			*/
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
            if( tmp->serial == nserial)
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

            COUT << "Error : IP changed for client # " << clt->serial << endl;
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
				if( p2.send( packet.getCommand(), 0, (char *)packet.getData(), packet.getDataLength(), SENDANDFORGET, iptmp, acct_sock, __FILE__, __LINE__ ) < 0 )
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
    default:
        COUT << "Unknown command " << Cmd(cmd) << " ! "
             << "from client " << clt->serial << endl;
    }
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::addClient( Client * clt)
{
	cout<<">>> SEND ENTERCLIENT =( serial n°"<<clt->serial<<" )= --------------------------------------"<<endl;
	Packet packet2;
	string savestr, xmlstr;

	if( zonemgr->addClient( clt))
	{
		// If the system is loaded, there are people in it -> BROADCAST
		ClientState	tmpcs;
		memcpy( &tmpcs, packet.getData(), sizeof( ClientState));
		tmpcs.received();
		clt->old_state = tmpcs;
		clt->current_state = tmpcs;
		//memcpy( &clt->old_state, &tmpcs, sizeof(ClientState));
		//memcpy( &clt->current_state, &tmpcs, sizeof(ClientState));
		tmpcs.tosend();
		// Here the other client in the same zone should be warned of a new client
		// Should also send data about the ship !!! filename ? IDs ?
		// maybe those thing should be managed in account.xml
		// For now assuming a default ship on client side

		// Send savebuffer after clientstate
		SaveNetUtil::GetSaveStrings( clt, savestr, xmlstr);
		unsigned int buflen = sizeof(ClientState)+2*sizeof(unsigned int)+savestr.length()+xmlstr.length();
		char * savebuf = new char[buflen];
		memcpy( savebuf, &tmpcs, sizeof( ClientState));
		// Put the save buffer after the ClientState
		SaveNetUtil::GetSaveBuffer( savestr, xmlstr, savebuf+sizeof( ClientState));
		packet2.bc_create( CMD_ENTERCLIENT, clt->serial, savebuf, buflen, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
		delete savebuf;
		cout<<"<<< SEND ENTERCLIENT -----------------------------------------------------------------------"<<endl;
		zonemgr->broadcast( clt, &packet2 ); // , &NetworkToClient );
		cout<<">>> SEND ADDED YOU =( serial n°"<<clt->serial<<" )= --------------------------------------"<<endl;
		cout<<"Serial : "<<clt->serial<<endl;
		// Send info about other ships in the system to "clt"
		zonemgr->sendZoneClients( clt);
	}

	// char * cltsbuf = new char[MAXBUFFER];
	// int cltsbufsize;
	// Send an accepted entering command and current zone's clients infos
	// So the packet buffer should contain info about other ships (desciptions) present in the zone
	// cltsbufsize = zonemgr->getZoneClients( clt, cltsbuf);
	//packet2.send( CMD_ADDEDYOU, clt->serial, cltsbuf, cltsbufsize, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
	cout<<"ADDED client n "<<clt->serial<<" in ZONE "<<clt->zone<<endl;
	//delete cltsbuf;
	//cout<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
}

/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/

void	NetServer::posUpdate( Client * clt)
{
	//Packet	pckt;
	//pckt.create( CMD_UPDATECLT, clt->serial, (char*) packet.getData(), sizeof( ClientState));
	//zonemgr->broadcast( clt, pckt );

	// Set old position
	//memcpy( &clt->old_state, &clt->current_state, sizeof( ClientState));
	clt->old_state = clt->current_state;
	// Update client position in client list
	//memcpy( &clt->current_state, packet.getData(), sizeof( ClientState));
	clt->current_state = *((ClientState *) packet.getData());
	// Put deltatime in the delay part of ClientState so that it is send to other clients later
	clt->current_state.received();
	Cockpit * cp = _Universe->isPlayerStarship( clt->game_unit.GetUnit());
	cp->savegame->SetPlayerLocation( clt->current_state.getPosition());
	clt->current_state.setDelay( clt->deltatime);
	//clt->current_state.display();
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

	if( acctserver)
	{
		// Send a disconnection info to account server
		char * buf = new char[NAMELEN*2+1];
		int nbc = strlen( clt->name);
		memcpy( buf, clt->name, nbc);
		memcpy( buf+NAMELEN, clt->passwd, NAMELEN);
		buf[nbc] = 0;
		Packet p2;
		if( p2.send( CMD_LOGOUT, clt->serial, buf, NAMELEN+NAMELEN,
		             SENDANDFORGET, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
        {
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		}
		//p2.display( "", 0);
		delete buf;
	}

	// Removes the client from its starsystem
	if( clt->zone>0)
		zonemgr->removeClient( clt);
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
	    COUT << "Client " << clt->serial << " disconnected" << endl;
	    COUT << "There were "
	         << tcpClients.size()+udpClients.size() << " clients - ";
	    tcpClients.remove( clt);
	}
	else
	{
		Packet p1;
	    p1.send( CMD_DISCONNECT, clt->serial, NULL, 0,
		         SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
	    COUT << "Client " << clt->serial << " disconnected" << endl;
	    COUT << "There were "
	         << tcpClients.size()+udpClients.size() << " clients - ";
	    udpClients.remove( clt);
	}
	// Broadcast client EXIT zone
	Packet p;
	p.bc_create( CMD_EXITCLIENT, clt->serial, NULL, 0,
	             SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
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

	if( acctserver)
	{
		// Send a disconnection info to account server
		char * buf = new char[NAMELEN*2+1];
		int nbc = strlen( clt->name);
		memcpy( buf, clt->name, nbc+1);
		memcpy( buf+NAMELEN, clt->passwd, strlen( clt->passwd));
		buf[NAMELEN*2] = 0;
		if( p2.send( CMD_LOGOUT, clt->serial, buf, NAMELEN*2, SENDANDFORGET, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		//p2.display( "", 0);
		delete buf;
	}

	// Removes the client from its starsystem
	if( clt->zone>0)
		zonemgr->removeClient( clt);
    if( clt->isTcp() )
	{
		clt->sock.disconnect( __PRETTY_FUNCTION__, false );
	    COUT <<"Client "<<clt->serial<<" disconnected"<<endl;
	    COUT <<"There was "<< udpClients.size()+tcpClients.size() <<" clients - ";
	    tcpClients.remove( clt );
	}
	else
	{
	    COUT <<"Client "<<clt->serial<<" disconnected"<<endl;
	    COUT <<"There was "<< udpClients.size()+tcpClients.size() <<" clients - ";
	    udpClients.remove( clt );
	}
	// Broadcast client EXIT zone
	if( clt->zone>0)
	{
		// p.create( CMD_EXITCLIENT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->serial);
		p.bc_create( CMD_EXITCLIENT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
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
	char * buffer;
	// Loop through all cockpits and write save files
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		SaveNetUtil::GetSaveStrings( i, savestr, xmlstr);
		// Write the save and xml unit
		FileUtil::WriteSaveFiles( savestr, xmlstr, "./serversaves", cp->savegame->GetCallsign());
		// SEND THE BUFFERS TO ACCOUNT SERVER
		if( acctserver && acct_con)
		{
			bool found = false;
			Client * clt;
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
			buffer = new char[savestr.length() + xmlstr.length() + 2*sizeof( unsigned int)];
			SaveNetUtil::GetSaveBuffer( savestr, xmlstr, buffer);
			if( pckt.send( CMD_SAVEACCOUNTS, clt->serial, buffer, strlen( buffer), SENDRELIABLE, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
				cout<<"ERROR sending SAVE to account server"<<endl;
			delete buffer;
		}
	}
}
