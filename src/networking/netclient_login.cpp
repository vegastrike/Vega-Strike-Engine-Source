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
	char flags = 0;
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
	     << "   - buffer: " << netbuf.getData() << endl;
	// Now the loop
	int timeout=0, recv=0;
	// int ret=0;

	Packet packet;

	string login_tostr = vs_config->getVariable( "network", "logintimeout", "10" );
	timeval tv = {atoi( login_tostr.c_str()), 0};
	
	while( !timeout )
	{
		recv=this->recvMsg( &packet, &tv );
		if( recv==0 )
		{
			globalsaves.push_back( "");
			globalsaves.push_back( "!!! NETWORK ERROR : Connection to game server timed out !!!");
			timeout = 1;
		} else if (recv<0) {
			globalsaves.push_back( "");
			globalsaves.push_back( "!!! NETWORK ERROR in recieving socket.");
			timeout = 1;
		} else {
			break;
		}
	}
	COUT<<"End of login loop"<<endl;
	if( globalsaves.empty() || globalsaves[0]!="")
	{
		this->callsign = str_callsign;
	}
	//cout<<"GLOBALSAVES[0] : " 
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

	Packet packet;

	double initial = queryTime();
	double newtime=0;
	double elapsed=0;
	string login_tostr = vs_config->getVariable( "network", "logintimeout", "10" );
	int login_to = atoi( login_tostr.c_str());
	while( !timeout && !recv)
	{
		// If we have no response in "login_to" seconds -> fails
		newtime = queryTime();
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
	// Set the zone number
	// Get the galaxy file from buffer with relative path to datadir !
	string univfile = netbuf.getString();
	unsigned char * digest=0;
#ifdef CRYPTO
	digest = netbuf.getBuffer( FileUtil::Hash.DigestSize());
	cerr<<"Initial system = "<<VSFileSystem::datadir+univfile<<" - File Hash = "<<digest<<endl;
#endif
	// Compare to local hash and ask for the good file if we don't have it or bad version
	if( !FileUtil::HashCompare( univfile, digest, UniverseFile))
	{
		VsnetDownload::Client::NoteFile f( this->clt_sock, univfile, VSFileSystem::UniverseFile);
		_downloadManagerClient->addItem( &f);
		timeval timeout={10,0};
		while( !f.done())
		{
			if (recvMsg( NULL, &timeout )<=0) {
//NETFIXME: What to do if the download times out?
				break;
			}
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
		timeval timeout={10,0};
		while( !f.done())
		{
			if (recvMsg( NULL, &timeout )<=0) {
				//NETFIXME: what to do if timeout elapses...
				break;
			}
		}
	}
    this->zone = netbuf.getShort();
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
		NETWORK_ATOM = (double) atof( strnetatom.c_str());

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
/**** Synchronize server time and client time             ****/
/*************************************************************/

#define NUM_TIMES 10 // Number of times to send back and forth and obtain average.
// NETFIXME: Correctly obtain ping time.
#include "vs_random.h" // For random ping time.

void NetClient::synchronizeTime()
{

	int i=0;
	int timeout=0;
	int recv;
	timeval tv = { 1, 0 }; // Timeout after 1 second, request send again.
	double ping; // use deltaTime?
	double pingavg=0.;
	double timeavg=0.;
	std::map<double, double> times; // sorted container.
	double initialTime=queryTime();
	this->clt_sock.set_block();
	// Wait for NUM_TIMES (10) successful tries, or 10 consecutive 1-second timeouts
	// (we use UDP on the response (SENDANDFORGET) to improve timing accuracy).
	while (i<NUM_TIMES&&timeout<10) {
		Packet packet;
		
		packet.send( CMD_SERVERTIME, 0,
					 0, 0, // No data.
					 SENDRELIABLE, NULL, this->clt_sock,
					 __FILE__, PSEUDO__LINE__(343) );
		recv=this->recvMsg( &packet, &tv );
		// If we have no response.
		if (recv<=0) {
			COUT << "synchronizeTime() Timed out" << endl;
			++timeout;
			// NETFIXME: What should we do in case of CMD_SERVERTIME timeout? Probably lost packet.
		} else if (packet.getCommand() == CMD_SERVERTIME ) {
			// NETFIXME: obtain actual ping time
			//ping = getPingTime( &tv );
			ping = exp(vsrandom.uniformInc(-10, 0));
			if (ping>0&&ping<1.) {
				++i;
				NetBuffer data (packet.getData(), packet.getDataLength());
				double serverTime=data.getDouble();
				double currentTime=queryTime();
				serverTime+=initialTime-currentTime;
				times.insert(std::multimap<double, double>::value_type(ping, serverTime-ping));
				timeout=0;
			} else {
				++timeout;
			}
		}
	}
	this->clt_sock.set_nonblock();
//	std::sort(times[0], times[i]);
	if (i>=10) {
		int mid=i/2;
		double median=0.;
		double tot=0.;
		int location=0;
		std::map<double, double>::const_iterator iter;
		for (iter=times.begin();iter!=times.end();++iter) {
			if (location==mid) {
				median=iter->first;
				if (i%2==1) {
					++iter;
					median+=iter->first;
				}
				break;
			}
			++location;
		}
		if (i%2==1) {
			median/=2;
		}
		for (iter=times.begin();iter!=times.end();++iter) {
			double wdiff=exp(-10*(median-iter->first)*(median-iter->first));
			pingavg+=wdiff*iter->first;
			timeavg+=wdiff*iter->second;
			tot+=wdiff;
		}
		pingavg/=tot;
		timeavg/=tot;
	} else {
		COUT << "Error in time synchronization: connection ended or timed out.";
	}
	this->deltatime=pingavg;
	double newTime=timeavg+queryTime()-initialTime;
	COUT << "Setting time to: New time: " << newTime << endl;
	setNewTime(newTime);
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

