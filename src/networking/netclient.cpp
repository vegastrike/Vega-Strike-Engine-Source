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
  NetClient - Network Client Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#include "netclient.h"
#include "netui.h"

#include <iostream>
#include <stdio.h>
#if !defined(WIN32)
#include <unistd.h>
#endif
#include <config.h>

#include "vs_globals.h"
#include "endianness.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "../config_xml.h"
#include "client.h"
#include "const.h"
#include "packet.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "gfx/matrix.h"
#include "lin_time.h"
#include "vs_path.h"
#include "packet.h"
#include "cmd/role_bitmask.h"
#include "gfx/cockpit_generic.h"

#include "networking/vsnet_clientstate.h"
#include "networking/vsnet_debug.h"
#include "networking/vsnet_dloadmgr.h"
#include "networking/vsnet_notify.h"
#include "vegastrike.h"
#include "client.h"
#include "networking/netbuffer.h"
#include "networking/networkcomm.h"
#include "md5.h"

#ifdef micro_sleep
#undef micro_sleep
#endif
#define micro_sleep(m) _sock_set.waste_time( 0, m )

using std::cout;
using std::endl;
using std::cin;

double NETWORK_ATOM;
vector<string> globalsaves;
extern vector<unorigdest *> pendingjump;
extern Hashtable<std::string, StarSystem, char[127]> star_system_table;
typedef vector<Client *>::iterator VC;

/*************************************************************/
/**** Tool functions                                      ****/
/*************************************************************/

typedef vector<ObjSerial>::iterator ObjI;
vector<ObjSerial>	localSerials;
bool isLocalSerial( ObjSerial sernum)
{
	COUT<<"Looking for serial : "<<sernum<<" in ";
	bool ret=false;
	for( ObjI i=localSerials.begin(); !ret && i!=localSerials.end(); i++)
	{
		COUT<<(*i)<<", ";
		if( sernum==(*i))
			ret = true;
	}
	COUT<<endl;

	return ret;
}

Unit * getNetworkUnit( ObjSerial cserial)
{
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		if( Network[i].getUnit()->GetSerial() == cserial)
			return Network[i].getUnit();
	}
	return NULL;
}

NetClient::NetClient()
    : save("")
{
    game_unit = NULL;
    old_timestamp = 0;
    current_timestamp = 0;
    old_time = 0;
    cur_time = 0;
    enabled = 0;
    nbclients = 0;
	jumpok = false;
	ingame = false;
	current_freq = MIN_COMMFREQ;
	selected_freq = MIN_COMMFREQ;
#ifdef NETCOMM
	NetComm = new NetworkCommunication();
#else
    NetComm = NULL;
#endif

    _downloadManagerClient.reset( new VsnetDownload::Client::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloadManagerClient );

}

NetClient::~NetClient()
{
#ifdef NETCOMM
	if( NetComm!=NULL)
		delete NetComm;
#endif
}

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
/**** Packet recpetion loop                               ****/
/*************************************************************/

bool	NetClient::PacketLoop( Cmd command)
{
	Packet packet;
	bool timeout = false;
	bool recv = false;

	COUT<<"Enter NetClient::PacketLoop"<<endl;

	string packet_tostr = vs_config->getVariable( "network", "packettimeout", "10" );
	int packet_to = atoi( packet_tostr.c_str());

	double initial = getNewTime();
	double newtime=0;
	double elapsed=0;
	int ret;
	while( !timeout && !recv)
	{
		// If we have no response in "login_to" seconds -> fails
		UpdateTime();
		newtime = getNewTime();
		elapsed = newtime-initial;
		//COUT<<elapsed<<" seconds since login request"<<endl;
		if( elapsed > packet_to)
		{
			COUT<<"Timed out"<<endl;
			timeout = true;
			cleanup();
		}
		ret=this->checkMsg( &packet );
		if( ret>0)
		{
			if( packet.getCommand() == command)
				COUT<<"Got a response with corresponding command"<<endl;
			else
			{
				COUT<<"Got a response with unexpected command : ";
				displayCmd( packet.getCommand());
				COUT<<endl<<"!!! PROTOCOL ERROR -> EXIT !!!"<<endl;
				cleanup();
			}
			recv = true;
		}
		else if( ret<0)
		{
			COUT<<"!!! Error, dead connection to server -> EXIT !!!"<<endl;
			cleanup();
		}

		micro_sleep( 40000);
	}
	return recv;
}

/*************************************************************/
/**** Login loop                                          ****/
/*************************************************************/

int		NetClient::checkAcctMsg( )
{
	int len=0;
	AddressIP	ip2;
	Packet packeta;
	int ret=0;

	// Watch account server socket
	// Get the number of active clients
	if( acct_sock.isActive( ))
	{
		//COUT<<"Net activity !"<<endl;
		// Receive packet and process according to command

		PacketMem mem;
		if( (len=acct_sock.recvbuf( mem, &ip2 ))>0 )
		{
			ret = 1;
			Packet p( mem );
			packeta = p;
			NetBuffer netbuf( packeta.getData(), packeta.getDataLength());
			switch( packeta.getCommand())
			{
				case LOGIN_DATA :
				{
					COUT << ">>> LOGIN DATA --------------------------------------"<<endl;
					// We received game server info (the one we should connect to)
					_serverip = netbuf.getString();
					_serverport = netbuf.getString();
					COUT << "<<< LOGIN DATA --------------------------------------"<<endl;
				}
				break;
				case LOGIN_ERROR :
					COUT<<">>> LOGIN ERROR =( DENIED )= --------------------------------------"<<endl;
					globalsaves.push_back( "");
					globalsaves.push_back( "!!! ACCESS DENIED : Account does not exist !!!");
				break;
				case LOGIN_ALREADY :
					COUT<<">>> LOGIN ERROR =( ALREADY LOGGED IN )= --------------------------------------"<<endl;
					globalsaves.push_back( "");
					globalsaves.push_back( "!!! ACCESS DENIED : Account already logged in !!!");
				break;
				default:
					COUT<<">>> UNKNOWN COMMAND =( "<<hex<<packeta.getSerial()<<" )= --------------------------------------"<<endl;
					globalsaves.push_back( "");
					globalsaves.push_back( "!!! PROTOCOL ERROR : Unexpected command received !!!");
			}
		}
		else
		{
			globalsaves.push_back( "");
			globalsaves.push_back( "!!! NETWORK ERROR : Connection to account server lost !!!");
			acct_sock.disconnect( __PRETTY_FUNCTION__, false );
		}
	}

	return ret;
}

vector<string>	NetClient::loginLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginLoop" << endl;

	Packet	packet2;
	NetBuffer netbuf;

	//memset( buffer, 0, tmplen+1);
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
/**** Launch the client                                   ****/
/*************************************************************/

extern bool cleanexit;

void	NetClient::start( char * addr, unsigned short port)
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

	keeprun = 1;

	cout<<"Loading data files..."<<endl;
	this->readDatafiles();

	cout<<"Initializing network connection..."<<endl;
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

	if( this->authenticate() == -1)
	{
		perror( "Error login in ");
		cleanexit=true;
		winsys_exit(1);
	}

	cout<<"Initiating client loop"<<endl;
	while( keeprun)
	{
		this->checkKey();
		this->checkMsg( NULL );
		micro_sleep( 30000);
	}

	this->disconnect();
}

void	NetClient::checkKey()
{
}

/**************************************************************/
/**** Check if its is time to get network messages         ****/
/**************************************************************/

// This function is made to decide whether it is time to check
// network messages or not... depending on how often we want to
// do so.
// For now, it is always time to receive network messages

int		NetClient::isTime()
{
	int ret=0;
	cur_time += GetElapsedTime();
	//COUT<<"cur_time="<<cur_time<<" - elapsed="<<GetElapsedTime()<<endl;
	if( cur_time > NETWORK_ATOM)
	{
		cur_time = 0;
		ret = 1;
	}
	return ret;
}

/**************************************************************/
/**** Send packets to server                               ****/
/**************************************************************/

// void	NetClient::sendMsg()
// {
// }

/**************************************************************/
/**** Check if we have to send a webcam picture            ****/
/**************************************************************/

void	NetClient::sendWebcamPicture()
{
}

/**************************************************************/
/**** Check if server has sent something                   ****/
/**************************************************************/

int NetClient::checkMsg( Packet* outpacket )
{
    int ret=0;
	string jpeg_str( "");

    if( clt_sock.isActive( ) )
    {
        ret = recvMsg( outpacket );
    }
#ifdef NETCOMM
	// If we have network communications enabled and webcam support enabled we grab an image
	if( NetComm->IsActive())
	{
		// Send grabbed image from webcam
		NetComm->SendImage( this->clt_sock);
		// Here also send samples
		NetComm->SendSound( this->clt_sock);
	}
#endif
	
    return ret;
}

/**************************************************************/
/**** Receive a message from the server                    ****/
/**************************************************************/

int NetClient::recvMsg( Packet* outpacket )
{
    ObjSerial	packet_serial=0;

    // Receive data
    AddressIP sender_adr;
	PacketMem mem;
	Unit * un = NULL;
	int mount_num;
	ObjSerial mis;
	ObjSerial local_serial;
	if( this->game_unit.GetUnit() != NULL)
		local_serial = this->game_unit.GetUnit()->GetSerial();
	Cockpit * cp;

    int recvbytes = clt_sock.recvbuf( mem, &sender_adr );

    if( recvbytes <= 0)
    {
        perror( "Error recv -1 ");
        clt_sock.disconnect( "socket error", 0 );
        return -1;
    }
    else
    {
        Packet p1( mem );
		NetBuffer netbuf( p1.getData(), p1.getDataLength());
	    p1.setNetwork( &sender_adr, clt_sock );
	    if( outpacket )
	    {
	        *outpacket = p1;
	    }
        packet_serial     = p1.getSerial();
        old_timestamp     = current_timestamp;
        current_timestamp = p1.getTimestamp();
        deltatime         = current_timestamp - old_timestamp;
	    Cmd cmd           = p1.getCommand( );
	    COUT << "Rcvd: " << cmd << " ";
        switch( cmd )
        {
            // Login accept
            case LOGIN_ACCEPT :
			{
				Packet pckt;
                COUT << ">>> LOGIN ACCEPTED =( serial n°" << packet_serial << " )= --------------------------------------" << endl;
                // Should receive player's data (savegame) from server if there is a save
                this->serial = packet_serial;
                localSerials.push_back( packet_serial);
				globalsaves.push_back( netbuf.getString());
				globalsaves.push_back( netbuf.getString());
				// Get the galaxy file from buffer with relative path to datadir !
				string univfile = netbuf.getString();
				unsigned char * md5_digest = netbuf.getBuffer( MD5_DIGEST_SIZE);
				// Compare to local md5 and ask for the good file if we don't have it or bad version
				if( 0 /* !md5CheckFile( univfile, md5_digest) */)
				{
					VsnetDownload::Client::File* requested_file;
					requested_file = new VsnetDownload::Client::File( this->clt_sock, univfile, "");
                	_downloadManagerClient->addItem( requested_file);
					while( requested_file->state()!=VsnetDownload::Client::Completed)
						micro_sleep( 10000);
					/*
					netbuf.addString( univfile);
					pckt.send( CMD_ASKFILE, packet_serial,
                               netbuf.getData(), netbuf.getDataLength(),
                               SENDRELIABLE, NULL, this->clt_sock,
                               __FILE__, PSEUDO__LINE__(663) );
					this->PacketLoop( CMD_ASKFILE);
					*/
				}

				// Get the initial system file...
				string sysfile = netbuf.getString();
				md5_digest = netbuf.getBuffer( MD5_DIGEST_SIZE);
				// THINK TO PUT THAT TEST BACK WHEN DOWNLOAD THREAD IS COMPLETE !!!!!!!
				if( 0 /* !md5CheckFile( sysfile, md5_digest) */)
				{
					VsnetDownload::Client::File* requested_file;
					requested_file = new VsnetDownload::Client::File( this->clt_sock, sysfile, "");
                	_downloadManagerClient->addItem( requested_file);
					while( requested_file->state()!=VsnetDownload::Client::Completed)
						micro_sleep( 10000);
					/*
					netbuf.addString( sysfile);
					pckt.send( CMD_ASKFILE, packet_serial,
                               netbuf.getData(), netbuf.getDataLength(),
                               SENDRELIABLE, NULL, this->clt_sock,
                               __FILE__, PSEUDO__LINE__(677) );
					this->PacketLoop( CMD_ASKFILE);
					*/
					// Loop until the download is complete
				}
				//globalsaves = FileUtil::GetSaveFromBuffer( p1.getData()+2*NAMELEN);

				/*
                VsnetDownload::Client::TestItem* t;
                t = new VsnetDownload::Client::TestItem( clt_sock, "TESTFILE" );
                _downloadManagerClient->addItem( t );
				*/
			}
            break;
			case CMD_ASKFILE :
			{
				FILE * fp;
				string filename;
				string file;
				// If packet serial == 0 then it means we have an up to date file
				if( packet_serial==this->game_unit.GetUnit()->GetSerial())
				{
					// Receive the file and write it (trunc if exists)
					filename = netbuf.getString();
					file = netbuf.getString();
					cerr<<"RECEIVING file : "<<filename<<endl;
					fp = fopen( (datadir+filename).c_str(), "w");
					if (!fp)
					{
						cerr<<"!!! ERROR : opening received file !!!"<<endl;
						exit(1);
					}
					if( fwrite( file.c_str(), sizeof( char), file.length(), fp) != file.length())
					{
						cerr<<"!!! ERROR : writing received file !!!"<<endl;
						exit(1);
					}
				}
				else
				{
					// Something is wrong
					displayError( packet_serial);
				}
			}
			break;
            case CMD_DOWNLOAD :
				COUT << endl;
                if( _downloadManagerClient )
                {
                    _downloadManagerClient->processCmdDownload( clt_sock, netbuf );
                }
                break;
            // Login failed
            case LOGIN_ERROR :
                COUT<<">>> LOGIN ERROR =( DENIED )= ------------------------------------------------"<<endl;
                //COUT<<"Received LOGIN_ERROR"<<endl;
                this->disconnect();
				globalsaves.push_back( "");
				globalsaves.push_back( "!!! ACCESS DENIED : Account does not exist !!!");
                return -1;
                break;
			case LOGIN_UNAVAIL :
				COUT<<">>> ACCOUNT SERVER UNAVAILABLE ------------------------------------------------"<<endl;
				globalsaves.push_back( "");
				globalsaves.push_back( "!!! ACCESS DENIED : Account server unavailable !!!");
				this->disconnect();
				return -1;
				break;
            // Create a character
            case CMD_CREATECHAR :
				COUT << endl;
                // Should begin character/ship creation process
                //this->createChar();
                break;
            // Receive start locations
            case CMD_LOCATIONS :
				COUT << endl;
                // Should receive possible starting locations list
                this->receiveLocations( &p1 );
                break;
            case CMD_SNAPSHOT :
                // Should update another client's position
                //COUT<<"Received a SNAPSHOT from server"<<endl;
				// We don't want to consider a late snapshot
				if( old_timestamp > current_timestamp)
					break;
                this->receivePosition( &p1 );
                break;
            case CMD_ENTERCLIENT :
                COUT << ">>> " << local_serial << " >>> ENTERING CLIENT =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                this->addClient( &p1 );
                break;
            case CMD_EXITCLIENT :
                COUT << ">>> " << local_serial << " >>> EXITING CLIENT =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                this->removeClient( &p1 );
                break;
            case CMD_ADDEDYOU :
                {
                    COUT << ">>> " << local_serial << " >>> ADDED IN GAME =( serial n°"
                         << packet_serial << " )= --------------------------------------" << endl;
				    // Get the zone id in the packet
                    char flags = netbuf.getChar( );
                    if( flags & CMD_CAN_COMPRESS ) clt_sock.allowCompress( true );
				    this->zone = netbuf.getShort();
				    _Universe->current_stardate.Init( netbuf.getString());
				    cout << "WE ARE ON STARDATE "
                         <<_Universe->current_stardate.GetFullCurrentStarDate() << endl;
                    COUT << "Compression: " << ( (flags & CMD_CAN_COMPRESS) ? "yes" : "no" ) << endl;
                    //this->getZoneData( &p1 );
                }
                break;
            case CMD_DISCONNECT :
                /*** TO REDO IN A CLEAN WAY ***/
                COUT << ">>> " << local_serial << " >>> DISCONNECTED -> Client killed =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                exit(1);
                break;
//             case CMD_ACK :
//                 /*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
//                 /*** We must make sure those 2 conditions are enough ***/
//                 COUT << ">>> ACK =( " << current_timestamp
//                      << " )= ---------------------------------------------------" << endl;
// 				p1.ack( );
//                 break;
			case CMD_FIREREQUEST :
				// WE RECEIVED A FIRE NOTIFICATION SO FIRE THE WEAPON
				mount_num = netbuf.getInt32();
				mis = netbuf.getSerial();
				// Find the unit
				//Unit * un = zonemgr->getUnit( packet.getSerial(), zone);
				if( mis==local_serial) // WE have fired and receive the broadcast
					un = this->game_unit.GetUnit();
				else
					un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un!=NULL)
				{
					// Set the concerned mount as ACTIVE and others as INACTIVE
					vector <Mount>
						::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
					for (;i!=un->mounts.end();++i)
						(*i).status=Mount::INACTIVE;
					un->mounts[mount_num].processed=Mount::ACCEPTED;
					un->mounts[mount_num].status=Mount::ACTIVE;
					// Store the missile id in the mount that should fire a missile
					un->mounts[mount_num].serial=mis;
					// Ask for fire
					if( mis != 0)
						un->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
					else
						un->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
				}
				else
					COUT<<"!!! Problem -> CANNOT FIRE UNIT NOT FOUND !!!"<<endl;

			break;
			case CMD_UNFIREREQUEST :
				// WE RECEIVED AN UNFIRE NOTIFICATION SO DEACTIVATE THE WEAPON
				mount_num = netbuf.getInt32();
				mis = netbuf.getSerial();
				// Find the unit
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un != NULL)
				{
					// Set the concerned mount as ACTIVE and others as INACTIVE
					vector <Mount>
						::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
					for (;i!=un->mounts.end();++i)
						(*i).status=Mount::INACTIVE;
					un->mounts[mount_num].processed=Mount::UNFIRED;
					// Store the missile id in the mount that should fire a missile
					un->mounts[mount_num].serial=mis;
					// Ask for fire
					if( mis != 0)
						un->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
					else
						un->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
				}
				else
					COUT<<"!!! Problem -> CANNOT UNFIRE UNIT NOT FOUND !!!"<<endl;

			break;
			case CMD_SCAN :
				// We received the target info with the target serial in the packet as an answer to a scanRequest

				// Update info with received buffer

				// And tell all VDUs we received the target info
				cp = _Universe->isPlayerStarship( this->game_unit.GetUnit());
				cp->ReceivedTargetInfo();
			break;
			case CMD_SNAPDAMAGE :
			{
				// In case we use damage snapshots : we do not call ApplyNetDamage
				// In fact we trusted the client only for visual FX : Check where they are done !
				// but the server computes the damage itself

				// SHOULD READ THE DAMAGE SNAPSHOT HERE !
				int nbupdates = packet_serial;
				ObjSerial serial;
				size_t it=0;
				unsigned short damages;
				for( int i=0; i<nbupdates; i++)
				{
					damages = netbuf.getShort();
					serial = netbuf.getSerial();
					un = UniverseUtil::GetUnitFromSerial( serial);

					if( damages & Unit::SHIELD_DAMAGED)
					{
						un->shield = netbuf.getShield();
					}
					if( damages & Unit::ARMOR_DAMAGED)
					{
						un->armor = netbuf.getArmor();
					}
					if( damages & Unit::COMPUTER_DAMAGED)
					{
						un->computer.itts = netbuf.getChar();
						un->computer.radar.color = netbuf.getChar();
						un->limits.retro = netbuf.getFloat();
						un->computer.radar.maxcone = netbuf.getFloat();
						un->computer.radar.lockcone = netbuf.getFloat();
						un->computer.radar.trackingcone = netbuf.getFloat();
						un->computer.radar.maxrange = netbuf.getFloat();
						for( it = 0; it<1+UnitImages::NUMGAUGES+MAXVDUS; it++)
							un->image->cockpit_damage[it] = netbuf.getFloat();
					}
					if( damages & Unit::MOUNT_DAMAGED)
					{
						un->image->ecm = netbuf.getShort();
						for( it=0; it<un->mounts.size(); it++)
						{
							if( sizeof( Mount::STATUS) == sizeof( char))
								un->mounts[it].status = ( Mount::STATUS) netbuf.getChar();
							else if( sizeof( Mount::STATUS) == sizeof( unsigned short))
								un->mounts[it].status = ( Mount::STATUS) netbuf.getShort();
							else if( sizeof( Mount::STATUS) == sizeof( unsigned int))
								un->mounts[it].status = ( Mount::STATUS) netbuf.getInt32();

							un->mounts[it].ammo = netbuf.getShort();
							un->mounts[it].time_to_lock = netbuf.getFloat();
							un->mounts[it].size = netbuf.getShort();
						}
					}
					if( damages & Unit::CARGOFUEL_DAMAGED)
					{
						un->SetFuel( netbuf.getFloat());
						un->SetAfterBurn(netbuf.getShort());
						un->image->cargo_volume = netbuf.getFloat();
						for( it=0; it<un->image->cargo.size(); it++)
							un->image->cargo[it].quantity = netbuf.getInt32();
					}
					if( damages & Unit::JUMP_DAMAGED)
					{
						un->shield.leak = netbuf.getChar();
						un->shield.recharge = netbuf.getFloat();
						un->SetEnergyRecharge( netbuf.getFloat());
						un->SetMaxEnergy( netbuf.getFloat());
						un->jump.energy = netbuf.getShort();
						un->jump.damage = netbuf.getChar();
						un->image->repair_droid = netbuf.getChar();
					}
					if( damages & Unit::CLOAK_DAMAGED)
					{
						un->cloaking = netbuf.getShort();
						un->image->cloakenergy = netbuf.getFloat();
						un->cloakmin = netbuf.getShort();
						un->shield = netbuf.getShield();
					}
					if( damages & Unit::LIMITS_DAMAGED)
					{
						un->computer.max_pitch = netbuf.getFloat( );
						un->computer.max_yaw = netbuf.getFloat( );
						un->computer.max_roll = netbuf.getFloat( );
						un->limits.roll = netbuf.getFloat( );
						un->limits.yaw = netbuf.getFloat( );
						un->limits.pitch = netbuf.getFloat( );
						un->limits.lateral = netbuf.getFloat( );
					}
				}
			}
			break;
#ifdef NET_SHIELD_SYSTEM_1
			case CMD_DAMAGE :
			{
				float amt = netbuf.getFloat();
				float ppercentage = netbuf.getFloat();
				float spercentage = netbuf.getFloat();
				Vector pnt = netbuf.getVector();
				Vector normal = netbuf.getVector();
				GFXColor col = netbuf.getColor();
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un)
				{
					un->shield = netbuf.getShield();
					un->armor = netbuf.getArmor();
					// Apply the damage
					un->ApplyNetDamage( pnt, normal, amt, ppercentage, spercentage, col);
				}
				else
					COUT<<"!!! Problem -> CANNOT APPLY DAMAGE UNIT NOT FOUND !!!"<<endl;
			}
			break;
			case CMD_DAMAGE1 :
			{
				/*
				float amt = netbuf.getFloat();
				float phasedamage = netbuf.getFloat();
				Vector pnt = netbuf.getVector();
				Vector normal = netbuf.getVector();
				GFXColor col = netbuf.getColor();
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				un->ApplyLocalDamage( pnt, normal, amt, NULL, col, phasedamage);
				*/
			}
			break;
#endif
			case CMD_KILL :
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un)
					un->Kill();
				else
					COUT<<"!!! Problem -> CANNOT KILL UNIT NOT FOUND !!!"<<endl;
			break;
			case CMD_JUMP :
			{
				StarSystem * sts;
				string newsystem = netbuf.getString();
				// Get the pointer to the new star system sent by server
				if( !(sts=star_system_table.Get( newsystem)))
				{
					// The system should have been loaded just before we asked for the jump so this is just a safety check
					cerr<<"!!! ERROR : Couldn't find destination Star system !!!"<<endl;
					exit(1);
				}
				// If we received a CMD_JUMP with serial==player serial jump is refused because of energy
				if( packet_serial!=0)
				{
					// The jump has been allowed
					// Check if server said we have the good file
					if( packet_serial!=un->GetSerial())
					{
						VsnetDownload::Client::File* requested_file;
						requested_file = new VsnetDownload::Client::File( this->clt_sock, newsystem, "");
                		_downloadManagerClient->addItem( requested_file);
						while( requested_file->state()!=VsnetDownload::Client::Completed)
							micro_sleep( 10000);
					}
					this->jumpok = true;
					string system2 = _Universe->isPlayerStarship( this->game_unit.GetUnit())->savegame->GetStarSystem();
					this->ingame = false;
				}
				else
				{
					// Jump was refused either because the destination system asked do not exist or because not enough jump energy
					this->jumpok = false;
				}
			}
			break;
			case CMD_STARTNETCOMM :
#ifdef NETCOMM
			{
				ClientPtr clt;
				// Check this is not us
				if( packet_serial != this->serial)
				{
					// Add the client to netcomm list in NetComm ?
					clt = Clients.get(packet_serial);
					NetComm->AddToSession( clt);
				}
			}
#endif
			break;
			case CMD_STOPNETCOMM :
#ifdef NETCOMM
			{
				ClientPtr clt;
				// Check this is not us
				if( packet_serial != this->serial)
				{
					// Remove the client to netcomm list in NetComm
					clt = Clients.get(packet_serial);
					NetComm->RemoveFromSession( clt);
				}
			}
#endif
			break;
            default :
                COUT << ">>> " << local_serial << " >>> UNKNOWN COMMAND =( " << hex << cmd
                     << " )= --------------------------------------" << endl;
                keeprun = 0;
                this->disconnect();
        }
    }
    return recvbytes;
}

/*************************************************************/
/**** Get the zone data from server                       ****/
/*************************************************************/

// NOT USED ANYMORE !!
void NetClient::getZoneData( const Packet* packet )
{
}

/*************************************************************/
/**** Adds an entering client in the actual zone          ****/
/*************************************************************/

void	NetClient::addClient( const Packet* packet )
{
	ObjSerial cltserial = packet->getSerial();
	ClientPtr clt;
	// NOTE : in splitscreen mode we may receive info about us so we comment the following test
	/*
	if( cltserial==this->serial)
	{
		// Maybe just ignore here
		COUT<<"FATAL ERROR : RECEIVED INFO ABOUT ME !"<<endl;
		exit(1);
	}
	*/

	clt = Clients.get(cltserial);
	if( clt)
	{
		// Client may exist if it jumped from a starsystem to another of if killed and respawned
		COUT<<"Existing client n°"<<cltserial<<endl;
	}
	else
	{
		clt = Clients.insert( cltserial, new Client );
		nbclients++;
		COUT<<"New client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	}


	//Packet ptmp = packet;
	NetBuffer netbuf( packet->getData(), packet->getDataLength());
	// Should receive the name
	clt->name = netbuf.getString();
	// If not a local player, add it in our array
	if( !isLocalSerial( cltserial))
	{
		// The save buffer and XML buffer come after the ClientState
		vector<string> saves;
		saves.push_back( netbuf.getString());
		saves.push_back( netbuf.getString());
		//saves = FileUtil::GetSaveFromBuffer( packet->getData()+sizeof( ClientState));
		COUT<<"SAVE="<<saves[0].length()<<" bytes - XML="<<saves[1].length()<<" bytes"<<endl;

		// We will ignore - starsys as if a client enters he is in the same system
		//                - pos since we received a ClientState
		//                - creds as we don't care about other players' credits for now
		string starsys;
		QVector pos;
		float creds;
		bool update=true;
		vector<string> savedships;
		// Parse the save buffer
		save.ParseSaveGame( "", starsys, "", pos, update, creds, savedships, 0, saves[0], false);

		// WE DON'T STORE FACTION IN SAVE YET
		string PLAYER_FACTION_STRING( "privateer");

		// CREATES THE UNIT... GET SAVE AND XML FROM SERVER
		// Use the first ship if there are more than one -> we don't handle multiple ships for now
		// We name the flightgroup with the player name
		cerr<<"Found saveship[0] = "<<savedships[0]<<endl;
		Unit * un = UnitFactory::createUnit( savedships[0].c_str(),
							 false,
							 FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
							 string(""),
							 Flightgroup::newFlightgroup ( callsign,savedships[0],PLAYER_FACTION_STRING,"default",1,1,"","",mission),
							 0, &saves[1]);
		clt->game_unit.SetUnit( un);
		// Set all weapons to inactive
		vector <Mount>
			::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
		for (;i!=un->mounts.end();++i)
			(*i).status=Mount::INACTIVE;
		un->SetNetworkMode( true);
		un->SetSerial( cltserial);
		//COUT<<"Addclient 4"<<endl;

		// Assign new coordinates to client
		un->SetPosition( save.GetPlayerLocation());
		un->SetSerial( cltserial);

		// In that case, we want cubic spline based interpolation
		//init_interpolation( cltserial);
	}
	// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
	else if( cltserial!=this->game_unit.GetUnit()->GetSerial())
	{
		clt->game_unit.SetUnit( getNetworkUnit( cltserial));
		assert( clt->game_unit.GetUnit() != NULL);
	}
}

/*************************************************************/
/**** Removes an exiting client of the actual zone        ****/
/*************************************************************/

void	NetClient::removeClient( const Packet* packet )
{
	ObjSerial	cltserial = packet->getSerial();

	COUT<<" & HTONS(Serial) = "<<cltserial<<endl;
	if( !Clients.get(cltserial))
	{
		cerr<<"Error, client does not exists !!"<<endl;
		exit( 1);
	}

	// Removes the unit from starsystem, destroys it and delete client
	_Universe->activeStarSystem()->RemoveUnit(Clients.get(cltserial)->game_unit.GetUnit());
	nbclients--;
	Clients.remove(cltserial);
	COUT<<"Leaving client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
}

/*************************************************************/
/**** Send an update to server                            ****/
/*************************************************************/

void	NetClient::sendPosition( const ClientState* cs )
{
	// Serial in ClientState is updated in UpdatePhysics code at ClientState creation (with pos, veloc...)
	Packet pckt;
	NetBuffer netbuf;

	// Send the client state
	COUT<<"Sending ClientState == ";
	(*cs).display();
	netbuf.addClientState( (*cs));
	pckt.send( CMD_POSUPDATE, this->game_unit.GetUnit()->GetSerial(),
               netbuf.getData(), netbuf.getDataLength(),
               SENDANDFORGET, NULL, this->clt_sock,
               __FILE__, PSEUDO__LINE__(1197) );
}

/**************************************************************/
/**** Update another client position                       ****/
/**************************************************************/

void	NetClient::receivePosition( const Packet* packet )
{
	// When receiving a snapshot, packet serial is considered as the number of client updates
	ClientState cs;
	const char* databuf;
	ObjSerial   sernum=0;
	int		nbclts=0, i, j;
	// int		offset=0;
	// int		nbclts2=0;
	// int		cssize = sizeof( ClientState);
	// int		smallsize = sizeof( ObjSerial) + sizeof( QVector);
	// int		qfsize = sizeof( double);
	unsigned char	cmd;
	ClientPtr clt;
	Unit * un;

	nbclts = packet->getSerial();
	//nbclts = ntohs( nbclts2);
	COUT << "Received update for " << nbclts << " clients - LENGTH="
	     << packet->getDataLength() << endl;
	databuf = packet->getData();
	NetBuffer netbuf( packet->getData(), packet->getDataLength());
	for( i=0, j=0; (i+j)<nbclts;)
	{
		//cmd = *(databuf+offset);
		cmd = netbuf.getChar();
		//offset += sizeof( unsigned char);
		if( cmd == CMD_FULLUPDATE)
		{
			cs = netbuf.getClientState();
			// Do what needed with update
			COUT<<"Received FULLSTATE ";
			// Tell we received the ClientState so we can convert byte order from network to host
			//cs.display();
			sernum = cs.getSerial();
			clt = Clients.get(sernum);
			un = clt->game_unit.GetUnit();
			// Test if this is a local player
			// Is it is, ignore position update
			if( clt && !_Universe->isPlayerStarship( Clients.get(sernum)->game_unit.GetUnit()))
			{
				// Backup old state
				un->prev_physical_state = un->curr_physical_state;
				// Update concerned client directly in network client list
				un->curr_physical_state.position = cs.getPosition();
				un->curr_physical_state.orientation = cs.getOrientation();
				un->Velocity = cs.getVelocity();

				// In that case, we want cubic spline based interpolation
				//predict( sernum);
				//init_interpolation( sernum);
			}
			//offset += cssize;
			i++;
		}
		else if( cmd == CMD_POSUPDATE)
		{
			// Set the serial #
			//sernum = *((ObjSerial *) databuf+offset);
			//sernum = ntohs( sernum);
			sernum = netbuf.getShort();
			clt = Clients.get(sernum);
			COUT<<"Received POSUPDATE for serial "<<sernum<<" -> ";
			//offset += sizeof( ObjSerial);
			if( clt && !_Universe->isPlayerStarship( clt->game_unit.GetUnit()))
			{
				// Backup old state
				un->prev_physical_state = un->curr_physical_state;
				// Update concerned client directly in network client list
				un->curr_physical_state.position = cs.getPosition();
				un->curr_physical_state.orientation = cs.getOrientation();
				un->Velocity = cs.getVelocity();

				// Set the new received position in curr_physical_state
				un->curr_physical_state.position = netbuf.getQVector();
				//predict( sernum);
			}
			else
			{
				// QVector tmppos = netbuf.getVector();
				COUT<<"ME OR LOCAL PLAYER = IGNORING"<<endl;
			}
			//offset += sizeof( QVector);
			j++;
		}
	}
}

/*************************************************************/
/**** Says we are ready and going in game                 ****/
/*************************************************************/

void	NetClient::inGame()
{
	Packet    packet2;
	NetBuffer netbuf;
    char      flags = 0;
    if( canCompress() ) flags |= CMD_CAN_COMPRESS;
    netbuf.addChar( flags );

	//ClientState cs( this->serial, this->game_unit.GetUnit()->curr_physical_state, this->game_unit.GetUnit()->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !! NOT NEEDED ANYMORE -> THE SERVER ALREADY KNOWS
	//netbuf.addClientState( cs);
	packet2.send( CMD_ADDCLIENT, this->serial,
                  netbuf.getData(), netbuf.getDataLength(),
                  SENDRELIABLE, NULL, this->clt_sock,
                  __FILE__, PSEUDO__LINE__(1307) );
	this->game_unit.GetUnit()->SetSerial( this->serial);
	COUT << "Sending ingame with serial n°" << this->serial
         << " " << (canCompress() ? "(compress)" : "(no compress)") <<endl;
	this->ingame = true;
}

/*************************************************************/
/**** Says we are still alive                             ****/
/*************************************************************/

void NetClient::sendAlive()
{
    if( clt_sock.isTcp() == false )
    {
        Packet	p;
        p.send( CMD_PING, this->game_unit.GetUnit()->GetSerial(),
                NULL, 0,
                SENDANDFORGET, NULL, this->clt_sock,
                __FILE__, PSEUDO__LINE__(1325) );
    }
}

/*************************************************************/
/**** Receive the ship and char from server               ****/
/*************************************************************/

void NetClient::receiveSave( const Packet* packet )
{
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

/*************************************************************/
/**** Disconnect from the server                          ****/
/*************************************************************/

void	NetClient::disconnect()
{
	keeprun = 0;
	// Disconnection is handled in the cleanup() function for each player
}

void	NetClient::logout()
{
	keeprun = 0;
	Packet p;
	p.send( CMD_LOGOUT, this->game_unit.GetUnit()->GetSerial(),
            NULL, 0,
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1382) );
	clt_sock.disconnect( "Closing connection to server", false );
}

/*************************************************************/
/**** Read the client's datafiles                         ****/
/*************************************************************/

// Should load game's data

void	NetClient::readDatafiles()
{
}


/*************************************************************/
/**** INTERPOLATION STUFF                                 ****/
/*************************************************************/

void	NetClient::predict( ObjSerial clientid)
{
	// This function is to call after the state have been updated (which should be always the case)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	Unit * un = Clients.get(clientid)->game_unit.GetUnit();
	unsigned int del = Clients.get(clientid)->deltatime;
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	//QVector A( un->prev_physical_state.position);
	QVector B( un->curr_physical_state.position);
	//Vector  VA( un-> ???? OLD VELOCITY ??? );
	Vector  VB( un->Velocity);
	//Vector  AA( un->GetPrevAcceleration() ???? );
	Vector  AB( un->GetAcceleration());
	//QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	//QVector A2( A3 - (VB + AB*delay));

	// HERE : Backup the current state ???? --> Not sure
	un->curr_physical_state.position = A3;
}

void	NetClient::init_interpolation( ObjSerial clientid)
{
	// This function is to call after the state have been updated (which should be always the case)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	/************* VA IS TO BE UNCOMMENTED ****************/
	Unit * un = Clients.get(clientid)->game_unit.GetUnit();
	unsigned int del = Clients.get(clientid)->deltatime;
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	QVector A( un->prev_physical_state.position);
	QVector B( un->curr_physical_state.position);
	// Should get old velocity in VA here : NOT ACTUAL ONE !!!
	Vector  VA( un->Velocity);
	Vector  VB( un->Velocity);
	//Vector  AA( un->GetPrevAcceleration() ???? );
	Vector  AB( un->GetAcceleration());
	QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	QVector A2( A3 - (VB + AB*delay));

	//Clients.get(clientid)->spline.createSpline( A, A1, A2, A3);
}

Transformation NetClient::spline_interpolate( ObjSerial clientid, double blend)
{
	// blend = time
	// Add a linear interpolation for orientation
	Quaternion orient;
	// There should be another function called when received a new position update and creating the spline
	QVector pos( Clients.get(clientid)->spline.computePoint( blend));

	return Transformation( orient, pos);
}

/******************************************************************************************/
/*** WEAPON STUFF                                                                      ****/
/******************************************************************************************/

// Send a info request about the target
void	NetClient::scanRequest( Unit * target)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( target->GetSerial());
	netbuf.addShort( this->zone);

	p.send( CMD_TARGET, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1485) );
}

// In fireRequest we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetClient::fireRequest( ObjSerial serial, int mount_index, char mis)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);
	netbuf.addShort( this->zone);
	netbuf.addChar( mis);

	p.send( CMD_FIREREQUEST, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1503) );
}

void	NetClient::unfireRequest( ObjSerial serial, int mount_index)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);
	netbuf.addInt32( this->zone);

	p.send( CMD_UNFIREREQUEST, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1518) );
}

bool	NetClient::jumpRequest( string newsystem)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addString( newsystem);
	unsigned char * md5 = new unsigned char[MD5_DIGEST_SIZE];
	md5Compute( datadir+"/"+newsystem+".system", md5);
	netbuf.addBuffer( md5, MD5_DIGEST_SIZE);

	p.send( CMD_JUMP, this->game_unit.GetUnit()->GetSerial(),
            netbuf.getData(), netbuf.getDataLength(),
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1534) );
	// Should wait for jump authorization
	this->PacketLoop( CMD_JUMP);
	bool ret;
	if( this->jumpok)
		ret = true;
	else
		ret = false;

	jumpok = false;

	return ret;
}

/******************************************************************************************/
/*** COMMUNICATION STUFF                                                               ****/
/******************************************************************************************/

void	NetClient::startCommunication()
{
#ifdef NETCOMM
	selected_freq = current_freq;
	NetBuffer netbuf;
	netbuf.addFloat( selected_freq);
	NetComm->InitSession( selected_freq);
	//cerr<<"Session started."<<endl;
	//cerr<<"Grabbing an image"<<endl;
	Packet p;
	p.send( CMD_STARTNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1565) );
	cerr<<"Starting communication session\n\n"<<endl;
	//NetComm->GrabImage();
#endif
}

void	NetClient::stopCommunication()
{
#ifdef NETCOMM
	NetBuffer netbuf;
	netbuf.addFloat( selected_freq);
	Packet p;
	p.send( CMD_STOPNETCOMM, serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1578) );
	NetComm->DestroySession();
	cerr<<"Stopped communication session"<<endl;
#endif
}

void	NetClient::decreaseFrequency()
{
	if( current_freq == MIN_COMMFREQ)
		current_freq = MAX_COMMFREQ;
	else
		current_freq -= .1;
}

void	NetClient::increaseFrequency()
{
	if( current_freq == MAX_COMMFREQ)
		current_freq = MIN_COMMFREQ;
	else
		current_freq += .1;
}

float	NetClient::getSelectedFrequency()
{ return this->selected_freq;}

float	NetClient::getCurrentFrequency()
{ return this->current_freq;}

void	NetClient::sendTextMessage( string message)
{
#ifdef NETCOMM
	// Only send if netcomm is active and we are connected on a frequency
	if( NetComm->IsActive())
		NetComm->SendMessage( this->clt_sock, message);
#endif
}

bool NetClient::IsNetcommActive() const
{
#ifdef NETCOMM
    return ( this->NetComm==NULL ? false : this->NetComm->IsActive() );
#else
    return false;
#endif
}

bool NetClient::canCompress() const
{
#ifdef HAVE_ZLIB_H
    return true;
#else
    return false;
#endif
}


ClientPtr NetClient::Clients::insert( int x, Client* c )
{
    if( c != NULL )
    {
        ClientPtr cp( c );
        _map.insert( ClientPair( x, cp ) );
        return cp;
    }
    else
    {
        return ClientPtr();
    }
}

ClientPtr NetClient::Clients::get( int x )
{
    ClientIt it = _map.find(x);
    if( it == _map.end() ) return ClientPtr();
    return it->second;
}

bool NetClient::Clients::remove( int x )
{
    size_t s = _map.erase( x );
    if( s == 0 ) return false;
    return true;
    // shared_ptr takes care of delete
}

