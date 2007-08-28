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

#include "networking/netclient.h"
#include "networking/lowlevel/netui.h"

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
#include "cmd/unit_const_cache.h"
#include "configxml.h"
#include "networking/client.h"
#include "networking/const.h"
#include "networking/lowlevel/packet.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "gfx/matrix.h"
#include "lin_time.h"
#include "vsfilesystem.h"
#include "cmd/role_bitmask.h"
#include "cmd/base_util.h"
#include "gfx/cockpit_generic.h"

#include "networking/lowlevel/vsnet_clientstate.h"
#include "networking/lowlevel/vsnet_debug.h"
#include "networking/lowlevel/vsnet_dloadmgr.h"
#include "networking/lowlevel/vsnet_notify.h"
#include "vegastrike.h"
#include "networking/client.h"
#include "networking/lowlevel/netbuffer.h"
#include "networking/networkcomm.h"
#include "posh.h"
#include "networking/prediction.h"
#include "fileutil.h"


using std::cout;
using std::endl;
using std::cin;

double NETWORK_ATOM;

extern vector<unorigdest *> pendingjump;
extern Hashtable<std::string, StarSystem, 127> star_system_table;
typedef vector<Client *>::iterator VC;
typedef vector<ObjSerial>::iterator ObjI;

extern const Unit* getUnitFromUpgradeName(const string& upgradeName, int myUnitFaction = 0);
extern int GetModeFromName(const char *);  // 1=add, 2=mult, 0=neither.
static const string LOAD_FAILED = "LOAD_FAILED";
extern Cargo* GetMasterPartList(const char *input_buffer);

/*************************************************************/
/**** Tool functions                                      ****/
/*************************************************************/

vector<ObjSerial>	localSerials;
bool isLocalSerial( ObjSerial sernum)
{
	//COUT<<"Looking for serial : "<<sernum<<" in ";
	bool ret=false;
	for( ObjI i=localSerials.begin(); !ret && i!=localSerials.end(); i++)
	{
		//COUT<<(*i)<<", ";
		if( sernum==(*i))
			ret = true;
	}
	//COUT<<endl;

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
void NetClient::Reinitialize() {
  deltatime = 0;
  game_unit = NULL;
  latest_timestamp=0;
  keeprun=1;
  //old_time = 0;
  cur_time = getNewTime();
  enabled = 0;
  nbclients = 0;
  jumpok = false;
  ingame = false;
#ifdef CRYPTO
  FileUtil::use_crypto = true;
#endif
  
  NetComm = NULL;
  lastsave.resize(0);
  //_downloadManagerClient.reset( new VsnetDownload::Client::Manager( _sock_set ) );
  //_sock_set.addDownloadManager( _downloadManagerClient );
  _serverip="";
  _serverport=0;
  callsign=password="";
  this->Clients=ClientsMap();
  
  delete clt_tcp_sock;
  clt_tcp_sock=new SOCKETALT;
  //leave UDP well-enough alone

}
NetClient::NetClient()
  : save("")
{
  keeprun=1;
  clt_tcp_sock=new SOCKETALT;
  clt_udp_sock=new SOCKETALT;
  deltatime = 0;
  game_unit = NULL;
  latest_timestamp=0;
  //old_time = 0;
  cur_time = getNewTime();
  enabled = 0;
  nbclients = 0;
  jumpok = false;
  ingame = false;
#ifdef CRYPTO
  FileUtil::use_crypto = true;
#endif

  NetComm = NULL;
  
  _downloadManagerClient.reset( new VsnetDownload::Client::Manager( _sock_set ) );
  _sock_set.addDownloadManager( _downloadManagerClient );
  
#ifdef CRYPTO
  cout<<endl<<endl<<POSH_GetArchString()<<endl;
#endif
}

NetClient::~NetClient()
{
  
  
	if( NetComm!=NULL)
    {
		delete NetComm;
        NetComm = NULL;
    }
        delete clt_tcp_sock;
        delete clt_udp_sock;
}

/*************************************************************/
/**** Login loop                                          ****/
/*************************************************************/

int		NetClient::checkAcctMsg( )
{

	std::string    packeta;
    AddressIP ipadr;
	int ret=0;

	// Watch account server socket
	// Get the number of active clients
	if( acct_sock->isActive( ))
	{
		//COUT<<"Net activity !"<<endl;
		// Receive packet and process according to command

		if(acct_sock->recvstr( packeta )!=false&&packeta.length() )
		{
			ret = 1;
                        std::string netbuf=packeta;
                        
			switch( getSimpleChar(netbuf)) 
			{
				case ACCT_LOGIN_DATA :
				{
					COUT << ">>> LOGIN DATA --------------------------------------"<<endl;
					// We received game server info (the one we should connect to)
					getSimpleString(netbuf);//uname
					getSimpleString(netbuf);//passwd
					_serverip = getSimpleString(netbuf);
					string srvportstr = getSimpleString(netbuf);
					const char *srvport = srvportstr.c_str();
					int porttemp = atoi(srvport);
					if (porttemp>65535) porttemp=0;
					if (porttemp<0) porttemp=0;
					this->_serverport = (unsigned short)porttemp;
					COUT << "<<< LOGIN DATA --------------------------------------"<<endl;
				}
				break;
				case ACCT_LOGIN_ERROR :
					COUT<<">>> LOGIN ERROR =( DENIED )= --------------------------------------"<<endl;
                                        lastsave.resize(0);
					lastsave.push_back( "");
					lastsave.push_back( "!!! ACCESS DENIED : Account does not exist !!!");
				break;
				case ACCT_LOGIN_ALREADY :
					COUT<<">>> LOGIN ERROR =( ALREADY LOGGED IN )= --------------------------------------"<<endl;
                                        lastsave.resize(0);
					lastsave.push_back( "");
					lastsave.push_back( "!!! ACCESS DENIED : Account already logged in !!!");
				break;
				default:
					COUT<<">>> UNKNOWN COMMAND =( "<<std::hex<<packeta<<std::dec<<" )= --------------------------------------"<<std::endl;
                                        lastsave.resize(0);
					lastsave.push_back( "");
					lastsave.push_back( "!!! PROTOCOL ERROR : Unexpected command received !!!");
			}
		}
		else
		{
			char str[127];
			sprintf(str, "!!! NETWORK ERROR : Connection to account server lost (error number %d)!!!",
#ifdef _WIN32
				WSAGetLastError()
#else
				errno
#endif
				);
//			lastsave.push_back( "");
//			lastsave.push_back( str);
			//acct_sock.disconnect( __PRETTY_FUNCTION__, false );
		}
	}

	return ret;
}

/*************************************************************/
/**** Launch the client                                   ****/
/*************************************************************/

/*
extern bool cleanexit;

void	NetClient::start( char * addr, unsigned short port)
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

	keeprun = 1;

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
	if (!this->clt_sock) {
		perror( "Error creating socket ");
		cleanexit=true;
		VSExit(1);
	}
	if( this->authenticate() == -1)
	{
		perror( "Error login in ");
		cleanexit=true;
		VSExit(1);
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
*/

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
	//COUT<<"cur_time="<<cur_time<<" - elapsed="<<GetElapsedTime()<<endl;
	if( (getNewTime()-cur_time) > NETWORK_ATOM)
	{
		cur_time = getNewTime();
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
/**** Check if server has sent something                   ****/
/**************************************************************/

int NetClient::checkMsg( Packet* outpacket )
{
    int ret=0;
	string jpeg_str( "");
	timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=0;

    if( clt_tcp_sock->isActive( ) || clt_udp_sock->isActive( ) )
    {
        ret = recvMsg( outpacket, &tv );
    }
	if (ret==-1) {
		NetClient::CleanUp(); // Kill networking!!!
		UniverseUtil::startMenuInterface(false, "Connection to VegaServer closed.");
		return -1;
	}
	// If we have network communications enabled and webcam support enabled we grab an image
	if( NetComm!=NULL && NetComm->IsActive())
	{
		// Here also send samples
		NetComm->SendSound( *this->clt_tcp_sock, this->serial);
	}
	
    return ret;
}

#include "lowlevel/vsnet_err.h"
extern void SwitchUnits2(Unit*un);
void NetClient::Respawn( ObjSerial newserial) {
  unsigned int whichcp;
  for (whichcp=0;whichcp<_Universe->numPlayers();++whichcp) {
    if (_Universe->AccessCockpit(whichcp)->GetParent()==NULL)
      break;
  }
  if (whichcp==_Universe->numPlayers()) {
    whichcp=0;
    cerr<<"Error could not find blank cockpit to respawn into\n";
  }
  QVector pos(0,0,0);
  bool setplayerXloc;
  string mysystem;
  Cockpit * cp = _Universe->AccessCockpit(whichcp);
  static float initialzoom = XMLSupport::parse_float(vs_config->getVariable("graphics","inital_zoom_factor","2.25"));
  cp->zoomfactor=initialzoom;
  cp->savegame->SetStarSystem(mysystem);
  cp->savegame->ParseSaveGame ("",mysystem,"",pos,setplayerXloc,cp->credits,cp->unitfilename,whichcp, lastsave[0], false);
  string fullsysname=mysystem+".system";
  StarSystem * ss = _Universe->GenerateStarSystem(fullsysname.c_str(),"",Vector(0,0,0));
  _Universe->pushActiveStarSystem(ss);
  unsigned int oldcp=_Universe->CurrentCockpit();
  _Universe->SetActiveCockpit(cp);
  std::string unkeyname=cp->unitfilename[0];
  int fgsnumber = 0;
  if (cp->fg) {
    fgsnumber=cp->fg->flightgroup_nr++;
    cp->fg->nr_ships++;
    cp->fg->nr_ships_left++;
  }
  Unit *un=UnitFactory::createUnit(unkeyname.c_str(), false,FactionUtil::GetFactionIndex( cp->savegame->GetPlayerFaction()),std::string(""),cp->fg,fgsnumber, &lastsave[1],newserial);
  un->SetSerial(newserial);
  //  fighters[a]->faction = FactionUtil::GetFactionIndex( cp->savegame->GetPlayerFaction());
  cp->SetParent(un,unkeyname.c_str(),"",pos);
  un->SetPosAndCumPos(pos);
  this->game_unit.SetUnit(un);
  localSerials.push_back( newserial);
  ss->AddUnit(un);
  AddClientObject(un,newserial);
  SwitchUnits2(un);
  cp->SetView(CP_FRONT);
  _Universe->SetActiveCockpit(oldcp);
  _Universe->popActiveStarSystem();
  
}
/**************************************************************/
/**** Receive a message from the server                    ****/
/**************************************************************/
extern bool preEmptiveClientFire(const weapon_info*);
int NetClient::recvMsg( Packet* outpacket, timeval *timeout )
{
	/* // Returns false if no data hasarrived on the socket.
	   // Required if you only want to poll.
	if( !clt_sock.isActive( ) ) {
		return 0;
	}
	*/
	using namespace VSFileSystem;
    ObjSerial	packet_serial=0;

	static vector<Mount::STATUS> backupMountStatus;
	
    // Receive data
	Unit * un = NULL;
	int mount_num;
	ObjSerial mis=0;
	ObjSerial local_serial=0;
	if( this->game_unit.GetUnit() != NULL)
		local_serial = this->game_unit.GetUnit()->GetSerial();
	Cockpit * cp;

    Packet    p1;
    AddressIP ipadr;
/*				
				// Restart game!!!
				_Universe->Loop(bootstrap_first_loop);*/
	static bool udpgetspriority=true;
	bool wasudp=udpgetspriority;

	// First check if there is data in the client's recv queue.
	int recvbytes = (udpgetspriority?clt_udp_sock:clt_tcp_sock)->recvbuf( &p1, &ipadr );
	
    if( recvbytes <= 0) {
		recvbytes = (udpgetspriority==false?clt_udp_sock:clt_tcp_sock)->recvbuf( &p1, &ipadr );
		wasudp=!udpgetspriority;
	}
	
	if (recvbytes <= 0) {
		// Now, select and wait for data to come in the queue.
		clt_tcp_sock->addToSet( _sock_set );
		clt_udp_sock->addToSet( _sock_set );
		int socketstat = _sock_set.wait( timeout );
		if (!clt_tcp_sock->valid()) {
			perror( "Error socket closed ");
			clt_tcp_sock->disconnect( "socket error closed" );
			// NETFIXME: Error handling on socket error?  Exit?
			return -1;
		}
		if( socketstat < 0)
		{
			perror( "Error select -1 ");
			clt_tcp_sock->disconnect( "socket error recv err" );
			return -1;
		}
		if ( socketstat == 0 )
		{
//			COUT << "recvMsg socketstat == 0: " << (vsnetEWouldBlock()?"wouldblock":"") << endl;
			return -1;
			// timeout expired.
		}

		//	NETFIXME: check for file descriptors in _sock_set.fd_set...
		// Check the queues again.
		recvbytes = (udpgetspriority?clt_udp_sock:clt_tcp_sock)->recvbuf( &p1, &ipadr );
		wasudp=udpgetspriority;
		
		if( recvbytes <= 0) {
			recvbytes = (udpgetspriority==false?clt_udp_sock:clt_tcp_sock)->recvbuf( &p1, &ipadr );
			wasudp=!udpgetspriority;
		}
		udpgetspriority=!udpgetspriority;
		if( recvbytes <= 0)
		{
			// If nothing has come in either queue, and the select did not return 0, then this must be from a socket error.
			perror( "Error recv -1 ");
			clt_tcp_sock->disconnect( "socket error recv" );
			return -1;
		}
    }
	if (true) { //p1.getDataLength()>0) {

		bool nostarsystem = _Universe->activeStarSystem()==NULL?true:false;
		
		_Universe->netLock(true); // Don't bounce any commands back to the server again!
		
		NetBuffer netbuf( p1.getData(), p1.getDataLength());
	    if( outpacket )
	    {
	        *outpacket = p1;
	    }
        packet_serial     = p1.getSerial();
	    Cmd cmd           = p1.getCommand( );
		if (cmd!=CMD_SNAPSHOT)
			COUT << "Rcvd " << (wasudp?"UDP":"TCP") << ": " << cmd << " from serial " << packet_serial << endl;
        switch( cmd )
        {
            // Login accept
            case CMD_CONNECT:
			{
				this->server_netversion = netbuf.getSerial();
				string ipaddress = netbuf.getString();
				cout<< "Successful connection to VegaServer version " <<
					this->server_netversion << "from address " << ipaddress << endl;
			}
				break;
            case CMD_CHOOSESHIP :
				this->loginChooseShip( p1);
				break;
            case LOGIN_ACCEPT :
				this->loginAccept( p1);
            break;
			case CMD_ASKFILE :
			{
				// NETFIXME: Broken code... shouldn't write to client's stuff
				// Also, shouldn't open files Read-only and then write to them.
				// Also it shouldn't exit(1)...
				FILE * fp;
				string filename;
				string file;
				filename = netbuf.getString();
				file = netbuf.getString();
				// If packet serial == 0 then it means we have an up to date file
				if( packet_serial==local_serial)
				{
					// Receive the file and write it (trunc if exists)
					cerr<<"RECEIVING file : "<<filename<<endl;
					VSFile f;
					VSError err = f.OpenReadOnly(filename, ::VSFileSystem::UnknownFile);
					if (err>Ok)
					{
						cerr<<"!!! ERROR : opening received file !!!"<<endl;
						VSExit(1);
					}
					if( f.Write( file) != file.length())
					{
						cerr<<"!!! ERROR : writing received file !!!"<<endl;
						VSExit(1);
					}
					f.Close();
				}
				else
				{
					// Something is wrong
					//displayError( packet_serial);
				}
			}
			break;
            case CMD_DOWNLOAD :
				COUT << endl;
                if( _downloadManagerClient )
                {
                    _downloadManagerClient->processCmdDownload( *clt_tcp_sock, netbuf );
                }
                break;
            // Login failed
            case LOGIN_ERROR :
                COUT<<">>> LOGIN ERROR =( DENIED )= ------------------------------------------------"<<endl;
                this->disconnect();
				lastsave.push_back( "");
				lastsave.push_back( "!!! ACCESS DENIED : Account does not exist !!!");
                return -1;
                break;
			case LOGIN_UNAVAIL :
				COUT<<">>> ACCOUNT SERVER UNAVAILABLE ------------------------------------------------"<<endl;
				lastsave.push_back( "");
				lastsave.push_back( "!!! ACCESS DENIED : Account server unavailable !!!");
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
            case CMD_TXTMESSAGE:
              {
                string sender = netbuf.getString();
                string message = netbuf.getString();
                UniverseUtil::IOmessage(0,sender,"all",message);
              }
              break;

            case CMD_SNAPSHOT :
                {
					if (nostarsystem) break;
                    // Should update another client's position
                    // Zone hack:
	                // When receiving a snapshot, packet serial is considered as the
                    // number of client updates.
                    unsigned int numUnits  = p1.getSerial( );
                    unsigned int timestamp = p1.getTimestamp( );
                    double       deltatime = netbuf.getFloat( );

//                    COUT << "   *** #units=" << numUnits << " ts=" << timestamp << " delta-t=" << deltatime << endl;

                    this->receivePositions( numUnits, timestamp, netbuf, deltatime );
                }
                break;
            case CMD_ENTERCLIENT :
			{
				if (nostarsystem) break;
				// Saving 4 bytes for every 50kB saved game isn't worth the bugs that come with it.
//				if (p1.getSerial()) {
//					this->enterClient( netbuf, p1.getSerial() );
//				} else {
					this->AddObjects( netbuf);
//				}
			}
            break;
            case CMD_EXITCLIENT :
				if (nostarsystem) break;
                COUT << ">>> " << local_serial << " >>> EXITING CLIENT =( serial #"
                     << packet_serial << " )= --------------------------------------" << endl;
                this->removeClient( &p1 );
                break;
            case CMD_ADDEDYOU :
                {
					if (nostarsystem) break;
                    COUT << ">>> " << local_serial << " >>> ADDED IN GAME =( serial #"
                         << packet_serial << " )= --------------------------------------" << endl;
                    //now we have to make the unit if it is null (this would be a respawn)
                    if (this->game_unit.GetUnit()==NULL) {
                      this->Respawn(packet_serial);
                    }
                    this->game_unit.GetUnit()->curr_physical_state = netbuf.getTransformation();
                }
                break;
            case CMD_DISCONNECT :
                /*** TO REDO IN A CLEAN WAY ***/
                COUT << ">>> " << local_serial << " >>> DISCONNECTED -> Client killed =( serial #"
                     << packet_serial << " )= --------------------------------------" << endl;
				
                break;
//             case CMD_ACK :
//                 /*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
//                 /*** We must make sure those 2 conditions are enough ***/
//                 COUT << ">>> ACK =( " << latest_timestamp
//                      << " )= ---------------------------------------------------" << endl;
// 				p1.ack( );
//                 break;
			case CMD_FIREREQUEST :
			{
				if (nostarsystem) break;
				// WE RECEIVED A FIRE NOTIFICATION SO FIRE THE WEAPON
				float energy = netbuf.getFloat();
				mis = netbuf.getSerial();
				mount_num = netbuf.getInt32();
				// Find the unit
				if( p1.getSerial()==local_serial) // WE have fired and receive the broadcast
					un = this->game_unit.GetUnit();
				else
					un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un!=NULL)
				{
					// Set the concerned mount as ACTIVE and others as INACTIVE
					vector <Mount>
						::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
                                        if (mount_num>un->mounts.size())
                                          mount_num=un->mounts.size();
					int j;
					
					un->energy = energy; // It's important to set energy before firing.
					
					for (j=backupMountStatus.size();j<un->mounts.size();++j) {
						backupMountStatus.push_back(Mount::UNCHOSEN);
					}
					
					for (j=0;i!=un->mounts.end();++i,++j) {
						backupMountStatus[j]=(*i).status;
						if ((*i).status==Mount::ACTIVE)
							(*i).status=Mount::INACTIVE;
					}
                                        Cockpit *ps = _Universe->isPlayerStarship(un);
					for (j=0;j<mount_num;++j) {
						int mnt = netbuf.getInt32();
						if (mnt<un->mounts.size()&&mnt>=0) {
                                                  if (ps==NULL||!preEmptiveClientFire(un->mounts[mnt].type)) {
							un->mounts[mnt].processed=Mount::ACCEPTED;
							un->mounts[mnt].status=Mount::ACTIVE;
							// Store the missile id in the mount that should fire a missile
							un->mounts[mnt].serial=mis;
                                                  }
						}
					}
					// Ask for fire
					if( mis != 0)
						un->Fire(ROLES::FIRE_MISSILES|ROLES::EVERYTHING_ELSE,false);
					else
						un->Fire(ROLES::EVERYTHING_ELSE|ROLES::FIRE_GUNS,false);
					
					i = un->mounts.begin();
					for (j=0;i!=un->mounts.end();++i,++j) {
						(*i).status=backupMountStatus[j];
					}
				}
				else
					COUT<<"!!! Problem -> CANNOT FIRE UNIT NOT FOUND !!!"<<endl;
			}
			break;
			case CMD_UNFIREREQUEST :
				if (nostarsystem) break;
				// WE RECEIVED AN UNFIRE NOTIFICATION SO DEACTIVATE THE WEAPON
				mount_num = netbuf.getInt32();
//				mis = netbuf.getSerial();
				// Find the unit
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				if( un != NULL)
				{
					// Set the concerned mount as ACTIVE and others as INACTIVE
					vector <Mount>
						::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
					int j;
                                        if (mount_num>un->mounts.size())
                                          mount_num=un->mounts.size();

					for (j=backupMountStatus.size();j<un->mounts.size();++j) {
						backupMountStatus.push_back(Mount::UNCHOSEN);
					}
					
					for (j=0;i!=un->mounts.end();++i,++j) {
						backupMountStatus[j]=(*i).status;
						if ((*i).status==Mount::ACTIVE)
							(*i).status=Mount::INACTIVE;
					}
					
					for (j=0;j<mount_num;++j) {
						int mnt = netbuf.getInt32();
						if (mnt<un->mounts.size()&&mnt>=0) {
							un->mounts[mnt].processed=Mount::UNFIRED;
							un->mounts[mnt].status=Mount::ACTIVE;
							// Store the missile id in the mount that should fire a missile
							un->mounts[mnt].serial=0;//mis;
						}
					}
					// Ask for fire
					un->UnFire();
					
					i = un->mounts.begin();
					for (j=0;i!=un->mounts.end();++i,++j) {
						(*i).status=backupMountStatus[j];
					}
				}
				else
					COUT<<"!!! Problem -> CANNOT UNFIRE UNIT NOT FOUND !!!"<<endl;

			break;
			case CMD_TARGET:
				if (nostarsystem) break;
				un = UniverseUtil::GetUnitFromSerial( packet_serial );
				if (un) {
					unsigned short targserial = netbuf.getSerial();
                                        Unit * target_un = UniverseUtil::GetUnitFromSerial( targserial );
										if (target_un) {
											COUT<<"Confirmed targeting unit "<<target_un->name<<" ("<<targserial<<")."<<endl;
										}
                                        Unit* oldtarg=un->Target();
                                        if (oldtarg&&oldtarg->GetSerial()==0&&(target_un==NULL||target_un->GetSerial()==0)) {
											COUT <<"Setting target from " << oldtarg->name << " to NULL." << endl;
                                          //don't do anything
                                        }else {
                                          un->computer.target.SetUnit(target_un);
                                        }
				}
			break;
			case CMD_SCAN :
				if (nostarsystem) break;
				// We received the target info with the target serial in the packet as an answer to a scanRequest

				// Update info with received buffer

				// And tell all VDUs we received the target info
				cp = _Universe->isPlayerStarship( this->game_unit.GetUnit());
				cp->ReceivedTargetInfo();
			break;
			case CMD_SNAPDAMAGE :
			{
				if (nostarsystem) break;
				// In case we use damage snapshots : we do not call ApplyNetDamage
				// In fact we trusted the client only for visual FX : Check where they are done !
				// but the server computes the damage itself

				// SHOULD READ THE DAMAGE SNAPSHOT HERE !
				int nbupdates = packet_serial;
				ObjSerial serial;
                                int offset=netbuf.getOffset();
				for( int i=0; i<nbupdates; i++)
				{
                                  
					serial = netbuf.getSerial();
                                        int noffset=netbuf.getOffset();
                                        if (noffset==offset) {
                                          COUT << "ERROR Premature end of Snapshot buffer "<<std::hex<<std::string(netbuf.getData(),netbuf.getSize()) << std::dec << std::endl;
                                          break;
                                        }
                                        offset=noffset;
					Unit *un = UniverseUtil::GetUnitFromSerial(serial);
					receiveUnitDamage( netbuf, un );
				}
			}
			break;
#if 1
			case CMD_DAMAGE :
			{
				if (nostarsystem) break;
				float amt = netbuf.getFloat();
				float ppercentage = netbuf.getFloat();
				float spercentage = netbuf.getFloat();
				Vector pnt = netbuf.getVector();
				Vector normal = netbuf.getVector();
				GFXColor col = netbuf.getColor();
				un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				float hul = netbuf.getFloat();
				Shield sh = netbuf.getShield();
				Armor ar = netbuf.getArmor();
				if( un)
				{
					// Apply the damage
					un->ApplyNetDamage( pnt, normal, amt, ppercentage, spercentage, col);
					un->shield=sh;
					un->armor=ar;
					un->hull=hul;
				}
				else
					COUT<<"!!! Problem -> CANNOT APPLY DAMAGE UNIT NOT FOUND !!!"<<endl;
			}
			break;
#endif
#if 0
			case CMD_DAMAGE1 :
			{
				/*
			    if (nostarsystem) break;
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
			{
				if (nostarsystem) break;
				ClientPtr clt = Clients.get( p1.getSerial());
				// If it is not a player
				if( !clt)
				{
					un = UniverseUtil::GetUnitFromSerial( p1.getSerial());
					if( un)
					{
						un->Destroy();
					}
					else
						COUT<<"!!! Problem -> CANNOT KILL UNIT NOT FOUND !!!"<<endl;
				}
				else
				{
					un = clt->game_unit.GetUnit();
					//Remove the player unit
					nbclients--;
					Clients.remove(p1.getSerial());
					if (un) {
						un->Destroy();
					}
					COUT<<"Client #"<<p1.getSerial()<<" killed - now "<<nbclients<<" clients in system"<<endl;

					string msg = clt->callsign+" has died.";
					UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
				}
			}
			break;
			case CMD_JUMP :
				if (nostarsystem) break;
			if (1) {
                          unsigned short port;
                          std::string srvipadr;
                          SetConfigServerAddress(srvipadr,port);
                          Reconnect(srvipadr,XMLSupport::tostring((unsigned int)port));
                        }else{//this is the old way of doing it
				StarSystem * sts;
				string newsystem = netbuf.getString();
				ObjSerial unserial = netbuf.getSerial();
				ObjSerial jumpserial = netbuf.getSerial();
				unsigned short zoneid = netbuf.getShort();
				un = this->game_unit.GetUnit();
				if (!un)
					break;
				// Get the pointer to the new star system sent by server
				if( !(sts=star_system_table.Get( newsystem)))
				{
					// The system should have been loaded just before we asked for the jump so this is just a safety check
					cerr<<"!!! FATAL ERROR : Couldn't find destination Star system !!!"<<endl;
					sts = _Universe->GenerateStarSystem( newsystem.c_str(), "", Vector(0,0,0) );
				}
				// If unserial == un->GetSerial() -> then we are jumping otherwise it is another unit/player
				if( unserial == un->GetSerial())
				{
					this->zone = zoneid;
					// If we received a CMD_JUMP with serial==player serial jump is granted
					if( packet_serial==un->GetSerial())
					{
						this->jumpok = true;
						this->ingame = false;
					}
					// The jump has been allowed but we don't have the good system file
					else
					{
						// Here really do the jump function
						Unit * jumpun = UniverseUtil::GetUnitFromSerial( jumpserial);
						sts->JumpTo( un, jumpun, newsystem, true);
						string sysfile( newsystem+".system");
						VsnetDownload::Client::NoteFile f( *this->clt_tcp_sock, sysfile, SystemFile);
   		             	_downloadManagerClient->addItem( &f);
						
						timeval timeout = {10, 0};
						
						while( !f.done())
						{
							if (recvMsg( NULL, &timeout )<=0) {
//NETFIXME: What to do if the download times out?
								COUT << "recvMsg <=0: " << (vsnetEWouldBlock()?"wouldblock":"") << endl;
								break;
							}
						}
						this->jumpok = true;
					}
				}
				else
				{
					// If another player / unit is jumping force it
					Unit * jumpun = UniverseUtil::GetUnitFromSerial( jumpserial);
					sts->JumpTo( un, jumpun, newsystem, true);
				}
			}
			break;
			case CMD_SNAPCARGO:
			{
				if (nostarsystem) break;
				ObjSerial ser;
				Unit *mpl = UnitFactory::getMasterPartList();
				while ((ser = netbuf.getSerial())!=0) {
					Unit *un = UniverseUtil::GetUnitFromSerial( ser );
					unsigned int i;
					// Clear cargo... back to front to make it more efficient.
					if (un) {
						i=un->numCargo();
						while (i>0) {
							i--;
							un->RemoveCargo(i, un->GetCargo(i).GetQuantity(),true);
						}
					}
					float mass = netbuf.getFloat();
					float cargvol = netbuf.getFloat();
					float upgvol = netbuf.getFloat();
					if (un) {
						un->Mass = mass;
						un->image->CargoVolume = cargvol;
						un->image->UpgradeVolume = upgvol;
					}
					unsigned int numcargo = netbuf.getInt32();
					Cargo carg;
					for (i=0;i<numcargo;i++) {
						unsigned int mplind;
						unsigned int quantity = netbuf.getInt32();
						string str=netbuf.getString();
						if (un) {
							carg = *mpl->GetCargo(str.c_str(), mplind);
						}
						carg.SetQuantity(quantity);
						carg.SetPrice(netbuf.getFloat());
						carg.SetMass(netbuf.getFloat());
						carg.SetVolume(netbuf.getFloat());
						if (un) {
							un->AddCargo(carg,false);
						}
					}
				}
			}
			break;
			case CMD_CARGOUPGRADE:
			{
				if (nostarsystem) break;
				ObjSerial buyer_ser = netbuf.getSerial();
				ObjSerial seller_ser = netbuf.getSerial();
				int quantity = netbuf.getInt32();
				std::string cargoName = netbuf.getString();
				float price = netbuf.getFloat();
				float mass = netbuf.getFloat();
				float volume = netbuf.getFloat();
				int mountOffset = ((int)netbuf.getInt32());
				int subunitOffset = ((int)netbuf.getInt32());
				Unit *sender = UniverseUtil::GetUnitFromSerial( packet_serial);
				Unit *buyer = UniverseUtil::GetUnitFromSerial( buyer_ser);
				Unit *seller = UniverseUtil::GetUnitFromSerial( seller_ser);
				bool missioncarg=false;
				
				unsigned int cargIndex = 0;
				Cargo *cargptr=NULL;
				if (!sender) {
					break;
				}
				if (seller) {
					cargptr = seller->GetCargo(cargoName, cargIndex);
				}
				if (!cargptr) {
					cargptr = GetMasterPartList(cargoName.c_str());
					if (!cargptr) {
						break;
					}
				}
				Cargo carg = *cargptr;
				bool upgrade=false;
				bool repair=true;
				if (carg.GetCategory().find("upgrades")==0) {
					upgrade=true;
				}
				if (upgrade && !quantity) {
					repair=true;
				}
				if (!upgrade) {
					missioncarg = (mountOffset==1 && subunitOffset==1);
				}
				carg.mass = mass;
				carg.price = price;
				carg.volume = volume;
				carg.mission = missioncarg;
				if (quantity) {
					if (buyer) {
						carg.SetQuantity(quantity);
						buyer->AddCargo(carg, true);
					}
					if (seller) {
						seller->RemoveCargo(cargIndex, quantity, true);
					}
				}
				if (upgrade && !repair && (seller==sender || buyer==sender)) {
					double percent; // not used.
					int multAddMode = GetModeFromName(carg.GetContent().c_str());
					
					// Now we're sure it's an authentic upgrade...
					// Wow! So much code just to perform an upgrade!
					const string unitDir = GetUnitDir(sender->name.get().c_str());
					string templateName;
					int faction=0;
					if (seller==sender) {
						templateName = unitDir + ".blank";
						faction = seller->faction;
					} else if (buyer==sender) {
						faction = buyer->faction;
						templateName = unitDir + ".template";
					}
					const Unit *unitCarg = getUnitFromUpgradeName(carg.GetContent(), faction);
					if (!unitCarg) break; // not an upgrade, and already did cargo transactions.
					// Get the "limiter" for the upgrade.  Stats can't increase more than this.
					const Unit * templateUnit = UnitConstCache::getCachedConst(StringIntKey(templateName,faction));
					if (!templateUnit) {
						templateUnit = UnitConstCache::setCachedConst(StringIntKey(templateName,faction),
							UnitFactory::createUnit(templateName.c_str(),true,faction));
					}
					if (templateUnit->name == LOAD_FAILED) {
						templateUnit=NULL;
					}
					if(unitCarg->name == LOAD_FAILED) {
						break;
					}
					
					if (seller==sender) {
						// Selling it... Downgrade time!
						if (seller->canDowngrade(unitCarg, mountOffset, subunitOffset, percent, templateUnit)) {
							seller->Downgrade (unitCarg, mountOffset, subunitOffset, percent, templateUnit);
						}
					} else if (buyer==sender) {
						// Buying it... Upgrade time!
						if (buyer->canUpgrade(unitCarg, mountOffset, subunitOffset, multAddMode, true, percent, templateUnit)) {
							buyer->Upgrade   (unitCarg, mountOffset, subunitOffset, multAddMode, true, percent, templateUnit);
						}
					}
				}
				if (repair) {
					sender->RepairUpgradeCargo(&carg, seller, NULL);
				}
				Unit *player = game_unit.GetUnit();
				if (player && ((buyer&&buyer->isDocked(player)) || (seller&&seller->isDocked(player)) || player==buyer || player==seller)) {
					BaseUtil::refreshBaseComputerUI(&carg);
				}
				break;
			}
			case CMD_CREDITS:
				cp = _Universe->isPlayerStarship( this->game_unit.GetUnit());
				if (cp) {
					cp->credits=netbuf.getFloat();
				}
				//BaseUtil::refreshBaseComputerUI(NULL);
			break;
			case CMD_STARTNETCOMM :
#ifdef NETCOMM
			{
				float freq = netbuf.getFloat();
				char  secured = netbuf.getChar();
				char webc = netbuf.getChar();
				char pa = netbuf.getChar();

				if( freq == current_freq)
				{
					if( secured==NetComm->IsSecured())
					{
						ClientPtr clt;
						// Check this is not us
						if( packet_serial != this->serial)
						{
							// Add the client to netcomm list in NetComm ?
							clt = Clients.get(packet_serial);
							clt->webcam = webc;
							clt->portaudio = pa;
							NetComm->AddToSession( clt);
						}
					}
					else
						cerr<<"WARNING : Received a STARTCOMM from a channel not in the same mode"<<endl;
				}
				else
					cerr<<"WARNING : Received a STARTCOMM from another frequency"<<endl;
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
			case CMD_SOUNDSAMPLE :
#ifdef NETCOMM
			{
				NetComm->RecvSound( p1.getData(), p1.getDataLength(), false);
			}
#endif
			break;
			case CMD_SECSNDSAMPLE :
#ifdef NETCOMM
			{
				NetComm->RecvSound( p1.getData(), p1.getDataLength(), true);
			}
#endif
			break;
#if 0
                        //NETFIXME  this is probably more consistent
			case CMD_TXTMESSAGE :
#ifdef NETCOMM
			{
				string msg( p1.getData());
				NetComm->RecvMessage( msg, false);
			}
#endif
#endif
			case CMD_SECMESSAGE :
#ifdef NETCOMM
			{
				string msg( p1.getData());
				NetComm->RecvMessage( msg, true);
			}
#endif
			break;
			case CMD_DOCK :
			{
				if (nostarsystem) break;
				ObjSerial utdw_serial = netbuf.getSerial();
				int dockport = netbuf.getInt32();
				cerr<<"RECEIVED A DOCK AUTHORIZATION for unit "<<p1.getSerial()<<" to unit "<<utdw_serial<<" at docking port #"<<dockport<<endl;
				un = UniverseUtil::GetUnitFromSerial( utdw_serial);
				Unit * un2 = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				un->RequestClearance(un2);
				un2->ForceDock( un, dockport);
			}
			break;
			case CMD_UNDOCK :
			{
				if (nostarsystem) break;
				ObjSerial utdw_serial = netbuf.getSerial();
				cerr<<"RECEIVED A UNDOCK ORDER for unit "<<p1.getSerial()<<" to unit "<<utdw_serial<<endl;
				un = UniverseUtil::GetUnitFromSerial( utdw_serial);
				Unit * un2 = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				un2->UnDock( un);
			}
			break;
			case CMD_POSUPDATE :
			{
				if (nostarsystem) break;
				// If a client receives that it means the server want to force the client position to be updated
				// with server data
				QVector serverpos = netbuf.getQVector();
				un = this->game_unit.GetUnit();
				if (!un)
					break;
				un->old_state.setPosition( serverpos);
				un->curr_physical_state.position = serverpos;
			}
			break;
			case CMD_SERVERTIME:
			break;
            default :
                COUT << ">>> " << local_serial << " >>> UNKNOWN COMMAND =( " << std::hex << cmd
                     << " )= --------------------------------------" << std::endl;
                keeprun = 0;
                this->disconnect();
        }
		_Universe->netLock(false);
		
    }
    return recvbytes;
}

/*************************************************************/
/**** Disconnect from the server                          ****/
/*************************************************************/

void	NetClient::disconnect()
{
	keeprun = 0;
	// Disconnection is handled in the VSExit(1) function for each player
}

SOCKETALT*	NetClient::logout(bool leaveUDP)
{
	keeprun = 0;
	Packet p;
	Unit *un = this->game_unit.GetUnit();
	if (un) {
		p.send( CMD_LOGOUT, un->GetSerial(),
            (char *)NULL, 0,
            SENDRELIABLE, NULL, *this->clt_tcp_sock,
            __FILE__, PSEUDO__LINE__(1382) );
	}
	clt_tcp_sock->disconnect( "Closing connection to server" );
        if (!leaveUDP) {
          NetUIUDP::disconnectSaveUDP(*clt_udp_sock);
//          clt_udp_sock->disconnect( "Closing UDP connection to server");
        }else {
          if (lossy_socket==clt_udp_sock) {
            return clt_udp_sock;
          }
        }
        return NULL;
}

void NetClient::CleanUp() {
	if (Network) {
		for (int i=0;i<_Universe->numPlayers();i++) {
			Network[i].logout(false);
		}
		delete [] Network;
		Network = NULL;
	}
}

void NetClient::Reconnect(std::string srvipadr, std::string port) {
  vector<string> usernames;
  vector<string> passwords;
  vector <SOCKETALT*> udp;
  unsigned int i;
  if (!Network) {
    Network = new NetClient[_Universe->numPlayers()];
  }
  for (i=0;i<_Universe->numPlayers();++i) {
    usernames.push_back(Network[i].callsign);
    passwords.push_back(Network[i].password);
    SOCKETALT * udpsocket=Network[i].logout(true);
    if (udpsocket)
      udp.push_back(udpsocket);
    else
      udp.push_back(NULL);
    Network[i].disconnect();
  }
  _Universe->clearAllSystems();
  localSerials.resize(0);

  for (i=0;i<_Universe->numPlayers();++i) {
    Network[i].Reinitialize();
  }
  UniverseUtil::showSplashScreen("");
  //necessary? usually we would ask acctserver for it .. or pass it in NetClient::getConfigServerAddress(srvipadr, port);
  for (unsigned int k=0;k<_Universe->numPlayers();++k) {
    bool ret = false;
	string err;
	int response = Network[k].connectLoad( usernames[k], passwords[k], err);
	if (response==0) {
		COUT<<"Network login error: "<<err<<endl;
		UniverseUtil::startMenuInterface(false, "Jumping to system, but got a login error: "+err);
		return;
	}
    vector<string> *loginResp = Network[k].loginSavedGame(0);
	if (!loginResp) {
		COUT<<"Failed to get a ship";
		UniverseUtil::startMenuInterface(false,"Jumping to system, but failed to get a ship");
		return;
	}
	cout<<" logged in !"<<endl;
	Network[k].Respawn(Network[k].serial);
	Network[k].synchronizeTime(udp[k]);
	_Universe->AccessCockpit(k)->TimeOfLastCollision=getNewTime();
    Network[k].inGame();
  }
  UniverseUtil::hideSplashScreen();
}

ClientPtr NetClient::ClientsMap::insert( int x, Client* c )
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

ClientPtr NetClient::ClientsMap::get( int x )
{
    ClientIt it = _map.find(x);
    if( it == _map.end() ) return ClientPtr();
    return it->second;
}

bool NetClient::ClientsMap::remove( int x )
{
    size_t s = _map.erase( x );
    if( s == 0 ) return false;
    return true;
    // shared_ptr takes care of delete
}

Transformation	NetClient::Interpolate( Unit * un, double addtime)
{
	if (!un) return Transformation();
//	return un->curr_physical_state;
// NETFIXME: Interpolation is kind of borked...?
	ClientPtr clt=Clients.get(un->GetSerial());
	Transformation trans;
	if (clt) {
		clt->elapsed_since_packet += addtime;
		trans=clt->prediction->Interpolate( un, clt->elapsed_since_packet);
//		cerr << "  *** INTERPOLATE (" << un->curr_physical_state.position.i << ", " << un->curr_physical_state.position.j << ", " << un->curr_physical_state.position.k << "): next deltatime=" << clt->getNextDeltatime() << ", deltatime=" << clt->getDeltatime() << ", this-deltatime=" << this->deltatime << ", elapsed since packet=" << clt->elapsed_since_packet << "\n        =>        (" << trans.position.i << ", " << trans.position.j << ", " << trans.position.k << ")        Vel =    (" << un->Velocity.i << ", " << un->Velocity.j << ", " << un->Velocity.k << ")" << std::endl;
	} else {
		trans=un->curr_physical_state;
                if (rand()<RAND_MAX/5000)
                  cerr << "  *** Interpolate with NULL CLIENT serial " << un->GetSerial() << "!  Unit fullname=" << un->getFullname() << ";  name=" << un->name.get() << endl;
	}
	return trans;
}

