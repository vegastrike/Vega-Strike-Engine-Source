#include "cmd/unit_generic.h"
#include "client.h"
#include "acctserver.h"
#include "packet.h"
#include "lin_time.h"
#include "vsnet_serversocket.h"

VegaConfig * vs_config;
string acctdir;

AccountServer::AccountServer()
{
	cout<<"AccountServer init"<<endl;
	// Network = new TCPNetUI;
	newaccounts = 0;
	UpdateTime();
	srand( (int) getNewTime());
	serial_seed = (ObjSerial) (rand()*(500./(((double)(RAND_MAX))+1)));
}

/**************************************************************/
/**** Authenticate a connected client                      ****/
/**************************************************************/

ObjSerial	AccountServer::getUniqueSerial()
{
	// MAYBE CHANGE TO SOMETHING MORE "RANDOM"
	serial_seed = (serial_seed+3)%MAXSERIAL;
	return serial_seed;
}

AccountServer::~AccountServer()
{
	delete Network;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void	AccountServer::startMsg()
{
	cout<<endl<<"Vegastrike Account Server version 0.0.1"<<endl;
	cout<<"Written by Stephane Vaxelaire"<<endl<<endl;
}

void	AccountServer::start()
{
	string strperiod;
	keeprun = 1;

	SOCKETALT	comsock;

	startMsg();

	cout<<"Loading config file...";
	vs_config = new VegaConfig( ACCTCONFIGFILE);
	cout<<" done."<<endl;
	InitTime();
	UpdateTime();
	acctdir = vs_config->getVariable( "server", "accounts_dir", "");
	if( acctdir=="")
		acctdir = "./accounts/";
	strperiod = vs_config->getVariable( "server", "saveperiod", "");
	int period;
	if( strperiod=="")
		period = 7200;
	else
		period = atoi( strperiod.c_str());
	savetime = getNewTime()+period;

	cout<<"Loading accounts data... ";
	LoadAccounts( "accounts.xml");
	// Gets hashtable accounts elements and put them in vector Cltacct
	Cltacct = getAllAccounts();
	if(Cltacct.size()<=0)
	{
		cout<<"No account found in accounts.xml"<<endl;
		cleanup();
	}
	cout<<Cltacct.size()<<" accounts loaded."<<endl;

	// Create and bind socket
	cout<<"Initializing network..."<<endl;
	unsigned short tmpport;
	if( vs_config->getVariable( "network", "accountsrvport", "")=="")
		tmpport = ACCT_PORT;
	else
		tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
	Network = NetworkToClient.createServerSocket( tmpport );
	cout<<"done."<<endl;
	while( keeprun)
	{
		//cout<<"Loop"<<endl;
		// Check for incoming connections
		SocketSet set;

		Network->watchForNewConn( set, 0 );
		// Check sockets to be watched
		LS i;
		for( i=Socks.begin(); i!=Socks.end(); i++)
		{
			cout << "Adding an open connection to select" << endl;
	    	i->watch( set );
		}

		set.select( NULL );

		comsock = Network->acceptNewConn( set );
		if( comsock.valid() )
		{
			Socks.push_back( comsock);
			cout<<"New connection - socket allocated : "<<comsock<<endl;
		}

		// Loop for each active client and process request
		for( i=Socks.begin(); i!=Socks.end(); i++)
		{
			if( i->isActive( set ) )
			{
				this->recvMsg( (*i));
			}
		}

		// Remove dead connections
		this->removeDeadSockets();
		// Check for automatic server status save time
		curtime = getNewTime();

		micro_sleep(40000);
	}

	delete vs_config;
	Network->disconnect( "Shutting down.");
}

void	AccountServer::recvMsg( SOCKETALT sock)
{
	char			name[NAMELEN+1];
	char			passwd[NAMELEN+1];
	AddressIP		ipadr;
	unsigned int	len=0;
	int				recvcount=0;
	unsigned char	cmd;
	Account *		elem = NULL;
	int 			found=0, connected=0;
	char buffer[MAXBUFFER];

	// Receive data from sock
	//cout<<"Receiving on socket "<<sock.fd<<endl;
	len = MAXBUFFER;
	if( (recvcount = sock.recvbuf( buffer, len, &ipadr))>0)
	{
		Packet p( buffer, recvcount );

		packet = p;
		// Check the command of the packet
		cmd = packet.getCommand();
		const char * buf = packet.getData();
		cout<<"Buffer => "<<buf<<endl;
		VI j;
		switch( cmd)
		{
			case CMD_LOGIN :
				strcpy( name, buf);
				strcpy( passwd, buf+NAMELEN);
				cout<<">>> LOGIN REQUEST =( "<<name<<":"<<passwd<<" )= --------------------------------------"<<endl;

				for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						// We found the client in the account list
						found = 1;
						if( elem->isConnected())
						{
							// and he is connected
							connected = 1;
						}
					}
				}
				if( !found)
				{
					cout<<"Login/passwd not found"<<endl;
					Account elt(name, passwd);
					this->sendUnauthorized( sock, &elt);
					delete elem;
				}
				else
				{
					if( connected)
					{
						cout<<"Login already connected !"<<endl;
						this->sendAlreadyConnected( sock, elem);
					}
					else
					{
						cout<<"Login accepted !"<<endl;
						this->sendAuthorized( sock, elem);
						elem->setConnected( true);
					}
				}
				cout<<"<<< LOGIN REQUEST ------------------------------------------"<<endl;
			break;
			case CMD_LOGOUT :
				packet.display( "", 0);
				strcpy( name, buf);
				strcpy( passwd, buf+NAMELEN);
				cout<<">>> LOGOUT REQUEST =( "<<name<<":"<<passwd<<" )= --------------------------------------"<<endl;
				// Receive logout request containing name of player
				for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						found = 1;
						if( elem->isConnected())
							connected = 1;
						else
							connected = 0;
					}
				}
				if( !found)
				{
					cout<<"ERROR LOGOUT -> didn't find player to disconnect = <"<<name<<">:<"<<passwd<<">"<<endl;
				}
				else
				{
					if( connected)
					{
						elem->setConnected( false);
						cout<<"-= "<<name<<" =- Disconnected"<<endl;
					}
					else
					{
						cout<<"ERROR LOGOUT -> player exists but wasn't connected ?!?!"<<endl;
					}
				}
				cout<<"<<< LOGOUT REQUEST ---------------------------------------"<<endl;
			break;
			case CMD_NEWCHAR :
				cout<<">>> NEW CHAR REQUEST =( "<<name<<" )= --------------------------------------"<<endl;
				// Should receive the result of the creation of a new char/ship
				cout<<"<<< NEW CHAR REQUEST -------------------------------------------------------"<<endl;
			break;
			case CMD_NEWSUBSCRIBE :
				cout<<">>> SUBSRIBE REQUEST =( "<<name<<" )= --------------------------------------"<<endl;
				// Should receive a new subscription
				cout<<"<<< SUBSRIBE REQUEST --------------------------------------"<<endl;
			break;
			case CMD_RESYNCACCOUNTS:
			{
				cout<<">>> RESYNC ACCOUNTS --------------------------------------"<<endl;
				// Should receive a list of active players for the concerned server
				// So go through the Account list, look if server_sock == sock
				// Then compare status (connected and now not anymore...)
				ObjSerial nbclients = packet.getSerial();
				int i=0;
				cout<<">>>>>>>>> SYNC RECEIVED FOR "<<nbclients<<" CLIENTS <<<<<<<<<<<<"<<endl;
				ObjSerial sertmp;
				VI vi;
				
				// Loop through accounts
				// Maybe not necessary since when we get a game server disconnect we should have
				// set all its accounts disconnected
				/*
				for( vi = Cltacct.begin; vi!=Cltacct.end(); vi++)
				{
					// Check if the socket correspond to the one of the current account
					// -> it was a client on the server that asked for sync
					// We set them to disconnected in order to reactivate those in the received list
					if( (*vi)->getSocket() == sock)
						(*vi)->setConnected( false);
				}
				*/
				// Loop through received client serials
				for( i=0; i<nbclients; i++)
				{
					sertmp = ntohs( *( (ObjSerial *)(buf+sizeof( ObjSerial)*i)));
					// Loop through accounts
					for( vi = Cltacct.begin(); vi!=Cltacct.end(); vi++)
					{
						// Reactivate the serial we received from server
						if( (*vi)->getSerial() == sertmp)
							(*vi)->setConnected( true);
					}
				}
				cout<<"<<< RESYNC'ED ACCOUNTS --------------------------------------"<<endl;
			}
			break;
			case CMD_SAVEACCOUNTS :
				cout<<">>> SAVING ACCOUNT N° "<<packet.getSerial()<<"-----------------------------"<<endl;
				this->writeSave( packet.getData());
				cout<<"<<< ACCOUNT SAVED --------------------------------------"<<endl;
			break;
			default:
				cout<<">>> UNKNOWN command =( "<<cmd<<" )= ---------------------------------";
		}
		//cout<<"end received"<<endl;
	}
	/*
	else if( recvcount==0)
	{
		// SOCKET should have been closed by that game server
		cout<<"Received 0 data on socket "<<sock<<endl;
	}
	*/
	else
	{
		cout<<"Received failed or socket closed"<<endl;
		DeadSocks.push_back( sock);
	}
}

void	AccountServer::sendAuthorized( SOCKETALT sock, Account * acct)
{
	// Get a serial for client
	ObjSerial serial = getUniqueSerial();
	acct->setSerial( serial);
	cout<<"\tLOGIN REQUEST SUCCESS for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
	// Store socket as a game server id
	acct->setSocket( sock);
	// Verify that client already has a ship or if it is a new account
	if( acct->isNew())
	{
		// Send a command to make the client create a new character/ship
		Packet	packet2;
		if( packet2.send( LOGIN_NEW, serial, packet.getData(), packet.getDataLength(), SENDANDFORGET, NULL, sock, __FILE__, __LINE__ ) < 0 )
		{
			cout<<"ERROR sending authorization"<<endl;
			exit( 1);
		}
		// Should receive an answer from game server that contains ship's creation info to
		// be saved on the account server
	}
	else
	{
		// Should get the data about the player state and data so they can be sent with ACCEPT
		char	buf[MAXBUFFER];
		memcpy( buf, packet.getData(), NAMELEN*2);
		unsigned int maxsave = MAXBUFFER - Packet::getHeaderLength() - 2*NAMELEN - 2*sizeof( unsigned int);
		unsigned int readsize=0, readsize2=0;

		// Read the XML unit file
		string acctfile = acctdir+acct->name+".save";
		cout<<"Trying to open : "<<acctfile<<endl;
		FILE *fp = fopen( acctfile.c_str(), "r");
		if( fp == NULL)
		{
			cout<<"Account file does not exists... sending default one to game server"<<endl;
			acctfile = acctdir+"default.save";
			cout<<"Trying to open : "<<acctfile<<endl;
			fp = fopen( acctfile.c_str(), "r");
		}
		if( fp!=NULL)
		{
			readsize = fread( (buf+2*NAMELEN+sizeof( unsigned int)), sizeof( char), maxsave, fp);
			if( readsize>=maxsave)
			{
				cout<<"Error : account file is bigger than "<<maxsave<<" ("<<readsize<<")"<<endl;
				exit( 1);
			}
			fclose( fp);
		}
		else
		{
			cout<<"Error, default xml save not found"<<endl;
			cleanup();
		}
		// Put the size of the first save file in the buffer to send
		unsigned int savesize = htonl( readsize);
		memcpy( buf+2*NAMELEN, &savesize, sizeof( unsigned int));
		//unsigned int xml_size = ntohl( *( (unsigned int *)(buf+NAMELEN*2)));
		//cout<<"XML reversed = "<<xml_size<<endl;

		// Read the save file
		string acctsave = acctdir+acct->name+".xml";
		fp = fopen( acctsave.c_str(), "r");
		if( fp == NULL)
		{
			cout<<"Save file does not exists... sending default one to game server"<<endl;
			acctsave = acctdir+"default.xml";
			cout<<"Trying to open : "<<acctsave<<endl;
			fp = fopen( acctsave.c_str(), "r");
		}
		if( fp!=NULL)
		{
			// Read the XML unit file
			readsize2 = fread( (buf+readsize+2*NAMELEN+2*sizeof( unsigned int)), sizeof( char), maxsave, fp);
			if( (readsize2+readsize) >= maxsave)
			{
				cout<<"Error : save file is bigger than "<<maxsave<<" ("<<readsize2<<")"<<endl;
				exit( 1);
			}
			fclose( fp);
		}
		else
		{
			cout<<"Error, default save not found"<<endl;
			cleanup();
		}
		// Put the size of the second save file in the buffer to send
		unsigned int xmlsize = htonl( readsize2);
		//cout<<"NETWORK FORMAT : XML size = "<<xmlsize<<" --- SAVE size = "<<savesize<<endl;
		//cout<<"HOST FORMAT : XML size = "<<ntohl(xmlsize)<<" --- SAVE size = "<<ntohl(savesize)<<endl;
		memcpy( buf+2*NAMELEN+sizeof( unsigned int)+readsize, &xmlsize, sizeof( unsigned int));
		cout<<"Loaded -= "<<acct->name<<" =- save files ("<<(readsize+readsize2)<<")"<<endl;
		unsigned int total_size = readsize+readsize2+2*NAMELEN+2*sizeof( unsigned int);
		assert( total_size <= MAXBUFFER );

		// ??? memcpy( buf, packet.getData(), packet.getLength());

		// For now saves are really limited to maxsave bytes
		Packet	packet2;
		if( packet2.send( LOGIN_ACCEPT, serial, buf, total_size, SENDANDFORGET|COMPRESSED, NULL, sock, __FILE__, __LINE__ ) < 0 )
		{
			cout<<"ERROR sending authorization"<<endl;
			exit( 1);
		}
	}
}

void	AccountServer::sendUnauthorized( SOCKETALT sock, Account * acct)
{
	Packet	packet2;
	packet2.send( LOGIN_ERROR, 0, packet.getData(), packet.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ );
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
}

void	AccountServer::sendAlreadyConnected( SOCKETALT sock, Account * acct)
{
	Packet	packet2;
	packet2.send( LOGIN_ALREADY, acct->getSerial(), packet.getData(), packet.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ );
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<"> -> ALREADY LOGGED IN"<<endl;
}

void	AccountServer::save()
{
	// Loop through all accounts and write their status
	VI vi;
	for( vi=Cltacct.begin(); vi!=Cltacct.end(); vi++)
	{
	}
}

void	AccountServer::removeDeadSockets()
{
	VI vi;
	for (LS j=DeadSocks.begin(); j!=DeadSocks.end(); j++)
	{
		int nbc_disc = 0;
		cout<<">>>>>>> Closing socket number "<<(*j)<<endl;
		// Disconnect all of that server clients
		for( vi = Cltacct.begin(); vi!=Cltacct.end(); vi++)
		{
			// Check if the socket correspond to the one of the current account
			// -> it was a client on the server that got disconnected
			// We set them to disconnected in order to reactivate those in the resync list if we receive one

			// We check only if it is the same address since the "fd" member of the socket should have been
			// set to -1 because of disconnect
			if( (*vi)->getSocket().sameAddress( (*j)))
			{
				(*vi)->setConnected( false);
				nbc_disc++;
			}
		}
		cout<<"\tDisconnected "<<nbc_disc<<" clients associated with that server socket"<<endl;
		j->disconnect( "\tclosing socket", false );
		Socks.remove( (*j));
	}
	DeadSocks.clear();
}

void	AccountServer::writeSave( const char * buffer)
{
	vector<string> saves = FileUtil::GetSaveFromBuffer( buffer);
	string xmlstr = saves[0];
	string savestr = saves[1];

	// Find the account associated with serial packet.getSerial();
	VI vi;
	bool found = false;
	for( vi=Cltacct.begin(); vi!=Cltacct.end() && !found; vi++)
	{
		if( (*vi)->getSerial()==packet.getSerial())
			found = true;
	}
	if( !found)
	{
		// Problem, we should have found it
	}
	else
		// Save the files
		FileUtil::WriteSaveFiles( savestr, xmlstr, acctdir, (*vi)->name);
}
