#include "networking/client.h"
#include "networking/acctserver.h"
#include "networking/lowlevel/packet.h"
#include "lin_time.h"
#include "networking/lowlevel/vsnet_serversocket.h"
#include "vs_path.h"
#include "vs_globals.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/fileutil.h"
#include "posh.h"

VegaConfig * vs_config;
string acctdir;

AccountServer::AccountServer()
    : _sock_set( true )
{
	cout<<endl<<POSH_GetArchString()<<endl;
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


AccountServer::~AccountServer()
{
    delete Network;
}

/**************************************************************/
/**** Display info on the server at startup                ****/
/**************************************************************/

void    AccountServer::startMsg()
{
    cout<<endl<<"Vegastrike Account Server version 0.0.1"<<endl;
    cout<<"Written by Stephane Vaxelaire"<<endl<<endl;
}

void    AccountServer::start()
{
    string strperiod;
    keeprun = 1;

    SOCKETALT   comsock;

    startMsg();

    CONFIGFILE = new char[42];
    strcpy( CONFIGFILE, "accountserver.config");
    cout<<"Loading config file...";
    initpaths();
    //vs_config = new VegaConfig( ACCTCONFIGFILE);
    cout<<" done."<<endl;
    InitTime();
    UpdateTime();
    acctdir = datadir + vs_config->getVariable( "server", "accounts_dir", "");
    if( acctdir=="")
        acctdir = datadir + "/accounts/";
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
		VSExit(1);
    }
    cout<<Cltacct.size()<<" accounts loaded."<<endl;

    // Create and bind socket
    cout<<"Initializing network..."<<endl;
    unsigned short tmpport;

    if( vs_config->getVariable( "network", "accountsrvport", "")=="")
        tmpport = ACCT_PORT;
    else
        tmpport = atoi((vs_config->getVariable( "network", "accountsrvport", "")).c_str());
    Network = NetworkToClient.createServerSocket( tmpport, _sock_set );
    if( !Network)
    {
        cout<<"Error cannot start server... quitting."<<endl;
        VSExit(1);
    }
    cout<<"done."<<endl;

    _sock_set.start( );

    while( keeprun)
    {
        //COUT << "Loop" << endl;
        // Check for incoming connections

        _sock_set.wait( );

        comsock = Network->acceptNewConn( );
        if( comsock.valid() )
        {
            Socks.push_back( comsock);
            COUT << "New connection - socket allocated : "<<comsock<<endl;
        }

        // Loop for each active client and process request
        LS i;
        for( i=Socks.begin(); i!=Socks.end(); i++)
        {
            if( i->isActive( ) )
            {
                this->recvMsg( (*i));
            }
        }

        // Remove dead connections
        this->removeDeadSockets();
        // Check for automatic server status save time
        curtime = getNewTime();

        // micro_sleep(40000);
    }

    delete CONFIGFILE;
    delete vs_config;
    Network->disconnect( "Shutting down.", true );
}

void    AccountServer::recvMsg( SOCKETALT sock)
{
    string callsign, passwd;
    AddressIP       ipadr;
    int             recvcount=0;
    unsigned char   cmd;
    Account *       elem = NULL;
    int             found=0, connected=0;

    // Receive data from sock
    PacketMem mem;
    if( (recvcount = sock.recvbuf( mem, &ipadr))>0)
    {
        cout<<"Socket : "<<endl<<sock<<endl;
        //Packet p( buffer, recvcount );
        Packet p( mem);

        packet = p;
        NetBuffer netbuf( p.getData(), p.getDataLength());
        // Check the command of the packet
        cmd = packet.getCommand();
        cout<<"Buffer => "<<p.getData()<<endl;
        VI j;
        switch( cmd)
        {
            case LOGIN_DATA :
                // We receive a request from a client that wants to know to which game server he has to connect to
                callsign = netbuf.getString();
                passwd = netbuf.getString();
                cout<<">>> SERVER REQUEST =( "<<callsign<<":"<<passwd<<" )= --------------------------------------"<<endl;

                for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
                {
                    elem = *j;
                    if( !elem->compareName( callsign) && !elem->comparePass( passwd))
                    {
                        // We found the client in the account list
                        found = 1;
                        cout<<"Found player : "<<elem->callsign<<":"<<elem->passwd<<endl;
                    }
                }
                if( !found)
                {
                    COUT<<"Login/passwd not found"<<endl;
                    Account elt(callsign, passwd);
                    this->sendUnauthorized( sock, &elt);
                }
                else
                {
                    if( elem->isConnected())
                    {
                        COUT<<"Login already connected !"<<endl;
                        this->sendAlreadyConnected( sock, elem);
                    }
                    else
                    {
                        COUT<<"Login accepted send server ip !"<<endl;
                        // Send a packet with server IP Address
                        this->sendServerData( sock, elem);
                        //elem->setConnected( true);
                    }
                }
            break;
            case CMD_LOGIN :
                callsign = netbuf.getString();
                passwd = netbuf.getString();
                cout<<">>> LOGIN REQUEST =( "<<callsign<<":"<<passwd<<" )= --------------------------------------"<<endl;

                for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
                {
                    elem = *j;
                    if( !elem->compareName( callsign) && !elem->comparePass( passwd))
                    {
                        // We found the client in the account list
                        found = 1;
                        cout<<"Found player : "<<elem->callsign<<":"<<elem->passwd<<endl;
                    }
                }
                if( !found)
                {
#ifdef VSNET_DEBUG
                    COUT << "Login/passwd not found, we know:" << endl;
                    for( j=Cltacct.begin(); j!=Cltacct.end(); j++ )
		            {
                        COUT << "    " << (*j)->callsign << ":" << (*j)->passwd << endl;
		            }
#else
                    COUT<<"Login/passwd not found"<<endl;
#endif
                    Account elt(callsign, passwd);
                    this->sendUnauthorized( sock, &elt);
                }
                else
                {
                    if( elem->isConnected())
                    {
                        cout<<"Client already connected"<<endl;
                        this->sendUnauthorized( sock, elem);
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
                callsign = netbuf.getString();
                passwd = netbuf.getString();
                cout<<">>> LOGOUT REQUEST =( "<<callsign<<":"<<passwd<<" )= --------------------------------------"<<endl;
                // Receive logout request containing name of player
                for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
                {
                    elem = *j;
                    if( !elem->compareName( callsign) && !elem->comparePass( passwd))
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
                    cout<<"ERROR LOGOUT -> didn't find player to disconnect = <"<<callsign<<">:<"<<passwd<<">"<<endl;
                }
                else
                {
                    if( connected)
                    {
                        elem->setConnected( false);
                        cout<<"-= "<<callsign<<" =- Disconnected"<<endl;
                    }
                    else
                    {
                        cout<<"ERROR LOGOUT -> player exists but wasn't connected ?!?!"<<endl;
                    }
                }
                cout<<"<<< LOGOUT REQUEST ---------------------------------------"<<endl;
            break;
            case CMD_NEWCHAR :
                cout<<">>> NEW CHAR REQUEST =( "<<callsign<<" )= --------------------------------------"<<endl;
                // Should receive the result of the creation of a new char/ship
                cout<<"<<< NEW CHAR REQUEST -------------------------------------------------------"<<endl;
            break;
            case CMD_NEWSUBSCRIBE :
            {
                cout<<">>> SUBSRIBE REQUEST =( "<<callsign<<" )= --------------------------------------"<<endl;
                // Should receive a new subscription
                callsign = netbuf.getString();
                passwd = netbuf.getString();
                // Loop through accounts to see if the required callsign already exists
                bool found = false;
                Packet  packet2;
                for (  j=Cltacct.begin(); j!=Cltacct.end() && !found && !connected; j++)
                {
                    elem = *j;
                    if( !elem->compareName( callsign))
                    {
                        // We found an account with the requested name
                        found = true;
                        if( packet2.send( LOGIN_ERROR, packet.getSerial(), (char *)NULL, 0, SENDRELIABLE, NULL, sock, __FILE__, __LINE__ ) < 0 )
                        {
                            cout<<"ERROR sending authorization"<<endl;
                            exit( 1);
                        }
                    }
                }
                if( !found)
                {
                    // Add the account at the end of accounts.xml
                    string acctpath = datadir+"/accounts.xml";
                    FILE * fp = fopen( acctpath.c_str(), "r+b");
                    if( fp==NULL)
                    {
                        cout<<"ERROR opening accounts file";
                        if( packet2.send( (Cmd) 0, packet.getSerial(), (char *)NULL, 0, SENDRELIABLE, NULL, sock, __FILE__, __LINE__ ) < 0 )
                            cout<<"ERROR sending errormsg to subscription website"<<endl;
                        VSExit(1);
                    }
                    else
                    {
                        cout<<"Account file opened"<<endl;
                        //fseek( fp, 0, SEEK_SET);
                        char * fbuf = new char[MAXBUFFER];
                        size_t i=0;
                        vector<string> acctlines;
                        // Read a line per account and one line for the "<ACCOUNTS>" tag
                        for( i=0; i<Cltacct.size()+1; i++)
                        {
                            fbuf=fgets( fbuf, MAXBUFFER, fp);
                            acctlines.push_back( fbuf);
                            cout<<"Read line : "<<fbuf<<endl;
                        }
                        fclose( fp);
                        fp = fopen( acctpath.c_str(), "wb");
                        if( !fp)
                        {
                            cerr<<"!!! ERROR : opening account file in write mode !!!"<<endl;
                            VSExit(1);
                        }
                        acctlines.push_back( "\t<PLAYER name=\""+callsign+"\"\tpassword=\""+passwd+"\" />\n");
                        acctlines.push_back( "</ACCOUNTS>\n");
                        //cout<<"Adding to file : "<<acctstr<<endl;
                        for( i=0; i<acctlines.size(); i++)
                        {
                            if( fputs( acctlines[i].c_str(), fp) < 0)
                            {
                                cout<<"!!! ERROR : writing to account file !!!"<<endl;
                                VSExit(1);
                            }
                        }
                        /*
                        if( fputs( acctstr.c_str(), fp) < 0)
                        {
                            cout<<"ERROR writing new account to account file"<<endl;
                            VSExit(1);
                        }
                        */
                        fclose( fp);
                        Cltacct.push_back( new Account( callsign, passwd));
                    }
                    if( packet2.send( packet.getCommand(), packet.getSerial(), (char*)NULL, 0, SENDRELIABLE, NULL, sock, __FILE__, __LINE__ ) < 0 )
                    {
                        cout<<"ERROR sending authorization"<<endl;
                        exit( 1);
                    }
                }
                //DeadSocks.push_back( sock);
                cout<<"<<< SUBSRIBE REQUEST --------------------------------------"<<endl;
            }
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
                    sertmp = netbuf.getShort();
                    //sertmp = ntohs( *( (ObjSerial *)(buf+sizeof( ObjSerial)*i)));
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

void    AccountServer::sendAuthorized( SOCKETALT sock, Account * acct)
{
    // Get a serial for client
    //ObjSerial serial = getUniqueSerial();
    ObjSerial serial = 0;
    acct->setSerial( serial);
    cout<<"\tLOGIN REQUEST SUCCESS for <"<<acct->callsign<<">:<"<<acct->passwd<<">"<<endl;
    // Store socket as a game server id
    acct->setSocket( sock);
    // Verify that client already has a ship or if it is a new account
    if( acct->isNew())
    {
        // Send a command to make the client create a new character/ship
        Packet  packet2;
        if( packet2.send( LOGIN_NEW, serial, packet.getData(), packet.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ ) < 0 )
        {
            cout<<"ERROR sending authorization"<<endl;
            exit( 1);
        }
        // Should receive an answer from game server that contains ship's creation info to
        // be saved on the account server
    }
    else
    {
        NetBuffer netbuf;
        unsigned int readsize=0, readsize2=0, xmlsize=0, savesize=0;

    // Try to open save file
        string acctfile = acctdir+acct->callsign+".save";
        cerr<<"Trying to open : "<<acctfile<<endl;
        FILE *fp = fopen( acctfile.c_str(), "rb");
        if( fp == NULL)
        {
            cerr<<"Account save file does not exists... sending default one to game server"<<endl;
            acctfile = acctdir+"default.save";
            cerr<<"Trying to open : "<<acctfile<<endl;
            fp = fopen( acctfile.c_str(), "rb");
        }
        else
            cout<<"... done !"<<endl;
    // Try to open xml file
        string acctsave;
        // If we loaded default save -> we must load default xml
        if( acctfile==acctdir+"default.save")
            acctsave = acctdir+"default.xml";
        acctsave = acctdir+acct->callsign+".xml";
        cerr<<"Trying to open : "<<acctsave<<endl;
        FILE * fp2 = fopen( acctsave.c_str(), "rb");
        if( fp2 == NULL)
        {
            cout<<"XML save file does not exists... sending default one to game server"<<endl;
            acctsave = acctdir+"default.xml";
            cerr<<"Trying to open : "<<acctsave<<endl;
            if( acctfile!=acctdir+"default.save")
            {
                // We loaded an existing save but no corresponding xml file so load default
                fclose( fp);
                cerr<<"Default XML Loaded -> reload the default save !"<<endl;
                acctfile = acctdir+"default.save";
                cerr<<"Trying to open : "<<acctfile<<endl;
                fp = fopen( acctfile.c_str(), "rb");
            }
            fp2 = fopen( acctsave.c_str(), "rb");
        }
        else
            cout<<"... done !"<<endl;
    // Allocate the needed buffer
        char * savebuf;
        char * xmlbuf;
        if( fp!=NULL && fp2!=NULL)
        {
            fseek( fp, 0, SEEK_END);
            savesize = ftell( fp);
            fseek( fp, 0, SEEK_SET);
            fseek( fp2, 0, SEEK_END);
            xmlsize = ftell( fp2);
            fseek( fp2, 0, SEEK_SET);
            savebuf = new char[savesize];
            xmlbuf = new char[xmlsize];
        }
    // Read the save unit file
        if( fp!=NULL)
        {
            readsize = fread( savebuf, sizeof( char), savesize, fp);
            if( readsize!=savesize)
            {
                cout<<"Error reading save file : "<<readsize<<" read ("<<savesize<<" to read)"<<endl;
                exit( 1);
            }
            fclose( fp);
        }
        else
        {
            cout<<"Error, default save not found"<<endl;
			VSExit(1);
        }
        // Put the name and passwd of the player in the packet
        netbuf.addString( acct->callsign);
        netbuf.addString( acct->passwd);
        netbuf.addString( acct->serverip);
        netbuf.addString( acct->serverport);
        // Put the size of the first save file in the buffer to send
        netbuf.addString( string(savebuf));

    // Read the XML file
        if( fp2!=NULL)
        {
            // Read the XML unit file
            readsize2 = fread( xmlbuf, sizeof( char), xmlsize, fp2);
            if( xmlsize!=readsize2)
            {
                cout<<"Error reading xml save file : "<<readsize2<<" read ("<<xmlsize<<" to read)"<<endl;
                exit( 1);
            }
            fclose( fp2);
        }
        else
        {
            cout<<"Error, default xml not found"<<endl;
			VSExit(1);
        }
        netbuf.addString( string( xmlbuf));
        cout<<"Save size = "<<readsize<<" - XML size = "<<readsize2<<endl;
        cout<<"Loaded -= "<<acct->callsign<<" =- save files ("<<(readsize+readsize2)<<")"<<endl;

        Packet  packet2;
        if( packet2.send( LOGIN_ACCEPT, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE|COMPRESSED, NULL, sock, __FILE__, __LINE__ ) < 0 )
        {
            cout<<"ERROR sending authorization"<<endl;
            exit( 1);
        }
    }
}

void    AccountServer::sendUnauthorized( SOCKETALT sock, Account * acct)
{
    Packet  packet2;
    packet2.send( LOGIN_ERROR, 0, packet.getData(), packet.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ );
    cout<<"\tLOGIN REQUEST FAILED for <"<<acct->callsign<<">:<"<<acct->passwd<<">"<<endl;
}

void    AccountServer::sendAlreadyConnected( SOCKETALT sock, Account * acct)
{
    Packet  packet2;
    packet2.send( LOGIN_ALREADY, acct->getSerial(), packet.getData(), packet.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ );
    cout<<"\tLOGIN REQUEST FAILED for <"<<acct->callsign<<">:<"<<acct->passwd<<"> -> ALREADY LOGGED IN"<<endl;
}

void    AccountServer::sendServerData( SOCKETALT sock, Account * acct)
{
    Packet  packet2;
    NetBuffer netbuf;
    netbuf.addString( acct->serverip);
    netbuf.addString( acct->serverport);
    cout<<"-- sending SERVERIP="<<acct->serverip<<" - SERVERPORT="<<acct->serverport<<endl;
    packet2.send( LOGIN_DATA, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, sock, __FILE__, __LINE__ );
    cout<<"\tLOGIN DATA SENT for <"<<acct->callsign<<">:<"<<acct->passwd<<">"<<endl;
}


void    AccountServer::save()
{
    // Loop through all accounts and write their status
    VI vi;
    for( vi=Cltacct.begin(); vi!=Cltacct.end(); vi++)
    {
    }
}

void    AccountServer::removeDeadSockets()
{
    if( DeadSocks.empty() ) return;

    VI vi;
    int nbc_disc = 0;
    int nbs_disc = 0;
    for (LS j=DeadSocks.begin(); j!=DeadSocks.end(); j++)
    {
        bool found=false;
        COUT << ">>>>>>> Closing socket number "<<(*j)<<endl;
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
                found=true;
            }
        }
        if( !found)
            nbs_disc++;
        j->disconnect( "\tclosing socket", false );
        Socks.remove( (*j));
    }
    cout<<"\tDisconnected "<<nbc_disc<<" clients associated with that server socket"<<endl;
    cout<<"\tDisconnected "<<nbs_disc<<" non-clients sockets"<<endl;
    DeadSocks.clear();
}

void    AccountServer::writeSave( const char * buffer)
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
        FileUtil::WriteSaveFiles( savestr, xmlstr, acctdir, (*vi)->callsign);
}
