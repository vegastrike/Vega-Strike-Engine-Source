#include "acctserver.h"
#include "packet.h"
#include "lin_time.h"

VegaSimpleConfig * acct_config;

AccountServer::AccountServer()
{
	Network = new TCPNetUI();
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
	double	savetime;
	double	curtime;

	Socket	comsock;

	startMsg();

	cout<<"Loading config file...";
	acct_config = new VegaSimpleConfig( ACCTCONFIGFILE);
	cout<<" done."<<endl;
	InitTime();
	UpdateTime();
	strperiod = acct_config->getVariable( "server", "saveperiod", "");
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
	cout<<Cltacct.size()<<" accounts loaded."<<endl;

	// Create and bind socket
	cout<<"Initializing network..."<<endl;
	unsigned short tmpport;
	if( acct_config->getVariable( "network", "accountsrvport", "")=="")
		tmpport = ACCT_PORT;
	else
		tmpport = atoi((acct_config->getVariable( "network", "accountsrvport", "")).c_str());
	Network->createSocket( "127.0.0.1", tmpport, 1);
	cout<<"done."<<endl;
	while( keeprun)
	{
		//cout<<"Loop"<<endl;
		// Check for incoming connections
		comsock = Network->acceptNewConn( NULL);
		// Add comsock in the Network's client_set
		// Test which ones are active
		// Finally checkMsg() for active sockets
		if( comsock)
		{
			cout<<"Receiving message"<<endl;
			this->checkMsg( comsock);
			// Close the socket for that request
			Network->closeSocket( comsock);
		}

		// Check for automatic server status save time
		curtime = getNewTime();
		if( curtime - savetime > period)
		{
			// Not implemented
			this->save();
			savetime += period;
		}

		micro_sleep(40000);
	}

	delete acct_config;
	Network->disconnect( "Shutting down.");
}

void	AccountServer::checkMsg( SocketAlt sock)
{
	char			name[NAMELEN+1];
	char			passwd[NAMELEN+1];
	AddressIP		ipadr;
	unsigned int	len=0;
	int				recvcount=0;
	unsigned char	cmd;
	Account *		elem = NULL;
	int 			found=0, connected=0;

	// Receive data from sock
	if( (recvcount = Network->recvbuf( sock, (char *)&packet, len, &ipadr))>0)
	{
		// Check the command of the packet
		cmd = packet.getCommand();
		const char * buf = packet.getData();

		switch( cmd)
		{
			case CMD_LOGIN :
				strcpy( name, buf);
				strcpy( passwd, buf+NAMELEN);

				for ( VI j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						if( elem->isConnected())
							connected = 1;
						else
							found = 1;
					}
				}
				if( !found && !connected)
				{
					this->sendUnauthorized( sock, elem);
				}
				else
				{
					if( connected)
						this->sendAlreadyConnected( sock, elem);
					else
					{
						this->sendAuthorized( sock, elem);
						elem->setConnected( true);
					}
				}
			break;
			case CMD_LOGOUT :
				// Receive logout request containing name of player
				for ( VI j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
				{
					elem = *j;
					if( !elem->compareName( name) && !elem->comparePass( passwd))
					{
						if( elem->isConnected())
							connected = 1;
						else
							found = 1;
					}
				}
				if( !found)
				{
					cout<<"ERROR LOGOUT -> didn't find player to disconnect"<<endl;
				}
				else
				{
					if( connected)
						elem->setConnected( false);
					else
					{
						cout<<"ERROR LOGOUT -> player exists but wasn't connected ?!?!"<<endl;
					}
				}
			break;
			case CMD_NEWCHAR :
				// Should receive the result of the creation of a new char/ship
			break;
			case CMD_NEWSUBSCRIBE :
				// Should receive a new subscription
			break;
			default:
				cout<<"UNKNOWN command "<<cmd<<" ! ";
		}
	}
	else
	{
		cout<<"Received failed"<<endl;
	}
}

void	AccountServer::sendAuthorized( SocketAlt sock, Account * acct)
{
	Packet	packet2;
	// Get a serial for client
	ObjSerial serial = getUniqueSerial();
	cout<<"\tLOGIN REQUEST SUCCESS for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
	// Verify that client already has a ship or if it is a new account
	if( acct->isNew())
	{
		// Send a command to make the client create a new character/ship
		packet2.create( LOGIN_NEW, serial, packet.getData(), packet.getLength(), 1);
		if( Network->sendbuf( sock, (char *) &packet2, packet2.getLength(), NULL) == -1)
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
		packet2.create( LOGIN_ACCEPT, serial, packet.getData(), packet.getLength(), 1);
		if( Network->sendbuf( sock, (char *) &packet2, packet2.getLength(), NULL) == -1)
		{
			cout<<"ERROR sending authorization"<<endl;
			exit( 1);
		}
	}
}

void	AccountServer::sendUnauthorized( SocketAlt sock, Account * acct)
{
	Packet	packet2;

	packet2.create( LOGIN_ERROR, 0, packet.getData(), packet.getLength(), 1);
	//packet2.displayHex();
	//cout<<" done."<<endl;
	Network->sendbuf( sock, (char *) &packet2, packet2.getLength(), NULL);
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<">"<<endl;
}

void	AccountServer::sendAlreadyConnected( SocketAlt sock, Account * acct)
{
	Packet	packet2;

	packet2.create( LOGIN_ALREADY, 0, packet.getData(), packet.getLength(), 1);
	//packet2.displayHex();
	//cout<<" done."<<endl;
	Network->sendbuf( sock, (char *) &packet2, packet2.getLength(), NULL);
	cout<<"\tLOGIN REQUEST FAILED for <"<<acct->name<<">:<"<<acct->passwd<<"> -> ALREADY LOGGED IN"<<endl;
}

void	AccountServer::save()
{
}
