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
/*
#ifndef HAVE_SDL
	#include <sys/select.h>
#endif
*/

#include "client.h"
#include "packet.h"
#include "lin_time.h"
#include "netserver.h"

VegaConfig * vs_config;
//VegaConfig * vs_config;
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
	Network = new NetUI();
	this->nbclients = 0;
	this->nbaccts = 0;
	this->keeprun = 1;
	this->acctserver = 0;
	this->srvtimeout.tv_sec = 0;
	this->srvtimeout.tv_usec = 0;
	/***** number of zones should be determined as server loads zones files *****/
	zonemgr = new ZoneMgr( 10);
	// Here 500 could be something else between 1 and 0xFFFF
	UpdateTime();
	srand( (unsigned int) getNewTime());
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
}

NetServer::~NetServer()
{
	if( Network!=NULL)
		delete Network;
	if( NetAcct!=NULL)
		delete NetAcct;
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

void	NetServer::authenticate( Client * clt, AddressIP ipadr)
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
	Packet	packet2;
	char name[NAMELEN+1];
	char passwd[NAMELEN+1];
	const char * buf = packet.getData();
	strcpy( name, buf);
	strcpy( passwd, buf+NAMELEN);
#ifdef _UDP_PROTO
	// In UDP mode, client is created here
	clt = this->newConnection( &ipadr);
	if( !clt)
	{
		cout<<"Error creating new client connection"<<endl;
		exit(1);
	}
#endif
	memcpy( &clt->cltadr, &ipadr, sizeof( AddressIP));
	strcpy( clt->name, buf);

	// Assign its serial to client*
	if( !acctserver)
		clt->serial = getUniqueSerial();
	else
		clt->serial = packeta.getSerial();
	clt->current_state.setSerial( clt->serial);
	//cout<<"Authentication success for serial "<<clt->serial<<endl;
	cout<<"\tLOGIN REQUEST SUCCESS for <"<<name<<">"<<endl;
	// Verify that client already has a character
	if( newacct)
	{
		cout<<"This account has no ship/char so create one"<<endl;
		// Send a command to make the client create a new character/ship
	}
	else
	{
		// HERE SHOULD LOAD XML Unit desciription from the xml save in the packet
		char * xml = packet.getData() + NAMELEN*2;
		int xml_size = packet.getLength() - Packet::getHeaderLength() - NAMELEN*2;
		string strname( name);
		// Write temp XML file for unit
		string tmp;
		tmp = tmpdir+name+".xml";
		//WriteXMLUnit( tmp.c_str(), xml, xml_size);
		// Then load it in the Unit struct
		//LoadXMLUnit( clt->game_unit, tmp.c_str(), NULL);
		packet2.create( LOGIN_ACCEPT, clt->serial, xml, xml_size, 1);
		//cout<<" 1st packet -------------"<<endl;
		//packet2.displayHex();
		cout<<"Packet length = "<<packet2.getLength()<<endl;
		packet2.tosend();
		if( Network->sendbuf( clt->sock, (char *) &packet2, packet2.getSendLength(), &clt->cltadr) == -1)
		{
			cout<<"Error sending login accpet to client"<<endl;
			discList.push_back( clt);
		}
	}
}

void	NetServer::sendLoginError( Client * clt, AddressIP ipadr)
{
	Packet	packet2;
	char name[NAMELEN+1];
	char passwd[NAMELEN+1];
	const char * buf = packet.getData();
	strcpy( name, buf);
	strcpy( passwd, buf+NAMELEN);
	// Send a login error
	int		retsend;
	SocketAlt	sockclt = 0;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	packet2.create( LOGIN_ERROR, 0, NULL, 0, 1);
	//packet2.displayHex();
	//cout<<" done."<<endl;
	packet2.tosend();
	if( (retsend = Network->sendbuf( sockclt, (char *) &packet2, packet2.getSendLength(), &ipadr))<0)
		discList.push_back( clt);
	//cout<<"\tLOGIN REQUEST FAILED for <"<<name<<">:<"<<passwd<<">"<<endl;
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
	int		retsend;
	SocketAlt	sockclt = 0;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	packet2.create( LOGIN_ALREADY, 0, NULL, 0, 1);
	//packet2.displayHex();
	//cout<<" done."<<endl;
	packet2.tosend();
	if( (retsend = Network->sendbuf( sockclt, (char *) &packet2, packet2.getSendLength(), &ipadr))<0)
		discList.push_back( clt);
	//cout<<"\tLOGIN CLIENT ALREADY LOGGED IN for <"<<name<<">:<"<<passwd<<">"<<endl;
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

Client *	NetServer::newConnection( AddressIP * ipadr)
{
	SocketAlt	sock;
	Client *	ret = NULL;

	// Just ignore the client for now
	//if( Network->getNumClients()+1>=MAXCLIENT) {}

	// Get new connections if there are - do nothing in standard UDP mode
	sock = Network->acceptNewConn( ipadr);
	if( sock>0)
	{
		// Have to do it another way : 0 or NULL is in most cases considered as an error
		// whereas in SDL UDP mode it can be a valid channel id so I increment it by 1
		// in NetUI class and I have now to decrease it by one
#ifdef HAVE_SDLnet
#ifdef _UDP_PROTO
		sock = sock-1;
#endif
#endif
		ret = this->addNewClient( sock);
		nbclients++;
	}
	return ret;
}

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

Client *	NetServer::addNewClient( SocketAlt sock)
{
	Client * newclt = new Client;
	// New client -> now registering it in the active client list "Clients"

	// Store the associated socket
	newclt->sock = sock;
	// Adds the client
	Clients.push_back( newclt);
	cout<<"Added client with socket n°"<<sock<<" - Actual number of clients : "<<Clients.size()<<endl;

	return newclt;
}

/**************************************************************/
/**** Start the server loop                                ****/
/**************************************************************/

void	NetServer::start()
{
	string strperiod, strtimeout, strlogintimeout, stracct, strnetatom;
	int period;
	keeprun = 1;
	double	savetime=0;
	double	curtime=0;
	double	snaptime=0;
	acct_con = 1;

	startMsg();

	cout<<"Loading server config...";
	vs_config = new VegaConfig( SERVERCONFIGFILE);
	cout<<" config loaded"<<endl;
	strperiod = vs_config->getVariable( "server", "saveperiod", "");
	if( strperiod=="")
		period = 7200;
	else
		period = atoi( strperiod.c_str());
	tmpdir = vs_config->getVariable( "server", "tmpdir", "");
	if( strperiod=="")
		tmpdir = "./tmp/";
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

	string tmp;
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
		NetAcct = new TCPNetUI();
		cout<<"Initializing connection to account server..."<<endl;
		char srvip[256];
		if( vs_config->getVariable( "network", "accountsrvip", "")=="")
		{
			cout<<"Account server IP not specified, exiting"<<endl;
			exit(1);
		}
		memset( srvip, 0, 256);
		memcpy( srvip, (vs_config->getVariable( "network", "accountsrvip", "")).c_str(), vs_config->getVariable( "network", "accountsrvip", "").length());
		unsigned short tmpport;
		if( vs_config->getVariable( "network", "accountsrvport", "")=="")
			tmpport = ACCT_PORT;
		else
			tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
		acct_sock = NetAcct->createSocket( srvip, tmpport, 0);
		cout<<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create and bind socket
	cout<<"Initializing network..."<<endl;
	conn_sock = Network->createSocket( "127.0.0.1", atoi((vs_config->getVariable( "network", "serverport", "")).c_str()), 1);
	cout<<"done."<<endl;
	
	// Server loop
	while( keeprun)
	{
		UpdateTime();
		// Check a key press
		// this->checkKey();
		// Handle new connections in TCP mode
#ifdef _TCP_PROTO
		this->newConnection( NULL);
#endif
		// Check received communications
		this->checkMsg();
		if( acctserver && acct_con)
		{
			// Listen for account server answers
			this->checkAcctMsg();
			// And send to it the login request we received
			// Then send clients confirmations or errors
		}

#ifdef _UDP_PROTO
		// See if we have some timed out clients and disconnect them
		this->checkTimedoutClients();
#endif

		// Remove all clients to be disconnected
		LI j;
		for ( j=discList.begin(); j!=discList.end(); j++)
		{
			this->disconnect( (*j));
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
			zonemgr->broadcastSnapshots( this->Network);
			snapchanged = 0;
		}

		// Check for automatic server status save time (in seconds)
		curtime = getNewTime();
		if( curtime - savetime > period)
		{
			// Not implemented
			cout<<"Saving server status... ";
			this->save();
			savetime += period;
			cout<<"done."<<endl;
		}

		micro_sleep(10000);
	}

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

void	NetServer::checkAcctMsg()
{
	unsigned int len, len2;
	AddressIP	ipadr;
	Client *	clt = NULL;
	unsigned char cmd=0;

	NetAcct->resetSets();
	// Watch account server socket
	NetAcct->watchSocket( acct_sock);
	// Get the number of active clients
	if( NetAcct->activeSockets())
	{
		//cout<<"Net activity !"<<endl;
		// Receive packet and process according to command

		if( (len=NetAcct->recvbuf( acct_sock, (char *)&packeta, len2, &ipadr))>0)
		{
			// Maybe copy that in a "else" condition too if when it fails we have to disconnect a client

			// Here we get the latest client which asked for a login
			// Since coms between game servers and account server are TCP the order of request/answers
			// should be ok and we can use a "queue" for waiting clients
			if( waitList.size()==0)
			{
				cout<<"Error : trying to remove client on empty waitList"<<endl;
				exit( 1);
			}
			#ifdef _TCP_PROTO
			clt = waitList.front();
			#else
			ipadr = waitList.front();
			#endif
			waitList.pop();

			packeta.received();
			switch( packeta.getCommand())
			{
				case LOGIN_NEW :
					// We received a login authorization for a new account (no ship created)
					this->sendLoginAccept( clt, ipadr, 1);
				break;
				case LOGIN_ACCEPT :
					// Login is ok
					cout<<"LOGIN ACCEPTED"<<endl;
					this->sendLoginAccept( clt, ipadr, 0);
				break;
				case LOGIN_ERROR :
					cout<<"LOGIN ERROR"<<endl;
					// Login error -> disconnect
					this->sendLoginError( clt, ipadr);
				break;
				case LOGIN_ALREADY :
					cout<<"LOGIN ERROR - ALREADY LOGGED IN"<<endl;
					// Client already logged in -> disconnect
					this->sendLoginAlready( clt, ipadr);
				break;
				default:
					cout<<"UNKNOWN command "<<cmd<<" FROM ACCOUNT SERVER ! ";
			}
		}
		else
		{
			cout<<"Connection to account server lost !!"<<endl;
			NetAcct->closeSocket( acct_sock);
			acct_con = 0;
		}
	}
}

/**************************************************************/
/**** Check which clients are sending data to the server   ****/
/**************************************************************/

void	NetServer::checkMsg()
{
	Network->resetSets();
#ifdef _TCP_PROTO
	// First add all clients to be watched
	for( LI i=Clients.begin(); i!=Clients.end(); i++)
		Network->watchSocket( (*i)->sock);
	// Get the number of active clients
	int nb = Network->activeSockets();
	if( nb)
	{
		//cout<<"Net activity !"<<endl;
		// Loop for each active client and process request
		for( LI i=Clients.begin(); i!=Clients.end(); i++)
		{
			if( Network->isActive( (*i)->sock))
			{
				this->recvMsg( (*i));
			}
		}
	}
#endif
#ifdef _UDP_PROTO
	// Don't remember why I check for activity but there was a reason :)
	Network->watchSocket( Network->sock);
	int nb = Network->activeSockets();
	if( nb)
	{
		//cout<<"Client activity !"<<endl;
		// UDP uses only 1 socket so we can receive but we don't know which client it is yet
		this->recvMsg( NULL);
	}
#endif
}

/**************************************************************/
/**** Disconnects timed out clients                        ****/
/**************************************************************/

#ifdef _UDP_PROTO
void	NetServer::checkTimedoutClients()
{
	/********* Method 1 : compare latest_timestamp to current time and see if > CLIENTTIMEOUT */
	double curtime = (unsigned int) getNewTime();
	double deltatmp = 0;
	for (LI i=Clients.begin(); i!=Clients.end(); i++)
	{
		deltatmp = (fabs(curtime - (*i)->latest_timeout));
		//cout<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
		// Here considering a delta > 0xFFFFFFFF*X where X should be at least something like 0.9
		// This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
		// side -> when timestamp has grown enough to became bigger than what an u_int can store
		if( (*i)->ingame && deltatmp > clienttimeout /*&& deltatmp < (0xFFFFFFFF*0.8)*/ )
		{
			cout<<"ACTIVITY TIMEOUT for client number "<<(*i)->serial<<endl;
			discList.push_back( *i);
		}
		else if( (*i)->ingame && deltatmp > logintimeout)
		{
			cout<<"LOGIN TIMEOUT for client number "<<(*i)->serial<<endl;
			discList.push_back( *i);
		}
		/*
		else
			cout<<"CLIENT "<<(*i)->serial<<" - DELTA="<<deltatmp<<endl;
		*/
	}
}
#endif

/**************************************************************/
/**** Receive a message from a client                      ****/
/**************************************************************/

void	NetServer::recvMsg( Client * clt)
{
	char	command;
	//char *	buf;
	int len;
	unsigned int len2;
	SocketAlt sockclt=0;
	ObjSerial nserial;
	AddressIP	ipadr;
	unsigned int ts = 0;
	//unsigned int tstmp = 0;

#ifdef _USP_PROTO
	// This should only happen in UDP mode
	if( clt!=NULL)
	{
		sockclt = clt->sock;
	}
#endif

	if( (len=Network->recvbuf( sockclt, (char *)&packet, len2, &ipadr))<=0)
	{
		if( clt!=NULL) // Add the client to the disconnect list
			discList.push_back( clt);
		//else
		//	cout<<"Disconnecting new client"<<endl;
	}
	else
	{
		packet.received();
		//cout<<"Received packet length : "<<packet.getLength()<<endl;
		ts = packet.getTimestamp();
#ifdef _UDP_PROTO
		// Extract the serial from buffer received so we know who it is
		nserial = packet.getSerial();
		//cout<<"Received from serial : "<<nserial<<endl;
		// Find the corresponding client
		Client *	tmp;
		int			found = 0;
		double		curtime;
		command = packet.getCommand();
		for( LI i=Clients.begin(); i!=Clients.end() && !found; i++)
		{
			tmp = (*i);
			if( tmp->serial == nserial)
			{
				clt = tmp;
				found = 1;
			}
		}
		if( !found && command!=CMD_LOGIN)
		{
			cout<<"Error : non-login message received from unknown client !"<<endl;
			// Maybe send an error packet handled by the client
			return;
		}


		// Check if the client's IP is still the same (a very little protection against spoofing client serial#)
		if( clt!=NULL && !Network->isSameAddress( &ipadr, &clt->cltadr))
		{
			cout<<"Error : IP changed for client # "<<clt->serial<<endl;
			discList.push_back( clt);
		}
		else
		{
			if( clt!=NULL)
			{
				// We know which client it is so we update its timeout
				curtime = getNewTime();
				clt->latest_timeout = curtime;
			}
#endif
		/*
		cout<<"Received command n°"<<command;
		if( clt!=NULL)
			cout<<" from client n°"<<clt->serial;
		else
			cout<<" from new client";
		cout<<endl;
		*/
#ifdef _UDP_PROTO
			// Check for a late packet
			//tstmp = fabs( ts-clt->latest_timestamp);
			//if( (clt!=NULL && ts > clt->latest_timestamp && tstmp < (0xFFFFFFFF*0.8)) || clt==NULL)
			//{
#endif
				/*
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
				*/

				//packet.setLength( len);
				//packet.displayHex();
				//buf = packet.getData();
				//cout<<"Received "<<len<<" bytes of data";
				//cout<<"from IP : ";
				//Network->getIPof( ipadr);
				this->processPacket( clt, command, ipadr);
#ifdef _UDP_PROTO
			/*
			}
			else
			{
				cout<<"Received an old packet\n";
				packet.displayCmd();
			}
			*/
		}
#endif
		packet.reset();
	}
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::processPacket( Client * clt, unsigned char cmd, AddressIP ipadr)
{
		Packet p2;
		switch( cmd)
		{
			case CMD_LOGIN:
			{
				// Authenticate client
				// Need to give the IP address of incoming message in UDP mode to store it
				// in the Client struct
				if( !acctserver)
					this->authenticate( clt, ipadr);
				else
				{
				#ifdef _TCP_PROTO
					this->waitList.push( clt);
				#else
					this->waitList.push( ipadr);
				#endif
					// Redirect the login request packet to account server
					cout<<"Redirecting login request to account server on socket "<<acct_sock<<endl;
					cout<<"Packet to copy length : "<<packet.getLength()<<endl;
					p2.create( packet.getCommand(), 0, (char *)packet.getData(), packet.getLength(), 0);
					p2.tosend();
					if(NetAcct->sendbuf( acct_sock, (char *)&p2, p2.getSendLength(), NULL)<0)
					{
						perror( "ERROR sending redirected login request to ACCOUNT SERVER : ");
						cout<<"Socket was : "<<acct_sock<<endl;
						exit(1);
					}
				}
			}
			break;
			case CMD_INITIATE:
				this->sendLocations( clt);
			break;
			case CMD_ADDCLIENT:
				// Add the client to the game
				cout<<"Received ADDCLIENT request"<<endl;
				this->addClient( clt);
			break;
			case CMD_POSUPDATE:
				// Received a position update from a client
				//cout<<"Received POSUPDATE";
				//Network->getIPof( ipadr);
				//cout<<endl;
				this->posUpdate( clt);
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
			break;
			case CMD_LOGOUT:
				// Client wants to quit the game
				logoutList.push_back( clt);
			break;
			default:
				cout<<"Unknown command "<<cmd<<" ! ";
				cout<<"from client "<<clt->serial<<endl;
		}
}

/**************************************************************/
/**** Add a client in the game                             ****/
/**************************************************************/

void	NetServer::addClient( Client * clt)
{
	Packet packet2;
	//int		locserial=0;
	// Should get his last location or a starting location

	// Get the last client zone HERE
	clt->zone = 1;
	zonemgr->addClient( clt, 1);

	// Get the last location of client
	// For now, I set it to default values
	ClientState	tmpcs( clt->serial);
	// Here the other client in the same zone should be warned of a new client
	// Should also send data about the ship !!! filename ? IDs ?
	// maybe those thing should be managed in account.xml
	// For now assuming a default ship on client side
	packet2.create( CMD_ENTERCLIENT, clt->serial, (char *) &tmpcs, sizeof( ClientState), 1);
	//cout<<" 2nd packet -------------"<<endl;
	//packet2.displayHex();
	zonemgr->broadcast( clt, &packet2, this->Network);
	clt->ingame = 1;

	char * cltsbuf = new char[MAXBUFFER];
	int cltsbufsize;
	// Send an accepted entering command and current zone's clients infos
	// So the packet buffer should contain info about other ships (desciptions) present in the zone
	cltsbufsize = zonemgr->getZoneClients( clt, cltsbuf);

	packet2.create( CMD_ADDEDYOU, clt->serial, cltsbuf, cltsbufsize, 1);
	packet2.tosend();
	if( Network->sendbuf( clt->sock, (char *) &packet2, packet2.getSendLength(), &clt->cltadr) == -1)
	{
		cout<<"Error sending ADDED_YOU to client n°"<<clt->serial<<endl;
		exit(1);
	}
	cout<<"ADDED client n "<<clt->serial<<" in ZONE "<<clt->zone<<endl;
	delete cltsbuf;
}

/***************************************************************/
/**** Adds the client update to current client's zone snapshot */
/***************************************************************/

void	NetServer::posUpdate( Client * clt)
{
	//Packet	pckt;
	//pckt.create( CMD_UPDATECLT, clt->serial, (char*) packet.getData(), sizeof( ClientState));
	//zonemgr->broadcast( clt, pckt, this->Network);

	// Set old position
	memcpy( &clt->old_state, &clt->current_state, sizeof( ClientState));
	// Update client position in client list
	memcpy( &clt->current_state, packet.getData(), sizeof( ClientState));
	// Put deltatime in the delay part of ClientState so that it is send to other clients later
	clt->current_state.received();
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

void	NetServer::disconnect( Client * clt)
{
	Packet p, p1, p2;

	if( acctserver)
	{
		// Send a disconnection info to account server
		char * buf = new char[NAMELEN*2+1];
		int nbc = strlen( clt->name);
		memcpy( buf, clt->name, nbc);
		p2.create( CMD_LOGOUT, clt->serial, buf, nbc , 1);
		p2.tosend();
		if( NetAcct->sendbuf( acct_sock, (char *) &p2, p2.getSendLength(), &clt->cltadr)<0)
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		delete buf;
	}

#ifdef _TCP_PROTO
	Network->closeSocket( clt->sock);
#else
	p1.create( CMD_DISCONNECT, clt->serial, NULL, 0, 1);
	p1.tosend();
	if( Network->sendbuf( clt->sock, (char *) &p1, p1.getSendLength(), &clt->cltadr)<0)
		cout<<"ERROR sending DISCONNECT"<<endl;
#endif
	cout<<"Client "<<clt->serial<<" disconnected"<<endl;
	if( clt->ingame)
		zonemgr->removeClient( clt);
	cout<<"There was "<<Clients.size()<<" clients - ";
	Clients.remove( clt);
	// Broadcast client EXIT zone
	p.create( CMD_EXITCLIENT, clt->serial, NULL, 0, 1);
	zonemgr->broadcast( clt, &p, this->Network);
	if( clt != NULL)
	{
		delete clt;
		clt = NULL;
	}
	cout<<Clients.size()<<" clients left"<<endl;
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
		memcpy( buf, clt->name, nbc);
		p2.create( CMD_LOGOUT, clt->serial, buf, nbc , 1);
		p2.tosend();
		if( NetAcct->sendbuf( acct_sock, (char *) &p2, p2.getSendLength(), &clt->cltadr)<0)
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		delete buf;
	}

#ifdef _TCP_PROTO
	Network->closeSocket( clt->sock);
#endif
	cout<<"Client "<<clt->serial<<" disconnected"<<endl;
	if( clt->ingame)
		zonemgr->removeClient( clt);
	cout<<"There was "<<Clients.size()<<" clients - ";
	Clients.remove( clt);
	// Broadcast client EXIT zone
	p.create( CMD_EXITCLIENT, clt->serial, NULL, 0, 1);
	zonemgr->broadcast( clt, &p, this->Network);
	if( clt != NULL)
	{
		delete clt;
		clt = NULL;
	}
	cout<<Clients.size()<<" clients left"<<endl;
	nbclients--;
	//exit( 1);
}

/**************************************************************/
/**** Close all sockets for shutdown                       ****/
/**************************************************************/

void	NetServer::closeAllSockets()
{
	Network->disconnect( "Closing sockets");
#ifdef _TCP_PROTO
	for( LI i=Clients.begin(); i!=Clients.end(); i++)
		Network->closeSocket( (*i)->sock);
#endif
}

/**************************************************************/
/**** Save the server state                                ****/
/**************************************************************/

void	NetServer::save()
{
}
