#include "networking/lowlevel/vsnet_debug.h"
#include "cmd/unit_generic.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "networking/netclient.h"
#include "savegame.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/lowlevel/packet.h"
#include "lin_time.h"
#include "networking/lowlevel/vsnet_notify.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"
#include "networking/lowlevel/netui.h"
#include "networking/client.h"
#include "networking/fileutil.h"

vector<string> globalsaves;

/*************************************************************/
/**** Authenticate the client                             ****/
/*************************************************************/

int		NetClient::authenticate()
{
	COUT << " enter " << __PRETTY_FUNCTION__ << endl;

	Packet	packet2;
	string  str_callsign, str_passwd;
	NetBuffer netbuf;

	// Get the name and password from vegastrike.config
	// Maybe someday use a default Guest account if no callsign or password is provided thus allowing
	// Player to wander but not interact with the universe
	str_callsign = vs_config->getVariable ("player","callsign","");
	str_passwd = vs_config->getVariable ("player","password","");
	if( str_callsign.length() && str_passwd.length())
	{
	    COUT << "callsign:   " << str_callsign << endl
	         << " *** passwd: " << str_passwd << endl
	         << " *** buffer: " << netbuf.getData() << endl;
		netbuf.addString( str_callsign);
		netbuf.addString( str_passwd);

		packet2.send( CMD_LOGIN, 0,
                      netbuf.getData(), netbuf.getDataLength(),
                      SENDRELIABLE, NULL, this->clt_sock,
                      __FILE__, PSEUDO__LINE__(165) );
		COUT << "Send login for player <" << str_callsign << ">:< "<< str_passwd
		     << "> - buffer length : " << packet2.getDataLength()
             << " (+" << packet2.getHeaderLength() << " header len" <<endl;
	}
	else
	{
		cerr<<"Callsign and/or password not specified in vegastrike.config, please check this."<<endl<<endl;
		return -1;
	}

	return 0;
}

/*************************************************************/
/**** Login loop : waiting for game server to respond     ****/
/*************************************************************/

vector<string>	NetClient::loginLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginLoop" << endl;

	Packet	packet2;
	NetBuffer netbuf;

	//memset( buffer, 0, tmplen+1);
	char flags;
    if( canCompress() ) flags |= CMD_CAN_COMPRESS;
    netbuf.addChar( flags );
	netbuf.addString( str_callsign);
	netbuf.addString( str_passwd);

	COUT << "Buffering to send with CMD_LOGIN: " << endl;
	PacketMem m( netbuf.getData(), netbuf.getDataLength(), PacketMem::LeaveOwnership );
	m.dump( cerr, 3 );

	packet2.send( CMD_LOGIN, 0,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->clt_sock,
                  __FILE__, PSEUDO__LINE__(316) );
	COUT << "Sent login for player <" << str_callsign << ">:<" << str_passwd
		 << ">" << endl
	     << "   - buffer length : " << packet2.getDataLength() << endl
	     << "   - buffer: " << netbuf.getData() << endl
         << " " << (canCompress() ? "(compress)" : "(no compress)") <<endl;
	// Now the loop
	int timeout=0, recv=0;
	// int ret=0;
	UpdateTime();

	Packet packet;

	double initial = getNewTime();
	double newtime=0;
	double elapsed=0;
	string login_tostr = vs_config->getVariable( "network", "logintimeout", "10" );
	int login_to = atoi( login_tostr.c_str());
	while( !timeout && !recv)
	{
		// If we have no response in "login_to" seconds -> fails
		UpdateTime();
		newtime = getNewTime();
		elapsed = newtime-initial;
		//COUT<<elapsed<<" seconds since login request"<<endl;
		if( elapsed > login_to)
		{
			globalsaves.push_back( "");
			globalsaves.push_back( "!!! NETWORK ERROR : Connection to game server timed out !!!");
			timeout = 1;
		}
		recv=this->checkMsg( &packet );

		micro_sleep( 40000);
	}
	COUT<<"End of login loop"<<endl;
	if( globalsaves.empty() || globalsaves[0]!="")
	{
		this->callsign = str_callsign;
	}
	//cout<<"GLOBALSAVES[0] : "<<globalsaves[0]<<endl;
	//cout<<"GLOBALSAVES[1] : "<<globalsaves[1]<<endl;
	return globalsaves;
}

/*************************************************************/
/**** Login loop : waiting for account server to respond  ****/
/*************************************************************/

vector<string>	NetClient::loginAcctLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginAcctLoop" << endl;

	Packet	packet2;
	NetBuffer netbuf;

	//memset( buffer, 0, tmplen+1);
	netbuf.addString( str_callsign);
	netbuf.addString( str_passwd);

	COUT << "Buffering to send with LOGIN_DATA: " << endl;
	PacketMem m( netbuf.getData(), netbuf.getDataLength(), PacketMem::LeaveOwnership );
	m.dump( cerr, 3 );

	packet2.send( LOGIN_DATA, 0,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->acct_sock,
                  __FILE__, PSEUDO__LINE__(378) );
	COUT << "Sent ACCOUNT SERVER login for player <" << str_callsign << ">:<" << str_passwd
		 << ">" << endl
	     << "   - buffer length : " << packet2.getDataLength() << endl
	     << "   - buffer: " << netbuf.getData() << endl;
	// Now the loop
	int timeout=0, recv=0;
	// int ret=0;
	UpdateTime();

	Packet packet;

	double initial = getNewTime();
	double newtime=0;
	double elapsed=0;
	string login_tostr = vs_config->getVariable( "network", "logintimeout", "10" );
	int login_to = atoi( login_tostr.c_str());
	while( !timeout && !recv)
	{
		// If we have no response in "login_to" seconds -> fails
		UpdateTime();
		newtime = getNewTime();
		elapsed = newtime-initial;
		//COUT<<elapsed<<" seconds since login request"<<endl;
		if( elapsed > login_to)
		{
			globalsaves.push_back( "");
			globalsaves.push_back( "!!! NETWORK ERROR : Connection to account server timed out !!!");
			timeout = 1;
		}

		recv = checkAcctMsg( );

		micro_sleep( 40000);
	}
	COUT<<"End of loginAcct loop"<<endl;
	// globalsaves should be empty otherwise we filled it with an empty string followed by the error message
	if( globalsaves.empty() || globalsaves[0]!="")
	{
		//this->callsign = str_callsign;
		//savefiles = globalsaves;
		COUT << "Trying to connect to game server..." << endl
             << "\tIP=" << _serverip << ":" << _serverport << endl;
		this->init( _serverip.c_str(), atoi( _serverport.c_str()));
	}
	return globalsaves;
}

void	NetClient::loginAccept( Packet & p1)
{
	using namespace VSFileSystem;
	NetBuffer netbuf( p1.getData(), p1.getDataLength());

	Packet pckt;
	this->serial = p1.getSerial();
	COUT << ">>> LOGIN ACCEPTED =( serial n°" << serial << " )= --------------------------------------" << endl;
	// Should receive player's data (savegame) from server if there is a save
	localSerials.push_back( serial);

	string datestr = netbuf.getString();
	_Universe->current_stardate.InitTrek( datestr);
	cerr << "Stardate initialized"<<endl;
    cerr << "WE ARE ON STARDATE " << datestr << " - converted : "
         <<_Universe->current_stardate.GetFullTrekDate() << endl;
	globalsaves.push_back( netbuf.getString());
	globalsaves.push_back( netbuf.getString());
	// Get the galaxy file from buffer with relative path to datadir !
	string univfile = netbuf.getString();
	unsigned char * digest;
#ifdef CRYPTO
	digest = netbuf.getBuffer( FileUtil::Hash.DigestSize());
	cerr<<"Initial system = "<<VSFileSystem::datadir+univfile<<" - File Hash = "<<digest<<endl;
#endif
	// Compare to local hash and ask for the good file if we don't have it or bad version
	if( !FileUtil::HashCompare( univfile, digest, UniverseFile))
	{
		VsnetDownload::Client::NoteFile f( this->clt_sock, univfile, VSFileSystem::UniverseFile);
		_downloadManagerClient->addItem( &f);
		while( !f.done())
		{
			checkMsg( NULL);
			micro_sleep( 40000);
		}
	}

	// Get the initial system file...
	string sysfile = netbuf.getString();
	bool autogen;
	string fullsys = VSFileSystem::GetCorrectStarSysPath(sysfile, autogen);
#ifdef CRYPTO
	digest = netbuf.getBuffer( FileUtil::Hash.DigestSize());
	cerr<<"Initial system = "<<fullsys<<" - File Hash = "<<digest<<endl;
#endif
	if( !FileUtil::HashCompare( fullsys, digest, SystemFile))
	{
		VsnetDownload::Client::NoteFile f( this->clt_sock, sysfile, VSFileSystem::SystemFile);
		_downloadManagerClient->addItem( &f);
		while( !f.done())
		{
			checkMsg( NULL);
			micro_sleep( 40000);
		}
	}
}

/*************************************************************/
/**** Initialize the client network to account server     ****/
/*************************************************************/

SOCKETALT	NetClient::init_acct( char * addr, unsigned short port)
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

    _sock_set.start( );

	cout<<"Initializing connection to account server..."<<endl;
	acct_sock = NetUITCP::createSocket( addr, port, _sock_set );
	COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;

	return acct_sock;
}

/*************************************************************/
/**** Initialize the client network                       ****/
/*************************************************************/

SOCKETALT	NetClient::init( const char* addr, unsigned short port )
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

    _sock_set.start( );

	string strnetatom;
	strnetatom = vs_config->getVariable( "network", "network_atom", "");
	if( strnetatom=="")
		NETWORK_ATOM = 0.2;
	else
		NETWORK_ATOM = (double) atoi( strnetatom.c_str());

	string nettransport;
	nettransport = vs_config->getVariable( "network", "transport", "udp" );
	if( nettransport == "tcp" )
	{
	    this->clt_sock = NetUITCP::createSocket( addr, port, _sock_set );
	}
	else
	{
	    this->clt_sock = NetUIUDP::createSocket( addr, port, _sock_set );
	}
	COUT << "created " << (this->clt_sock.isTcp() ? "TCP" : "UDP")
	     << "socket (" << addr << "," << port << ") -> " << this->clt_sock << endl;

	/*
	if( this->authenticate() == -1)
	{
		perror( "Error login in ");
		return -1;
	}
	*/

	this->enabled = 1;
	return this->clt_sock;
}

/*************************************************************/
/**** Receive that start locations                        ****/
/*************************************************************/

// Receives possible start locations (first a short representing number of locations)
// Then for each number, a desc

void NetClient::receiveLocations( const Packet* )
{
	unsigned char	cmd;

#ifdef __DEBUG__
	COUT<<"Nb start locations : "<<nblocs<<endl;
#endif
	// Choose starting location here

	// Send the chosen location to the server
	cmd = CMD_ADDCLIENT;
}

/*************************************************************/
/**** Create a new character                              ****/
/*************************************************************/

void	NetClient::createChar()
{
}

