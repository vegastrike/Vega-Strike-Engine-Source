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
#include "posh.h"
#include "prediction.h"

#ifdef micro_sleep
#undef micro_sleep
#endif
#define micro_sleep(m) _sock_set.waste_time( 0, m )

using std::cout;
using std::endl;
using std::cin;

double NETWORK_ATOM;
extern vector<string> globalsaves;
extern vector<unorigdest *> pendingjump;
extern Hashtable<std::string, StarSystem, char[127]> star_system_table;
typedef vector<Client *>::iterator VC;
typedef vector<ObjSerial>::iterator ObjI;

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

	prediction = new CubicSplinePrediction();
#ifdef NETCOMM
	NetComm = new NetworkCommunication();
#else
    NetComm = NULL;
#endif

    _downloadManagerClient.reset( new VsnetDownload::Client::Manager( _sock_set ) );
    _sock_set.addDownloadManager( _downloadManagerClient );

#ifdef CRYPTO
	cout<<endl<<endl<<POSH_GetArchString()<<endl;
#endif
}

NetClient::~NetClient()
{
	if( prediction)
		delete prediction;
#ifdef NETCOMM
	if( NetComm!=NULL)
		delete NetComm;
#endif
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

/*************************************************************/
/**** Launch the client                                   ****/
/*************************************************************/

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
		// Here also send samples
		NetComm->SendSound( this->clt_sock, this->serial);
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
				this->loginAccept( p1);
            break;
			case CMD_ASKFILE :
			{
				FILE * fp;
				string filename;
				string file;
				filename = netbuf.getString();
				file = netbuf.getString();
				// If packet serial == 0 then it means we have an up to date file
				if( packet_serial==this->game_unit.GetUnit()->GetSerial())
				{
					// Receive the file and write it (trunc if exists)
					cerr<<"RECEIVING file : "<<filename<<endl;
					fp = fopen( (datadir+filename).c_str(), "wb");
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
					//displayError( packet_serial);
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
					this->game_unit.GetUnit()->SetCurPosition( netbuf.getQVector());
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
				Shield sh = netbuf.getShield();
				Armor ar = netbuf.getArmor();
				if( un)
				{
					un->shield = sh;
					un->armor = ar();
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
			{
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
					//Remove the player unit
					Unit * un = clt->game_unit.GetUnit();
					_Universe->activeStarSystem()->RemoveUnit(clt->game_unit.GetUnit());
					nbclients--;
					Clients.remove(p1.getSerial());
					un->Destroy();
					COUT<<"Client n°"<<p1.getSerial()<<" killed - now "<<nbclients<<" clients in system"<<endl;

					string msg = clt->callsign+" was killed";
					UniverseUtil::IOmessage(0,"game","all","#FFFF66"+msg+"#000000");
				}
			}
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
						VsnetDownload::Client::NoteFile f( this->clt_sock, newsystem);
   	             		_downloadManagerClient->addItem( &f);
						while( !f.done())
						{
							checkMsg( NULL);
							micro_sleep( 40000);
						}
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
			case CMD_TXTMESSAGE :
#ifdef NETCOMM
			{
				string msg( p1.getData());
				NetComm->RecvMessage( msg, false);
			}
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
				ObjSerial utdw_serial = netbuf.getSerial();
				int dockport = netbuf.getInt32();
				cerr<<"RECEIVED A DOCK AUTHORIZATION for unit "<<p1.getSerial()<<" to unit "<<utdw_serial<<" at docking port #"<<dockport<<endl;
				un = UniverseUtil::GetUnitFromSerial( utdw_serial);
				Unit * un2 = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				un2->ForceDock( un, dockport);
			}
			break;
			case CMD_UNDOCK :
			{
				ObjSerial utdw_serial = netbuf.getSerial();
				cerr<<"RECEIVED A UNDOCK ORDER for unit "<<p1.getSerial()<<" to unit "<<utdw_serial<<endl;
				un = UniverseUtil::GetUnitFromSerial( utdw_serial);
				Unit * un2 = UniverseUtil::GetUnitFromSerial( p1.getSerial());
				un2->UnDock( un);
			}
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
            (char *)NULL, 0,
            SENDRELIABLE, NULL, this->clt_sock,
            __FILE__, PSEUDO__LINE__(1382) );
	clt_sock.disconnect( "Closing connection to server", false );
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

