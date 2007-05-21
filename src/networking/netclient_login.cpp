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
#include "networking/lowlevel/vsnet_sockethttp.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"
#include "networking/lowlevel/netui.h"
#include "networking/client.h"
#include "networking/fileutil.h"


std::string global_username;
std::string global_password;
/*************************************************************/
/**** Authenticate the client                             ****/
/*************************************************************/

int		NetClient::authenticate()
{
	COUT << " enter " << __PRETTY_FUNCTION__ << endl;

	Packet	packet2;
	string  str_callsign, str_passwd;
	std::string netbuf;

	// Get the name and password from vegastrike.config
	// Maybe someday use a default Guest account if no callsign or password is provided thus allowing
	// Player to wander but not interact with the universe
	this->callsign=str_callsign = vs_config->getVariable ("player","callsign","");
	this->password=str_passwd = vs_config->getVariable ("player","password","");
        if (global_username.length())
          this->callsign=global_username;
        if (global_password.length())
          this->password=global_password;

	if( str_callsign.length() && str_passwd.length())
	{
		addSimpleString(netbuf,str_callsign);
		addSimpleString(netbuf,str_passwd);
	    COUT << "callsign:   " << str_callsign << endl
	         << " *** passwd: " << str_passwd << endl
	         << " *** buffer: " << netbuf << endl;

		packet2.send( CMD_LOGIN, 0,
                      netbuf.data(), netbuf.length(),
                      SENDRELIABLE, NULL, *this->clt_tcp_sock,
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

vector<string>	&NetClient::loginLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginLoop" << endl;

	Packet	packet2;
	std::string netbuf;

	//memset( buffer, 0, tmplen+1);
	addSimpleString(netbuf, str_callsign);
	addSimpleString(netbuf, str_passwd);

	COUT << "Buffering to send with CMD_LOGIN: " << endl;
	PacketMem m( (char*)netbuf.data(), netbuf.length(), PacketMem::LeaveOwnership );
	m.dump( cerr, 3 );

	packet2.send( CMD_LOGIN, 0,
                  netbuf.data(), netbuf.length(),
                  SENDRELIABLE, NULL, *this->clt_tcp_sock,
                  __FILE__, PSEUDO__LINE__(316) );
	COUT << "Sent login for player <" << str_callsign << ">:<" << str_passwd
		 << ">" << endl
	     << "   - buffer length : " << packet2.getDataLength() << endl
	     << "   - buffer: " << netbuf.data() << endl;
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
			lastsave.push_back( "");
			lastsave.push_back( "!!! NETWORK ERROR : Connection to game server timed out !!!");
			timeout = 1;
		} else if (recv<0) {
			char str[127];
			sprintf(str, "!!! NETWORK ERROR in recieving socket (error number %d)!!!",
#ifdef _WIN32
				WSAGetLastError()
#else
				errno
#endif
				);
			lastsave.push_back( "");
			lastsave.push_back( str);
			timeout = 1;
		} else {
			break;
		}
	}
	COUT<<"End of login loop"<<endl;
	if( lastsave.empty() || lastsave[0]!="")
	{
		this->callsign = str_callsign;
                this->password= str_passwd;
	}
	//cout<<"GLOBALSAVES[0] : " 
	//cout<<"GLOBALSAVES[1] : "<<globalsaves[1]<<endl;
	return lastsave;
}

/*************************************************************/
/**** Login loop : waiting for account server to respond  ****/
/*************************************************************/

vector<string>	&NetClient::loginAcctLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginAcctLoop" << endl;


	std::string netbuf;
        addSimpleChar(netbuf,ACCT_LOGIN_DATA);
	//memset( buffer, 0, tmplen+1);
	addSimpleString(netbuf, str_callsign);
	addSimpleString(netbuf, str_passwd);

	COUT << "Buffering to send with LOGIN_DATA: " <<netbuf<< endl;
	//PacketMem m( netbuf.getData(), netbuf.getDataLength(), PacketMem::LeaveOwnership );
        //	m.dump( cerr, 3 );
        acct_sock->sendstr(netbuf);
        /*
	packet2.send( LOGIN_DATA, 0,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->acct_sock,
                  __FILE__, PSEUDO__LINE__(378) );
        */
	COUT << "Sent ACCOUNT SERVER login for player <" << str_callsign << ">:<" << str_passwd
		 << ">" << endl
	     << "   - buffer length : " << netbuf.length() << endl
	     << "   - buffer: " << netbuf << endl;
	// Now the loop
	int timeout=0, recv=0;
	// int ret=0;

	Packet packet;

	double initial = queryTime();
	double newtime=0;
	double elapsed=0;
	string login_tostr = vs_config->getVariable( "network", "logintimeout", "20" );
	int login_to = atoi( login_tostr.c_str());
	while( !timeout && !recv)
	{
		// If we have no response in "login_to" seconds -> fails
		newtime = queryTime();
		elapsed = newtime-initial;
		//COUT<<elapsed<<" seconds since login request"<<endl;
		if( elapsed > login_to)
		{
			//lastsave.push_back( "");
			COUT << "!!! NETWORK ERROR : Connection to account server timed out !!!" << endl;
			timeout = 1;
			VSExit(1);
		}
		recv = checkAcctMsg( );
		_sock_set.waste_time(0,40000);
		
	}
	COUT<<"End of loginAcct loop"<<endl;
	// globalsaves should be empty otherwise we filled it with an empty string followed by the error message
	if( lastsave.empty() || lastsave[0]!="")
	{
		//this->callsign = str_callsign;
		//savefiles = globalsaves;
		COUT << "Trying to connect to game server..." << endl
             << "\tIP=" << _serverip << ":" << _serverport << endl;
		
	}
	return lastsave;
}

void	NetClient::loginAccept( Packet & p1)
{
	using namespace VSFileSystem;
	NetBuffer netbuf( p1.getData(), p1.getDataLength());

	Packet pckt;
	this->serial = p1.getSerial();
	COUT << ">>> LOGIN ACCEPTED =( serial #" << serial << " )= --------------------------------------" << endl;
	// Should receive player's data (savegame) from server if there is a save
	localSerials.push_back( serial);

	string datestr = netbuf.getString();
	_Universe->current_stardate.InitTrek( datestr);
	cerr << "Stardate initialized"<<endl;
    cerr << "WE ARE ON STARDATE " << datestr << " - converted : "
         <<_Universe->current_stardate.GetFullTrekDate() << endl;
	lastsave.push_back( netbuf.getString());
	lastsave.push_back( netbuf.getString());
	// Set the zone number
	// Get the galaxy file from buffer with relative path to datadir !
	string univfile = netbuf.getString();
	unsigned char * digest=0;
	unsigned short digest_length = netbuf.getShort();
	if (digest_length) {
		digest = netbuf.getBuffer( digest_length );
#ifdef CRYPTO
		cerr<<"Initial system = "<<VSFileSystem::datadir+univfile<<" - File Hash = "<<digest<<endl;
		// Compare to local hash and ask for the good file if we don't have it or bad version
		if( !FileUtil::HashCompare( univfile, digest, UniverseFile))
		{
			VsnetDownload::Client::NoteFile f( this->clt_tcp_sock, univfile, VSFileSystem::UniverseFile);
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
#endif
	}

	// Get the initial system file...
	string sysfile = netbuf.getString();
	bool autogen;
	string fullsys = VSFileSystem::GetCorrectStarSysPath(sysfile, autogen);
	digest_length = netbuf.getShort();
	if (digest_length) {
		digest = netbuf.getBuffer( digest_length );
#ifdef CRYPTO
		cerr<<"Initial system = "<<fullsys<<" - File Hash = "<<digest<<endl;
		if( !FileUtil::HashCompare( fullsys, digest, SystemFile))
		{
			VsnetDownload::Client::NoteFile f( this->clt_tcp_sock, sysfile, VSFileSystem::SystemFile);
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
#endif
	}
    this->zone = netbuf.getShort();
}

void NetClient::respawnRequest( )
{
	Packet packet2;
	NetBuffer netbuf;
	// No data.
	packet2.send( CMD_RESPAWN, 0,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, *this->clt_tcp_sock,
                  __FILE__, PSEUDO__LINE__(165) );
}
void NetClient::textMessage(const std::string & data )
{
	Packet packet2;
	NetBuffer netbuf;
        netbuf.addString(data);
	// No data.
	packet2.send( CMD_TXTMESSAGE, 0,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, *this->clt_tcp_sock,
                  __FILE__, PSEUDO__LINE__(165) );
}

void NetClient::GetConfigServerAddress( string &addr, unsigned short &port)
{
	int port_tmp;
	string srvport = vs_config->getVariable("network","server_port", "6777");
	port_tmp = atoi( srvport.c_str());
	if (port_tmp>65535||port_tmp<0)
		port_tmp=0;
	port=(unsigned short)port_tmp;
	
	addr = vs_config->getVariable("network","server_ip","");
	this->_serverip=addr;
	this->_serverport=srvport;
	cout<<endl<<"Server IP : "<<addr<<" - port : "<<srvport<<endl<<endl;
}

/*************************************************************/
/**** Initialize the client network to account server     ****/
/*************************************************************/
VsnetHTTPSocket*	NetClient::init_acct( const std::string& addr)
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << endl;

    _sock_set.start( );

	cout<<"Initializing connection to account server..."<<endl;
	acct_sock = new VsnetHTTPSocket( addr, _sock_set );
	COUT <<"accountserver on socket "<<acct_sock<<" done."<<endl;

	return acct_sock;
}

/*************************************************************/
/**** Initialize the client network                       ****/
/*************************************************************/

SOCKETALT	NetClient::init( const char* addr, unsigned short port )
{
	if (addr==NULL) {
		addr=_serverip.c_str();
		port=atoi(_serverport.c_str());
	}
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

    _sock_set.start( );

	string strnetatom;
	strnetatom = vs_config->getVariable( "network", "network_atom", "");
	if( strnetatom=="")
		NETWORK_ATOM = 0.2;
	else
		NETWORK_ATOM = (double) atof( strnetatom.c_str());
	
	*this->clt_tcp_sock = NetUITCP::createSocket( addr, port, _sock_set );
	this->lossy_socket = this->clt_tcp_sock;

	COUT << "created TCP socket (" << addr << "," << port << ") -> " << this->clt_tcp_sock << endl;

	/*
	if( this->authenticate() == -1)
	{
		perror( "Error login in ");
		return -1;
	}
	*/

	this->enabled = 1;
	return *this->clt_tcp_sock;
}

/*************************************************************/
/**** Synchronize server time and client time             ****/
/*************************************************************/
/**** This function creates the UDP socket and determines ****/
/**** whether to use the UDP or the TCP socket for lossy  ****/
/**** packet data.                                        ****/
/*************************************************************/

#define NUM_TIMES 10 // Number of times to send back and forth and obtain average.
// NETFIXME: Correctly obtain ping time.
#include "vs_random.h" // For random ping time.

void NetClient::synchronizeTime(SOCKETALT*udpsock,Cockpit *cp)
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
	int clt_port_read = XMLSupport::parse_int(vs_config->getVariable( "network", "udp_listen_port", "6778" ));
	if (clt_port_read>65535||clt_port_read<0)
		clt_port_read=0;
	unsigned short clt_port=(unsigned short)clt_port_read;

	string nettransport;
	nettransport = vs_config->getVariable( "network", "transport", "udp" );

	//std::string addr;
	unsigned short port=atoi(this->_serverport.c_str());
	//getConfigServerAddress(addr, port);
	
	if (!(udpsock!=NULL&&udpsock->setRemoteAddress(NetUIBase::lookupHost(this->_serverip.c_str(), port))))
          *this->clt_udp_sock=NetUIUDP::createSocket( this->_serverip.c_str(), port, clt_port, _sock_set );
        else
          this->clt_udp_sock=udpsock;
	COUT << "created UDP socket (" << this->_serverip << "," << port << ", listen on " << clt_port << ") -> " << this->clt_udp_sock << endl;
	
	if (nettransport=="udp") {
		// NETFIXME:  Keep trying ports until a connection is established.
		COUT << "Default lossy transport configured to UDP." << endl;
		this->lossy_socket=clt_udp_sock;
	} else {
		COUT << "Default lossy transport configured to TCP (behind firewall)." << endl;
		this->lossy_socket=clt_tcp_sock;
		clt_port=0;
	}
	
	this->clt_tcp_sock->set_block();
	this->clt_udp_sock->set_block();
	
	// Wait for NUM_TIMES (10) successful tries, or 10 consecutive 1-second timeouts
	// (we use UDP on the response (SENDANDFORGET) to improve timing accuracy).
	while (i<NUM_TIMES&&timeout<10) {
		Packet packet;
		NetBuffer outData;
		outData.addShort(clt_port);
		packet.send( CMD_SERVERTIME, 0,
					 outData.getData(), outData.getDataLength(), // No data.
					 SENDRELIABLE, NULL,* this->clt_tcp_sock,
					 __FILE__, PSEUDO__LINE__(343) );
		recv=this->recvMsg( &packet, &tv );
		// If we have no response.
		if (recv<=0) {
			COUT << "synchronizeTime() Timed out" << endl;
			++timeout;
			if (timeout>=10&&this->lossy_socket->isTcp()==false) {
				// no UDP requests made it, fallback to TCP.
				this->lossy_socket=this->clt_tcp_sock;
				clt_port=0;
				timeout=0;
				COUT << "Setting default lossy transport to TCP (UDP timeout)." << endl;
				// NETFIXME: We may want to try different UDP ports to allow multiple people behind one firewall.  In that case, keep falling back to a different port, and make sure to set_nonblock or set_block on each one.
			}
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
	this->clt_tcp_sock->set_nonblock();
	this->clt_udp_sock->set_nonblock();

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
        cur_time=newTime;
        cp->TimeOfLastCollision=cur_time;
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

