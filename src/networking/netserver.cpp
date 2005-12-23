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
#include "networking/client.h"
#include "networking/lowlevel/packet.h"
#include "lin_time.h"
#include "networking/netserver.h"
#include "networking/lowlevel/vsnet_serversocket.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/savenet_util.h"
#include "vsfilesystem.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"
#include "cmd/ai/script.h"
#include "cmd/ai/order.h"
#include "cmd/ai/fire.h"
#include "cmd/ai/fireall.h"
#include "cmd/ai/flybywire.h"
#include "cmd/role_bitmask.h"
#include "gfxlib_struct.h"
#include "posh.h"
#include "fileutil.h"

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
string	universe_path;

using namespace VSFileSystem;

void	getZoneInfoBuffer( unsigned short zoneid, NetBuffer & netbuf)
{
	VSServer->getZoneInfo( zoneid, netbuf);
}

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
	this->snapchanged = 0;
	/***** number of zones should be determined as server loads zones files *****/
	zonemgr = new ZoneMgr();
	UpdateTime();
	srand( (unsigned int) getNewTime());
	// Here 500 could be something else between 1 and 0xFFFF
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
	globalsave = new SaveGame("");

    _downloadManagerServer.reset( new VsnetDownload::Server::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloadManagerServer );
#ifdef CRYPTO
	FileUtil::use_crypto = true;
#endif
}

NetServer::~NetServer()
{
	delete zonemgr;
	delete globalsave;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void	NetServer::startMsg()
{
	cout<<endl<<"Vegastrike Server version "<<GAMESERVER_VERSION<<endl;
	cout<<"Written by Stephane Vaxelaire"<<endl<<endl<<endl;
	cout<<POSH_GetArchString()<<endl;
}

/**************************************************************/
/**** Start the server loop                                ****/
/**************************************************************/

extern void InitUnitTables(); // universe_generic.cpp

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
	VSFileSystem::InitPaths( CONFIGFILE);
	InitUnitTables(); // universe_generic.cpp
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
	NETWORK_ATOM = (double) atof( strnetatom.c_str());
	strnetatom = vs_config->getVariable( "network", "damage_atom", "1");
	DAMAGE_ATOM = (double) atof( strnetatom.c_str());
	strnetatom = vs_config->getVariable( "network", "planet_atom", "10");
	PLANET_ATOM = (double) atof( strnetatom.c_str());

	InitTime();
	UpdateTime();
	savetime = getNewTime();
	reconnect_time = getNewTime()+periodrecon;

	string tmp;
	char srvip[256];
	unsigned short tmpport = ACCT_PORT;
	stracct = vs_config->getVariable( "server", "useaccountserver", "");
	acctserver = ( stracct=="true");

	// Create and bind sockets
	COUT << "Initializing TCP server ..." << endl;
	tcpNetwork = NetUITCP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "6777")).c_str()), _sock_set );
    if( tcpNetwork == NULL )
    {
        COUT << "Couldn't create TCP server - quitting" << endl;
        exit( -100 );
    }

	COUT << "Initializing UDP server ..." << endl;
	udpNetwork = NetUIUDP::createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "6777")).c_str()), _sock_set );
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
			VSExit(1);
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
			VSExit(1);
		}
		COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create the _Universe telling it we are on server side
	universe_path = "";
	universe_file = vs_config->getVariable ("server","galaxy","milky_way.xml");
	cout<<"LOADING Universe file : "<<universe_file<<endl;
	_Universe = new Universe( argc, argv, universe_file.c_str(), true);
	cout<<"Universe LOADED"<<endl;
	string strmission = vs_config->getVariable( "server", "missionfile", "networking.mission");
	active_missions.push_back( mission = new Mission( strmission.c_str()));
	mission->initMission( false);

	// Loads dynamic universe
	string dynpath = "dynaverse.dat";
	VSFile f;
	VSError err = f.OpenReadOnly( dynpath, ::VSFileSystem::UnknownFile);
	if( err>Ok)
	{
		cerr<<"!!! ERROR : opening dynamic universe file " << dynpath.c_str() << " !!!"<<endl;
	}
	else
	{
		string dynaverse = f.ReadFull();
		char * dynchar = strdup( dynaverse.c_str());
		globalsave->ReadSavedPackets( dynchar,true);
		f.Close();
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
			reconnect_time = curtime+periodrecon;
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
					VSExit(1);
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
			disconnect( (*j), __FILE__, PSEUDO__LINE__(328) );
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



// NETFIXME: Why was StarSystem->Update() commented out?




		
		  unsigned int i;
		  static float nonactivesystemtime = XMLSupport::parse_float (vs_config->getVariable ("physics","InactiveSystemTime",".3"));
		  static unsigned int numrunningsystems = XMLSupport::parse_int (vs_config->getVariable ("physics","NumRunningSystems","4"));
		  float systime=nonactivesystemtime;
		  
		  for (i=0;i<_Universe->star_system.size()&&i<numrunningsystems;i++) {


// NETFIXME: No Director for you!


			  _Universe->star_system[i]->Update((i==0)?1:systime/i,false);
		  }
		  StarSystem::ProcessPendingJumps();
		/****************************** VS STUFF TO DO ************************************/

		  if( snapchanged && (curtime - snaptime)>NETWORK_ATOM)
		{
			//COUT<<"SENDING SNAPSHOT ----------"<<end;
			// If planet time we send planet and nebula info
			if( (curtime - planettime)>PLANET_ATOM)
			{
				zonemgr->broadcastSnapshots( true);
				planettime = curtime;
			}
			// Otherwise we just send ships/bases... info
			else
				zonemgr->broadcastSnapshots( false);
			snapchanged = 0;
			snaptime = curtime;
		}
#ifndef NET_SHIELD_SYSTEM_1
		// Time to send shield and damage info
		  if( (curtime - damagetime)>DAMAGE_ATOM)
		{
			zonemgr->broadcastDamage();
			damagetime = curtime;
		}
#endif

		// Check for automatic server status save time (in seconds)
		//curtime = getNewTime();
		//if( curtime - savetime > period*60)
		  if( (curtime - savetime)>SAVE_ATOM)
		{
			// Not implemented
			cout<<">>> Saving server status... ";
			this->save();
			savetime = curtime;
			cout<<"done."<<endl;
		}


		_sock_set.waste_time(0,10000);
	}

	delete CONFIGFILE;
	delete vs_config;
        vs_config=NULL;
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
				VSExit(1);
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
/**** Check which clients are sending data to the server   ****/
/**************************************************************/

// NETFIXME: Completely separate code logic in debug and non-debug. put #ifdef's only around print statements.

void	NetServer::checkMsg( SocketSet& sets )
#ifdef VSNET_DEBUG
{
    ostringstream ostr;
    bool          printit = false;
    ostr << "Checking activity on sockets, TCP=";
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
	    if( cl->sock.isActive( ) )
	    {
            ostr << cl->sock.get_fd() << "+ ";
            printit = true;
			this->recvMsg_tcp( cl );
		}
	}
    ostr << " ";
	if( udpNetwork->isActive( ) )
	{
        ostr << "UDP=" << udpNetwork->get_fd() << "+" << ends;
	    recvMsg_udp( );
        printit = true;
	}
    if( tcpNetwork->isActive( ) )
    {
		newConnection_tcp( );
    }
    ostr << ends;
    if( printit ) COUT << ostr.str() << endl;
}
#else
{
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
		if( cl->tcp_sock.isActive( ) )
		{
		    this->recvMsg_tcp( cl );
		}
	}
	if( udpNetwork.isActive( ) )
	{
	    recvMsg_udp( );
	}
    if( tcpNetwork->isActive( ) )
    {
		newConnection_tcp( );
    }
}
#endif

// Return true if ok, false if we received a late packet
bool	NetServer::updateTimestamps( ClientPtr cltp, Packet & p )
{
        assert( cltp );
        Client* clt = cltp.get();

		bool 	  ret = true;
		// A packet's timestamp is in ms whereas getNewTime is in seconds
		unsigned int int_ts = p.getTimestamp();

		cerr<<"GOT TIMESTAMP="<<int_ts<<" latest is="<<clt->getLatestTimestamp() << " in " << p.getCommand() << endl;
		double curtime = getNewTime();
		// Check for late packet : compare received timestamp to the latest we have
//		assert( int_ts >= clt->getLatestTimestamp());
		if( int_ts < clt->getLatestTimestamp() )
		{
			// If ts > 0xFFFFFFF0 (15 seconds before the maxin an u_int) 
			// This is not really a reliable test -> we may still have late packet in that range of timestamps
			// Only check for late packets when sent non reliable because we need others
//			if(p.getFlags() & SENDANDFORGET)
//			{
			ret = !( p.getCommand()==CMD_SNAPSHOT || p.getCommand()==CMD_POSUPDATE || p.getCommand()==CMD_PING); //only invalidates updates if its a snapshot or posupdate--same reason it updates the timestamps to begin with
//			}
				/*
			else if( clt->isTcp())
			{
				COUT << "!!!ERROR : Late packet in TCP mode : this should not happen !!!" << endl
                     << "   Previous client timestamp: " << clt->getLatestTimestamp() << "ms" << endl
                     << "   Current  client timtstamp: " << int_ts << "ms" << endl;
//				VSExit(1);
			}
				*/
		}
		// If packet is late we don't update time vars but we process it if we have to
		else
		{
			// Update the timeout vals anytime we receive a packet
			// Set old_timeout to the old_latest one and latest_timeout to current time in seconds
			clt->old_timeout = clt->latest_timeout;
			clt->latest_timeout = curtime;

			// Packet is not late so we update timestamps only when receving a CMD_SNAPSHOT
			// because we predict and interpolate based on the elapsed time between 2 SNAPSHOTS or PING
			if( p.getCommand()==CMD_SNAPSHOT || p.getCommand()==CMD_POSUPDATE || p.getCommand()==CMD_PING)
			{
				// Set old_timestamp to the old latest_timestamp and the latest_timestamp to the received one
				clt->setLatestTimestamp( int_ts );
			}
		}

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
	  			char flags = netbuf.getChar();
				SOCKETALT tmpsock;
				const AddressIP* iptmp;
				WaitListEntry entry;
				if( clt )
				{
					// This must be a TCP client
					entry.tcp = true;
					entry.t   = clt;
					this->waitList.push( entry );
					iptmp = &clt->cltadr;
					tmpsock = clt->tcp_sock;
				}
				else
				{
					entry.tcp = false;
					entry.u   = ipadr;
					this->waitList.push( entry );
					iptmp = &ipadr;
					COUT << "Waiting authorization for client IP : " << ipadr << endl;
				}

				// Redirect the login request packet to account server
				COUT << "Redirecting login request to account server on socket " << acct_sock << endl
				<< "*** Packet to copy length : " << packet.getDataLength()<<endl;
				if( p2.send( packet.getCommand(), 0, (char *)packet.getData(), packet.getDataLength(), SENDRELIABLE, iptmp, acct_sock, __FILE__, PSEUDO__LINE__(1031) ) < 0 )
				{
					perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
					COUT<<"SOCKET was : "<<acct_sock<<endl;
					VSExit(1);
				}
            }
            COUT<<"<<< LOGIN REQUEST --------------------------------------"<<endl;
        }
        break;
		case CMD_ADDCLIENT:
			// Add the client to the game
			COUT<<">>> ADD REQUEST =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
			//COUT<<"Received ADDCLIENT request"<<endl;
			this->addClient( clt);
			COUT<<"<<< ADD REQUEST --------------------------------------------------------------"<<endl;
			break;
		case CMD_POSUPDATE:
			// Received a position update from a client
//			cerr<<">>> POSITION UPDATE =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
			this->posUpdate( clt );
//			cerr<<"<<< POSITION UPDATE ---------------------------------------------------------------"<<endl;
			break;
		case CMD_LETSGO:
			// Add the client to the game in its zone
			//this->addClient( clt );
			break;
		case CMD_PING:
			// Nothing to do here, just receiving the packet is enough
			//COUT<<"Got PING from serial "<<packet.getSerial()<<endl;
			break;
		case CMD_SERVERTIME:
			
			serverTimeInitUDP( clt, netbuf );
		{
		}
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
                _downloadManagerServer->addCmdDownload( clt->tcp_sock, netbuf );
            }
			COUT<<"<<< CMD DOWNLOAD --------------------------------------------------------------"<<endl;
            break;
		case CMD_FIREREQUEST :
			// Here should put a flag on the concerned mount of the concerned Unit to say we want to fire
			// target_serial is in fact the serial of the firing unit (client itself or turret)
			target_serial = netbuf.getSerial();
			mount_num = netbuf.getInt32();
			zone = clt->game_unit.GetUnit()->activeStarSystem->GetZone();
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
				if (mount_num<un->mounts.size()&&mount_num>=0) {
					un->mounts[mount_num].status=Mount::ACTIVE;
				} else {
					COUT<<"ERROR --> Received a fire order on an invalid MOUNT: "<<mount_num<<" > "<<(un->mounts.size())<<endl;
				}
				// Ask for fire
				if( mis != 0)
					un->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
				else
					un->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
			}
		break;
		case CMD_UNFIREREQUEST :
			target_serial = netbuf.getSerial();
			mount_num = netbuf.getInt32();
			zone = clt->game_unit.GetUnit()->activeStarSystem->GetZone();
			// Find the unit
			// Set the concerned mount as ACTIVE and others as INACTIVE
			un = zonemgr->getUnit( target_serial, zone);
			if( un==NULL)
				COUT<<"ERROR --> Received an unfire order for non-existing UNIT"<<endl;
			else
			{
				vector <Mount>
					::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
				for (;i!=un->mounts.end();++i)
					(*i).status=Mount::INACTIVE;
				if (mount_num<un->mounts.size()&&mount_num>=0) {
					un->mounts[mount_num].status=Mount::ACTIVE;
				} else {
					COUT<<"ERROR --> Received an unfire order on an invalid MOUNT: "<<mount_num<<" > "<<(un->mounts.size())<<endl;
				}
				// Ask for fire
				un->UnFire();
			}
		break;
		case CMD_JUMP :
		{
			vector<string>	adjacent;
			string newsystem = netbuf.getString();
			ObjSerial jumpserial = netbuf.getSerial();
			unsigned short zonenum = netbuf.getShort();
			unsigned char * client_hash=0;
			unsigned char * server_hash;
#ifdef CRYPTO
			server_hash = new unsigned char[FileUtil::Hash.DigestSize()];
			client_hash = netbuf.getBuffer( FileUtil::Hash.DigestSize());
#endif
			bool found = false;
			NetBuffer	netbuf2;

			StarSystem * sts;
			Cockpit * cp;
			
			un = clt->game_unit.GetUnit();
			if( un==NULL)
				COUT<<"ERROR --> Received a jump request from non-existing UNIT"<<endl;
			else
			{
					cp = _Universe->isPlayerStarship( un);
					// Verify if there really is a jump point to the new starsystem
					adjacent = _Universe->getAdjacentStarSystems( cp->savegame->GetStarSystem()+".system");
					for( unsigned int i=0; !found && i<adjacent.size(); i++)
					{
						if( adjacent[i]==newsystem)
							found = true;
					}
					if( found)
					{
						Unit * jumpun = zonemgr->getUnit( jumpserial, zonenum);
						// Then activate jump drive to say we want to jump
						un->ActivateJumpDrive();
						// The jump reply is sent in Unit::jumpReactToCollision()
						// In the meantime we create the star system if it isn't loaded yet
						// The starsystem maybe loaded for nothing if the client has not enough warp energy to jump
						// but that's no big deal since they all will be loaded finally
						if( !(sts = _Universe->getStarSystem( newsystem+".system")))
							zonemgr->addZone( newsystem);

						clt->jumpfile = newsystem;
						if( FileUtil::HashCompare( newsystem, client_hash, SystemFile) )
							clt->jumpok = 1;
						else
							clt->jumpok = 2;
					}
			}	
#ifdef CRYPTO
			delete server_hash;
#endif
		}
		break;
		case CMD_SCAN :
		{
			// Received a target scan request
			// NETFIXME: WE SHOULD FIND A WAY TO CHECK THAT THE CLIENT HAS THE RIGHT SCAN SYSTEM FOR THAT
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
		// NETFIXME: SHOULD NOT RECEIVE CMD_CAMSHOT SINCE COMM SESSIONS ARE HANDLED IN A CLIENT-TO-CLIENT WAY
		/*
		case CMD_CAMSHOT :
		{
			p2.bc_create( packet.getCommand(), packet.getSerial(),
                          packet.getData(), packet.getDataLength(), SENDANDFORGET,
                          __FILE__, PSEUDO__LINE__(1281));
			// Send to concerned clients
			zonemgr->broadcast_camshots( clt->game_unit.GetUnit()->activeStarSystem->GetZone(), clt->serial, &p2);
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
			p2.bc_create( packet.getCommand(), packet_serial,
                          packet.getData(), packet.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1293));
			// Send to concerned clients
			zonemgr->broadcast( clt->game_unit.GetUnit()->activeStarSystem->GetZone(), packet_serial, &p2, true);
		}
		break;
		case CMD_STOPNETCOMM :
		{
			clt->comm_freq = -1;
			// float freq = netbuf.getFloat();
			// Broadcast players with same frequency that this client is leaving the comm session
			p2.bc_create( packet.getCommand(), packet_serial,
                          packet.getData(), packet.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1302));
			// Send to concerned clients
			zonemgr->broadcast( clt->game_unit.GetUnit()->activeStarSystem->GetZone(), packet_serial, &p2, true);
		}
		break;
		case CMD_SOUNDSAMPLE :
		{
			// Broadcast sound sample to the clients in the same zone and the have PortAudio support
			p2.bc_create( packet.getCommand(), packet_serial,
                          packet.getData(), packet.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1341));
			zonemgr->broadcastSample( clt->game_unit.GetUnit()->activeStarSystem->GetZone(), packet_serial, &p2, clt->comm_freq);

		}
		case CMD_TXTMESSAGE :
		{
			// Broadcast sound sample to the clients in the same zone and the have PortAudio support
			p2.bc_create( packet.getCommand(), packet_serial,
                          packet.getData(), packet.getDataLength(), SENDRELIABLE,
                          __FILE__, PSEUDO__LINE__(1341));
			zonemgr->broadcastText( clt->game_unit.GetUnit()->activeStarSystem->GetZone(), packet_serial, &p2, clt->comm_freq);

		}
		case CMD_DOCK :
		{
			Unit * docking_unit;
			un = clt->game_unit.GetUnit();
			ObjSerial utdwserial = netbuf.getShort();
			unsigned short zonenum = clt->game_unit.GetUnit()->activeStarSystem->GetZone();
			cerr<<"RECEIVED a DockRequest from unit "<<un->GetSerial()<<" to unit "<<utdwserial<<" in zone "<<zonenum<<endl;
			docking_unit = zonemgr->getUnit( utdwserial, zonenum);
			if( docking_unit)
			{
				// In Unit::ForceDocking() we increase dockingport bye one because it may be 0
				int dockport = un->Dock( docking_unit) - 1;
				if( dockport)
					this->sendDockAuthorize( un->GetSerial(), utdwserial, dockport, zonenum);
			}
			else
				cerr<<"!!! ERROR : cannot dock with unit serial="<<utdwserial<<endl;
		}
		break;
		case CMD_UNDOCK :
		{
			Unit * docking_unit;
			un = clt->game_unit.GetUnit();
			ObjSerial utdwserial = netbuf.getShort();
			unsigned short zonenum = clt->game_unit.GetUnit()->activeStarSystem->GetZone();
			cerr<<"RECEIVED an UnDockRequest from unit "<<un->GetSerial()<<" to unit "<<utdwserial<<" in zone "<<zonenum<<endl;
			docking_unit = zonemgr->getUnit( utdwserial, zonenum);
			if( docking_unit)
			{
				int dockport = un->UnDock( docking_unit);
				if( dockport)
					this->sendUnDock( un->GetSerial(), utdwserial, zonenum);
			}
			else
				cerr<<"!!! ERROR : cannot dock with unit serial="<<utdwserial<<endl;
		}
		break;
    	default:
        	COUT << "Unknown command " << Cmd(cmd) << " ! "
             	 << "from client " << clt->game_unit.GetUnit()->GetSerial() << endl;
    }
}

/**************************************************************/
/**** Broadcast a netbuffer to a given zone                ****/
/**************************************************************/

void	NetServer::broadcast( NetBuffer & netbuf, unsigned short zone, Cmd command, bool isTcp )
{
	Packet p;
	p.bc_create( command, 0,
                 netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE,
                 __FILE__, PSEUDO__LINE__(902));
	zonemgr->broadcast( zone, 0, &p, isTcp );
}

/**************************************************************/
/**** Close all sockets for shutdown                       ****/
/**************************************************************/

void	NetServer::closeAllSockets()
{
	tcpNetwork->disconnect( "Closing sockets", false );
	udpNetwork.disconnect( "Closing sockets", false );
	for( LI i=allClients.begin(); i!=allClients.end(); i++)
	{
        ClientPtr cl = *i;
		cl->tcp_sock.disconnect( __PRETTY_FUNCTION__, false );
	}
}

void	NetServer::addSystem( string & sysname, string & system)
{
	zonemgr->addSystem( sysname, system);
}

