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

#include "client.h"
#include "packet.h"
#include "lin_time.h"
#include "netserver.h"
#include "vsnet_serversocket.h"

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
	// Network = new DefaultNetUI;
	this->nbclients = 0;
	this->nbaccts = 0;
	this->keeprun = 1;
	this->acctserver = 0;
	this->srvtimeout.tv_sec = 0;
	this->srvtimeout.tv_usec = 0;
	/***** number of zones should be determined as server loads zones files *****/
	zonemgr = new ZoneMgr( 10);
	UpdateTime();
	srand( (unsigned int) getNewTime());
	// Here 500 could be something else between 1 and 0xFFFF
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
}

NetServer::~NetServer()
{
	// if( Network!=NULL) delete Network;
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
    const char * buf = packeta.getData();
    strcpy( name, buf);
    strcpy( passwd, buf+NAMELEN);

#ifdef _UDP_PROTO
	// In UDP mode, client is created here
	clt = newConnection_udp( ipadr );
	if( !clt)
	{
		cout<<"Error creating new client connection"<<endl;
		exit(1);
	}
#endif
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
		// const char * xml = buf + NAMELEN*2 + sizeof( unsigned int);
		unsigned int xml_size = ntohl( *( (unsigned int *)(buf+NAMELEN*2)));
		// const char * save = buf + NAMELEN*2 + sizeof( unsigned int)*2 + xml_size;
		unsigned int save_size = ntohl( *( (unsigned int *)(buf+ NAMELEN*2 + sizeof( unsigned int) + xml_size)));
		cout<<"XML="<<xml_size<<" bytes - SAVE="<<save_size<<" bytes"<<endl;

		//string strname( name);
		// Write temp XML file for unit
		//string tmp;
		//tmp = tmpdir+name+".xml";
		//WriteXMLUnit( tmp.c_str(), xml, xml_size);
		// Then load it in the Unit struct
		//LoadXMLUnit( clt->game_unit, tmp.c_str(), NULL);

                Packet packet2;
		packet2.send( LOGIN_ACCEPT, clt->serial, packeta.getData(), packeta.getDataLength(), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
		cout<<"<<< SENT LOGIN ACCEPT -----------------------------------------------------------------------"<<endl;
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
	// int		retsend;
	SOCKETALT	sockclt;
	if( clt!=NULL)
		sockclt = clt->sock;
	//cout<<"Creating packet... ";
	cout<<">>> SEND LOGIN ERROR -----------------------------------------------------------------"<<endl;
	packet2.send( LOGIN_ERROR, 0, NULL, 0, SENDRELIABLE, &ipadr, sockclt, __FILE__, __LINE__ );
	cout<<"<<< SENT LOGIN ERROR -----------------------------------------------------------------------"<<endl;
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
	cout<<">>> SEND LOGIN ALREADY =( serial n°"<<clt->serial<<" )= --------------------------------------"<<endl;
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
    COUT << " enter " << __FUNCTION__ << endl;

    if( Network == NULL ) return NULL;

    SOCKETALT sock( Network->get_udp_sock(), SOCKETALT::UDP, ipadr );

    Client* ret = addNewClient( sock);
    nbclients++;

    return ret;
}

Client* NetServer::newConnection_tcp( SocketSet& set )
{
    SOCKETALT	sock;
    Client*		ret = NULL;

    // Just ignore the client for now

    // Get new connections if there are - do nothing in standard UDP mode
    if( Network->isActive( set ) )
    {
        COUT << " enter " << __FUNCTION__ << endl;
        sock = Network->acceptNewConn( set );
        if( sock.valid() )
        {
            ret = this->addNewClient( sock);
            nbclients++;
        }
    }
    return ret;
}

/**************************************************************/
/**** Adds a new client                                    ****/
/**************************************************************/

Client* NetServer::addNewClient( SOCKETALT sock)
{
    Client * newclt = new Client;
    // New client -> now registering it in the active client list "Clients"

    // Store the associated socket
    newclt->sock = sock;
    // Adds the client
    Clients.push_back( newclt);
    COUT<<"Added client with socket n°"<<sock<<" - Actual number of clients : "<<Clients.size()<<endl;

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
			cleanup();
		}
		memset( srvip, 0, 256);
		memcpy( srvip, (vs_config->getVariable( "network", "accountsrvip", "")).c_str(), vs_config->getVariable( "network", "accountsrvip", "").length());
		unsigned short tmpport;
		if( vs_config->getVariable( "network", "accountsrvport", "")=="")
			tmpport = ACCT_PORT;
		else
			tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
		acct_sock = NetAcct->createSocket( srvip, tmpport );
		COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;
	}

	// Create and bind socket
	COUT << "Initializing network..." << endl;
	Network = NetworkToClient.createServerSocket( atoi((vs_config->getVariable( "network", "serverport", "")).c_str()) );
	COUT << "done." << endl;
	
	// Server loop
	while( keeprun)
	{
		SocketSet set;
		int       nb;

		UpdateTime();
		// Check a key press
		// this->checkKey();
		// Handle new connections in TCP mode
#ifdef _TCP_PROTO
		Network->watchForNewConn( set, 0 );
#endif
		// Check received communications
		prepareCheckMsg( set );
		if( acctserver && acct_con) prepareCheckAcctMsg( set );
		nb = set.select( NULL );
		if( nb > 0 )
		{
#ifdef _TCP_PROTO
			newConnection_tcp( set );
#endif
			checkMsg( set );
			if( acctserver && acct_con)
			{
				// Listen for account server answers
				checkAcctMsg( set );
				// And send to it the login request we received
				// Then send clients confirmations or errors
			}
		}

#ifdef _UDP_PROTO
		// See if we have some timed out clients and disconnect them
		this->checkTimedoutClients();
#endif

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
			zonemgr->broadcastSnapshots( &NetworkToClient );
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

void	NetServer::prepareCheckAcctMsg( SocketSet& set )
{
    acct_sock.watch( set );
}

void	NetServer::checkAcctMsg( SocketSet& set )
{
	unsigned int len2;
	int len=0;
	AddressIP	ipadr, ip2;
	Client *	clt = NULL;
	unsigned char cmd=0;
	char buffer[MAXBUFFER];

	// Watch account server socket
	// Get the number of active clients
	if( acct_sock.isActive( set ))
	{
		//cout<<"Net activity !"<<endl;
		// Receive packet and process according to command

		len2 = MAXBUFFER;
		if( (len=acct_sock.recvbuf( buffer, len2, &ip2))>0)
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
#ifdef _TCP_PROTO
			clt = waitList.front();
#else
			ipadr = waitList.front();
			COUT << "Got response for client IP : " << ipadr << endl;
#endif
			waitList.pop();

			Packet p( buffer, len );
			packeta = p;
			switch( packeta.getCommand())
			{
				case LOGIN_NEW :
					cout<<">>> NEW LOGIN =( serial n°"<<packet.getSerial()<<" )= --------------------------------------"<<endl;
					// We received a login authorization for a new account (no ship created)
					this->sendLoginAccept( clt, ipadr, 1);
					cout<<"<<< NEW LOGIN ----------------------------------------------------------------"<<endl;
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
#ifdef _TCP_PROTO
	// First add all clients to be watched
	for( LI i=Clients.begin(); i!=Clients.end(); i++)
	{
		(*i)->sock.watch( set );
	}
#endif
#ifdef _UDP_PROTO
	Network->watch( set );
#endif
}

void	NetServer::checkMsg( SocketSet& set )
{
#ifdef _TCP_PROTO
	for( LI i=Clients.begin(); i!=Clients.end(); i++)
	{
		if( (*i)->sock.isActive( set ) )
		{
			this->recvMsg_tcp( (*i));
		}
	}
#endif
#ifdef _UDP_PROTO
	this->recvMsg_udp( );
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
		if( (*i)->latest_timeout!=0)
		{
			//cout<<"DELTATMP = "<<deltatmp<<" - clienttimeout = "<<clienttimeout<<endl;
			// Here considering a delta > 0xFFFFFFFF*X where X should be at least something like 0.9
			// This allows a packet not to be considered as "old" if timestamp has been "recycled" on client
			// side -> when timestamp has grown enough to became bigger than what an u_int can store
			if( (*i)->ingame && deltatmp > clienttimeout && deltatmp < (0xFFFFFFFF*0.8) )
			{
				cout<<"ACTIVITY TIMEOUT for client number "<<(*i)->serial<<endl;
				cout<<"\t\tCurrent time : "<<curtime<<endl;
				cout<<"\t\tLatest timeout : "<<((*i)->latest_timeout)<<endl;
				cout<<"t\tDifference : "<<deltatmp<<endl;
				discList.push_back( *i);
			}
			else if( !(*i)->ingame && deltatmp > logintimeout)
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
#endif

/**************************************************************/
/**** Receive a message from a client                      ****/
/**************************************************************/

void	NetServer::recvMsg_tcp( Client * clt)
{
    char	command;
    AddressIP	ipadr;
    int nbpackets = 0;
    double ts = 0;

    assert( clt != NULL );

    SOCKETALT sockclt( clt->sock );
    char      buffer[MAXBUFFER];
    unsigned int    len = MAXBUFFER;

    int recvbytes = sockclt.recvbuf( buffer, len, &ipadr );

    if( recvbytes <= 0 )
    {
	COUT << "received " << recvbytes << " bytes from " << sockclt
	     << ", disconnecting" << endl;
        discList.push_back( clt);
    }
    else
    {
        Packet packet( buffer, len );
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

	COUT << "Created a packet with command " << displayCmd(Cmd(command)) << endl;
	PacketMem m( buffer, len );
	m.dump( cout, 3 );

        this->processPacket( clt, command, ipadr, packet );
    }
}

void NetServer::recvMsg_udp( )
{
    SOCKETALT sockclt( Network->get_udp_sock(), SOCKETALT::UDP, Network->get_adr() );
    Client*   clt = NULL;
    AddressIP ipadr;

    char   buffer[MAXBUFFER];
    unsigned int len = MAXBUFFER;
    int    ret;
    ret = sockclt.recvbuf( buffer, len, &ipadr );
    if( ret > 0 )
    {
        Packet packet( buffer, len );
	packet.setNetwork( &ipadr, sockclt );

        double    ts      = packet.getTimestamp();
        ObjSerial nserial = packet.getSerial(); // Extract the serial from buffer received so we know who it is
        char      command = packet.getCommand();

        COUT << "Received from serial : " << nserial << endl;

        // Find the corresponding client
        Client* tmp   = NULL;
	bool    found = false;
        for( LI i=Clients.begin(); i!=Clients.end(); i++)
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

        // Check if the client's IP is still the same (a very little protection against spoofing client serial#)
        if( clt!=NULL && ipadr!=clt->cltadr )
        {
	    assert( command != CMD_LOGIN ); // clt should be 0 because ObjSerial was 0

            COUT << "Error : IP changed for client # " << clt->serial << endl;
            discList.push_back( clt);
	    /* It is not entirely impossible for this to happen; it would be nice to add an additional identity
	     * check. For now we consider it an error.
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
            else
            {
                const AddressIP* iptmp;
#ifdef _TCP_PROTO
                this->waitList.push( clt);
                iptmp = &clt->cltadr;
#else
                this->waitList.push( ipadr);
                iptmp = &ipadr;
                COUT << "Waiting authorization for client IP : " << ipadr << endl;
#endif
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
	//int		locserial=0;
	// Should get his last location or a starting location

	// Get the last client zone HERE
	clt->zone = 1;
	zonemgr->addClient( clt, 1);

	// GET THE INITIAL CLIENTSTATE FROM PACKET AND SET IT !!
	// For now, I set it to default values
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
	packet2.bc_create( CMD_ENTERCLIENT, clt->serial, (char *) &tmpcs, sizeof( ClientState), SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
	cout<<"<<< SEND ENTERCLIENT -----------------------------------------------------------------------"<<endl;
	zonemgr->broadcast( clt, &packet2, &NetworkToClient );
	cout<<">>> SEND ADDED YOU =( serial n°"<<clt->serial<<" )= --------------------------------------"<<endl;
	clt->ingame = 1;

	char * cltsbuf = new char[MAXBUFFER];
	int cltsbufsize;
	// Send an accepted entering command and current zone's clients infos
	// So the packet buffer should contain info about other ships (desciptions) present in the zone
	cltsbufsize = zonemgr->getZoneClients( clt, cltsbuf);
	cout<<"Serial : "<<clt->serial<<endl;
	packet2.send( CMD_ADDEDYOU, clt->serial, cltsbuf, cltsbufsize, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
	cout<<"ADDED client n "<<clt->serial<<" in ZONE "<<clt->zone<<endl;
	delete cltsbuf;
	cout<<"<<< SENT ADDED YOU -----------------------------------------------------------------------"<<endl;
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
	//memcpy( &clt->old_state, &clt->current_state, sizeof( ClientState));
	clt->old_state = clt->current_state;
	// Update client position in client list
	//memcpy( &clt->current_state, packet.getData(), sizeof( ClientState));
	clt->current_state = *((ClientState *) packet.getData());
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
		if( p2.send( CMD_LOGOUT, clt->serial, buf, NAMELEN+NAMELEN , SENDANDFORGET, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		delete buf;
	}

#ifdef _TCP_PROTO
	// Network->closeSocket( clt->sock);
	clt->sock.disconnect( __PRETTY_FUNCTION__, false );
#else
	Packet p1;
	p1.send( CMD_DISCONNECT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
#endif
	cout<<"Client "<<clt->serial<<" disconnected"<<endl;
	if( clt->ingame)
		zonemgr->removeClient( clt);
	cout<<"There was "<<Clients.size()<<" clients - ";
	Clients.remove( clt);
	// Broadcast client EXIT zone
	Packet p;
	p.bc_create( CMD_EXITCLIENT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
	zonemgr->broadcast( clt, &p, &NetworkToClient );
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
		memcpy( buf, clt->name, nbc+1);
		if( p2.send( CMD_LOGOUT, clt->serial, buf, nbc, SENDANDFORGET, NULL, acct_sock, __FILE__, __LINE__ ) < 0 )
			cout<<"ERROR sending LOGOUT to account server"<<endl;
		delete buf;
	}

#ifdef _TCP_PROTO
	// Network->closeSocket( clt->sock);
	clt->sock.disconnect( __PRETTY_FUNCTION__, false );
#endif
	cout<<"Client "<<clt->serial<<" disconnected"<<endl;
	if( clt->ingame)
		zonemgr->removeClient( clt);
	cout<<"There was "<<Clients.size()<<" clients - ";
	Clients.remove( clt);
	// Broadcast client EXIT zone
	if( clt->ingame)
	{
		// p.create( CMD_EXITCLIENT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->serial);
		p.bc_create( CMD_EXITCLIENT, clt->serial, NULL, 0, SENDRELIABLE, &clt->cltadr, clt->sock, __FILE__, __LINE__ );
		zonemgr->broadcast( clt, &p, &NetworkToClient );
	}
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
	Network->disconnect( "Closing sockets" );
#ifdef _TCP_PROTO
	for( LI i=Clients.begin(); i!=Clients.end(); i++)
	{
		// Network->closeSocket( (*i)->sock);
		(*i)->sock.disconnect( __PRETTY_FUNCTION__, false );
	}
#endif
}

/**************************************************************/
/**** Save the server state                                ****/
/**************************************************************/

void	NetServer::save()
{
}
