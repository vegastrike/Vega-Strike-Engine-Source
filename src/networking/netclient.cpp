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
#include "vs_globals.h"
#include "endianness.h"
#include "cmd/unit_generic.h"
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

#include "vsnet_clientstate.h"
#include "vegastrike.h"
#include "client.h"
#include "networking/netbuffer.h"

using std::cout;
using std::endl;
using std::cin;

double NETWORK_ATOM;
vector<string> globalsaves;

/*************************************************************/
/**** Tool funcitons                                      ****/
/*************************************************************/

typedef vector<ObjSerial>::iterator ObjI;
vector<ObjSerial>	localSerials;
bool isLocalSerial( ObjSerial sernum)
{
	cout<<"Looking for serial : "<<sernum<<" in ";
	bool ret=false;
	for( ObjI i=localSerials.begin(); !ret && i!=localSerials.end(); i++)
	{
		cout<<(*i)<<", ";
		if( sernum==(*i))
			ret = true;
	}
	cout<<endl;

	return ret;
}

Unit * getNetworkUnit( ObjSerial cserial)
{
	for( int i=0; i<_Universe->numPlayers(); i++)
	{
		if( Network[i].getSerial() == cserial)
			return Network[i].getUnit();
	}
	return NULL;
}

NetClient::~NetClient()
{
    /*
	for( int i=0; i<MAXCLIENTS; i++)
    {
        if( Clients[i]!=NULL)
            delete Clients[i];
    }
	*/
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

		packet2.send( CMD_LOGIN, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			127
#endif
			);
		COUT << "Send login for player <" << str_callsign << ">:< "<< str_passwd
		     << "> - buffer length : " << packet2.getDataLength()
             << " (+" << packet2.getHeaderLength() << " header len" <<endl;
	}
	else
	{
		cout<<"Callsign and/or password not specified in vegastrike.config, please check this."<<endl<<endl;
		return -1;
	}

	return 0;
}

/*************************************************************/
/**** Login loop                                          ****/
/*************************************************************/

vector<string>	NetClient::loginLoop( string str_callsign, string str_passwd)
{
	COUT << "enter " << "NetClient::loginLoop" << endl;

	Packet	packet2;
	vector<string> savefiles;
	NetBuffer netbuf;

	//memset( buffer, 0, tmplen+1);
	netbuf.addString( str_callsign);
	netbuf.addString( str_passwd);

	COUT << "Buffering to send with CMD_LOGIN: " << endl;
	PacketMem m( netbuf.getData(), netbuf.getDataLength(), PacketMem::LeaveOwnership );
	m.dump( cout, 3 );

	packet2.send( CMD_LOGIN, 0, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			167
#endif
		);
	COUT << "Sent login for player <" << str_callsign << ">:<" << str_passwd
		 << ">" << endl
	     << "   - buffer length : " << packet2.getDataLength() << endl
	     << "   - buffer: " << netbuf.getData() << endl;
	// Now the loop
	int timeout=0, recv=0, ret=0;
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
		//cout<<elapsed<<" seconds since login request"<<endl;
		if( elapsed > login_to)
		{
			cout<<"Timed out"<<endl;
			timeout = 1;
		}
		ret=this->checkMsg( NULL, &packet );
		if( ret>0)
		{
			cout<<"Got a response"<<endl;
			recv = 1;
		}
		else if( ret<0)
		{
			cout<<"Error, dead connection to server"<<endl;
			recv=-1;
		}

		micro_sleep( 40000);
	}
	cout<<"End of login loop"<<endl;
	if( ret>0 && packet.getCommand()!=LOGIN_ERROR && packet.getCommand()!=LOGIN_UNAVAIL)
	{
		this->callsign = str_callsign;
		savefiles = globalsaves;
	}
	return savefiles;
}

/*************************************************************/
/**** Initialize the client network                       ****/
/*************************************************************/

SOCKETALT	NetClient::init( char * addr, unsigned short port)
{
    COUT << " enter " << __PRETTY_FUNCTION__
	     << " with " << addr << ":" << port << endl;

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
	    this->clt_sock = NetUITCP::createSocket( addr, port );
	}
	else
	{
	    this->clt_sock = NetUIUDP::createSocket( addr, port );
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
	    this->clt_sock = NetUITCP::createSocket( addr, port );
	}
	else
	{
	    this->clt_sock = NetUIUDP::createSocket( addr, port );
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
		this->checkMsg( NULL, NULL );
		micro_sleep( 30000);
	}

	this->disconnect();
}

void	NetClient::checkKey()
{
	/*
	fd_set	fd_keyb;
	int		s;
	char	c;

	struct timeval tout;
	tout.tv_sec = 0;
	tout.tv_usec = 0;

	FD_ZERO( &fd_keyb);
	FD_SET( 0, &fd_keyb);

	if( (s = select( 1, &fd_keyb, NULL, NULL, &tout))<0)
		perror( "Error reading standard input ");
	if( s>0)
	{
		if( read( 0, &c, 1)==-1)
			perror( "Error reading char on std input ");
		if( c != 0x0a)
		{
			if( c == 'Q' || c == 'q')
			{
				keeprun = 0;
			}
			else if( serial!=0)
			{
				Packet	packet2;
				packet2.send( CMD_POSUPDATE, this->serial, &c, sizeof(char), SENDRELIABLE, NULL, clt_sock, __FILE__, __LINE__ );
			}
		}
	}
*/
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
	//cout<<"cur_time="<<cur_time<<" - elapsed="<<GetElapsedTime()<<endl;
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

int NetClient::checkMsg( char* netbuffer, Packet* packet )
{
    int ret=0;

    timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    SocketSet set;
    clt_sock.watch( set );
    if( set.select( &t ) > 0 )
    {
        if( clt_sock.isActive( set ) )
        {
            ret = recvMsg( netbuffer, packet );
        }
    }
    return ret;
}

/**************************************************************/
/**** Receive a message from the server                    ****/
/**************************************************************/

int NetClient::recvMsg( char* netbuffer, Packet* outpacket )
{
    ObjSerial	packet_serial=0;

    // Receive data
    AddressIP sender_adr;
	PacketMem mem;
	Unit * un = NULL;
	int mount_num;
	ObjSerial mis;
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
                cout << ">>> " << this->serial << " >>> LOGIN ACCEPTED =( serial n°"
                     << packet_serial << " )= --------------------------------------"
					 << endl;
                // Should receive player's data (savegame) from server if there is a save
                this->serial = packet_serial;
                localSerials.push_back( this->serial);
				// Go forward to go after the 2 strings (name, passwd)
				netbuf.getString();
				netbuf.getString();
				globalsaves.push_back( netbuf.getString());
				globalsaves.push_back( netbuf.getString());
				//globalsaves = FileUtil::GetSaveFromBuffer( p1.getData()+2*NAMELEN);
                break;
            // Login failed
            case LOGIN_ERROR :
                cout<<">>> LOGIN ERROR =( DENIED )= ------------------------------------------------"<<endl;
                //cout<<"Received LOGIN_ERROR"<<endl;
                this->disconnect();
                return -1;
                break;
			case LOGIN_UNAVAIL :
				cout<<">>> ACCOUNT SERVER UNAVAILABLE ------------------------------------------------"<<endl;
				this->disconnect();
				return -1;
				break;
            // Create a character
            case CMD_CREATECHAR :
				cout << endl;
                // Should begin character/ship creation process
                //this->createChar();
                break;
            // Receive start locations
            case CMD_LOCATIONS :
				cout << endl;
                // Should receive possible starting locations list
                this->receiveLocations( &p1 );
                break;
            case CMD_SNAPSHOT :
				cout << endl;
                // Should update another client's position
                //cout<<"Received a SNAPSHOT from server"<<endl;
                this->receivePosition( &p1 );
                break;
            case CMD_ENTERCLIENT :
                cout << ">>> " << this->serial << " >>> ENTERING CLIENT =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                this->addClient( &p1 );
                break;
            case CMD_EXITCLIENT :
                cout << ">>> " << this->serial << " >>> EXITING CLIENT =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                this->removeClient( &p1 );
                break;
            case CMD_ADDEDYOU :
                cout << ">>> " << this->serial << " >>> ADDED IN GAME =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
				// Get the zone id in the packet
				this->zone = netbuf.getInt32();
                //this->getZoneData( &p1 );
                break;
            case CMD_DISCONNECT :
                /*** TO REDO IN A CLEAN WAY ***/
                cout << ">>> " << this->serial << " >>> DISCONNECTED -> Client killed =( serial n°"
                     << packet_serial << " )= --------------------------------------" << endl;
                exit(1);
                break;
            case CMD_ACK :
                /*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
                /*** We must make sure those 2 conditions are enough ***/
                cout << ">>> ACK =( " << current_timestamp
                     << " )= ---------------------------------------------------" << endl;
				p1.ack( );
                break;
			case CMD_FIREREQUEST :
				// WE RECEIVED A FIRE NOTIFICATION SO FIRE THE WEAPON
				mount_num = netbuf.getInt32();
				mis = netbuf.getSerial();
				// Find the unit
				//Unit * un = zonemgr->getUnit( packet.getSerial(), zone);
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

			break;
			case CMD_TARGET :
				// We received the target info with the target serial in the packet

				// Update info with received buffer

				// And tell all VDUs we received the target info
				cp = _Universe->isPlayerStarship( this->game_unit.GetUnit());
				cp->ReceivedTargetInfo();
			break;
            default :
                cout << ">>> " << this->serial << " >>> UNKNOWN COMMAND =( " << hex << cmd
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
	/*
	unsigned short nbclts;
	ClientState cs;
	int		state_size=sizeof( ClientState);
	int		desc_size=sizeof( ClientDescription);
	ObjSerial nser, nser2 = 0;
	int		offset=0;

	offset = sizeof( unsigned short);
	const char* buf = packet->getData();
	nbclts = *((const unsigned short *) buf);
	nbclts = ntohs( nbclts);
	for( int i=0; i<nbclts; i++)
	{
		memcpy( &cs, buf+offset, state_size);
		offset += state_size;
		memcpy( &cd, buf+offset, desc_size);
		offset += desc_size;
		cs.netswap();
		nser2 = cs.getSerial();
		nser = nser2;
		nser = ntohs( nser2);
		if( nser != this->serial && !isLocalSerial( nser))
		{
			cout<<"NEW CLIENT - ";
			cs.display();
			Clients[nser]->serial = nser;
			//memcpy( &Clients[nser]->current_state, &cs, state_size);
			Clients[nser]->current_state = cs;
			memcpy( &Clients[nser]->current_desc, &cd, desc_size);
			// Launch the unit in the game
			Clients[nser]->game_unit.SetUnit( UniverseUtil::launch (string(""),"avenger",string(""),string( "unit"), string("default"),1,0, cs.getPosition(), string("")));
			Clients[nser]->game_unit.GetUnit()->PrimeOrders();
			Clients[nser]->game_unit.GetUnit()->SetNetworkMode( true);

			// Assign new coordinates to client
			Clients[nser]->game_unit.GetUnit()->SetOrientation( cs.getOrientation());
			Clients[nser]->game_unit.GetUnit()->SetVelocity( cs.getVelocity());
			Clients[nser]->game_unit.GetUnit()->SetNetworkMode( true);
			// In that case, we want cubic spline based interpolation
			//init_interpolation( nser);
		}
		// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
		else if( nser!=this->serial)
		{
			Clients[nser] = new Client;
			cout<<"IT'S ANOTHER LOCAL PLAYER ";
			Clients[nser]->game_unit.SetUnit( getNetworkUnit( nser));
			assert( Clients[nser]->game_unit.GetUnit() != NULL);
			cs.display();
		}
	}
	*/
}

/*************************************************************/
/**** Adds an entering client in the actual zone          ****/
/*************************************************************/

void	NetClient::addClient( const Packet* packet )
{
	ObjSerial cltserial = packet->getSerial();
	Client * clt;
	// NOTE : in splitscreen mode we may receive info about us so we comment the following test
	/*
	if( cltserial==this->serial)
	{
		// Maybe just ignore here
		cout<<"FATAL ERROR : RECEIVED INFO ABOUT ME !"<<endl;
		exit(1);
	}
	*/

	clt = Clients[cltserial];
	if( clt != NULL)
	{
		cout<<"Error, client exists !!"<<endl;
		exit( 1);
	}

	clt = new Client;
	nbclients++;
	// Copy the client state in its structure
	clt->serial = cltserial;

	cout<<"New client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	cout<<"At : ";

	//Packet ptmp = packet;
	NetBuffer netbuf( packet->getData(), packet->getDataLength());
	// Should receive the name
	clt->name = netbuf.getString();
	//ClientState cs = clt->current_state = netbuf.getClientState();
	// If not a local player, add it in our array
	if( !isLocalSerial( cltserial))
	{
		// The save buffer and XML buffer come after the ClientState
		vector<string> saves;
		saves.push_back( netbuf.getString());
		saves.push_back( netbuf.getString());
		//saves = FileUtil::GetSaveFromBuffer( packet->getData()+sizeof( ClientState));
		cout<<"SAVE="<<saves[0].length()<<" bytes - XML="<<saves[1].length()<<" bytes"<<endl;

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
		Unit * un = UnitFactory::createUnit( callsign.c_str(),
							 false,
							 FactionUtil::GetFaction( PLAYER_FACTION_STRING.c_str()),
							 string(""),
							 Flightgroup::newFlightgroup ( callsign,savedships[0],PLAYER_FACTION_STRING,"default",1,1,"","",mission),
							 0, saves[1]);
		clt->game_unit.SetUnit( un);
		// Set all weapons to inactive
		vector <Mount>
			::iterator i = un->mounts.begin();//note to self: if vector<Mount *> is ever changed to vector<Mount> remove the const_ from the const_iterator
		for (;i!=un->mounts.end();++i)
			(*i).status=Mount::INACTIVE;
		//Clients[cltserial]->game_unit.GetUnit()->PrimeOrders();
		clt->game_unit.GetUnit()->SetNetworkMode( true);
		clt->game_unit.GetUnit()->SetSerial( cltserial);
		//cout<<"Addclient 4"<<endl;

		// Assign new coordinates to client
		clt->game_unit.GetUnit()->SetPosition( save.GetPlayerLocation());
		clt->current_state.setSerial( cltserial);
		clt->current_state.setPosition( save.GetPlayerLocation());
		// No need to set orientation and speed when client is created in the zone
		//clt->game_unit.GetUnit()->SetOrientation( cs.getOrientation());
		//clt->game_unit.GetUnit()->SetVelocity( cs.getVelocity());
		clt->game_unit.GetUnit()->SetNetworkMode( true);

		// In that case, we want cubic spline based interpolation
		//init_interpolation( cltserial);
	}
	// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
	else if( cltserial!=this->serial)
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

	cout<<" & HTONS(Serial) = "<<cltserial<<endl;
	if( Clients[cltserial] == NULL)
	{
		cout<<"Error, client does not exists !!"<<endl;
		exit( 1);
	}

	// Removes the unit from starsystem, destroys it and delete client
	_Universe->activeStarSystem()->RemoveUnit(Clients[cltserial]->game_unit.GetUnit());
	nbclients--;
	delete Clients[cltserial];
	cout<<"Leaving client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;

	Clients[cltserial] = NULL;
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
	cout<<"Sending ClientState == ";
	(*cs).display();
	netbuf.addClientState( (*cs));
	pckt.send( CMD_POSUPDATE, this->serial, netbuf.getData(), netbuf.getDataLength(), SENDANDFORGET, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			793
#endif
			);
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
	int		nbclts=0, i, j, offset=0;;
	// int		nbclts2=0;
	int		cssize = sizeof( ClientState);
	//int		smallsize = sizeof( ObjSerial) + sizeof( QVector);
	int		qfsize = sizeof( double);
	unsigned char	cmd;
	Client * clt;

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
			cout<<"Received FULLSTATE ";
			// Tell we received the ClientState so we can convert byte order from network to host
			//cs.display();
			sernum = cs.getSerial();
			clt = Clients[sernum];
			// Test if this is a local player
			// Is it is, ignore position update
			if( clt!=NULL && !_Universe->isPlayerStarship( Clients[sernum]->game_unit.GetUnit()))
			{
				// Backup old state
				clt->old_state = Clients[sernum]->current_state;
				// Update concerned client directly in network client list
				clt->current_state = cs;

				// Set the orientation by extracting the matrix from quaternion
				clt->game_unit.GetUnit()->SetOrientation( cs.getOrientation());
				clt->game_unit.GetUnit()->SetVelocity( cs.getVelocity());
				// Use SetCurPosition or SetPosAndCumPos ??
				clt->game_unit.GetUnit()->SetCurPosition( cs.getPosition());
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
			clt = Clients[sernum];
			cout<<"Received POSUPDATE for serial "<<sernum<<" -> ";
			//offset += sizeof( ObjSerial);
			if( clt!=NULL && !_Universe->isPlayerStarship( clt->game_unit.GetUnit()))
			{
				// Backup old state
				clt->old_state = clt->current_state;
				// Set the new received position in current_state
				//QVector tmppos( VSSwapHostDoubleToLittle( (double) *(databuf+offset)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize+qfsize)));
				QVector tmppos = netbuf.getVector();
				clt->current_state.setPosition( tmppos);
				// Use SetCurPosition or SetPosAndCumPos ??
				clt->game_unit.GetUnit()->SetCurPosition( tmppos);
				clt->current_state.display();
				//predict( sernum);
			}
			else
			{
				ClientState cs2;
				QVector tmppos = netbuf.getVector();
				cs2.setPosition( tmppos);
				cs2.display();
				cout<<"ME OR LOCAL PLAYER = IGNORING"<<endl;
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
	Packet packet2;
	NetBuffer netbuf;

	ClientState cs( this->serial, this->game_unit.GetUnit()->curr_physical_state, this->game_unit.GetUnit()->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !!
	netbuf.addClientState( cs);
	packet2.send( CMD_ADDCLIENT, this->serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			906
#endif
			);
	cout<<"Sending ingame with serial n°"<<this->serial<<endl;
}

/*************************************************************/
/**** Says we are still alive                             ****/
/*************************************************************/

void	NetClient::sendAlive()
{
    if( clt_sock.isTcp() == false )
    {
	Packet	p;
	p.send( CMD_PING, this->serial, NULL, 0, SENDANDFORGET, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			927
#endif
			);
    }
}

/*************************************************************/
/**** Receive the ship and char from server               ****/
/*************************************************************/

void	NetClient::receiveSave( const Packet* packet )
{
}

/*************************************************************/
/**** Receive that start locations                        ****/
/*************************************************************/

// Receives possible start locations (first a short representing number of locations)
// Then for each number, a desc

void	NetClient::receiveLocations( const Packet* )
{
	unsigned char	cmd;

#ifdef __DEBUG__
	cout<<"Nb start locations : "<<nblocs<<endl;
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
	p.send( CMD_LOGOUT, this->serial, NULL, 0, SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			987
#endif
			);
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

	unsigned int del = Clients[clientid]->current_state.getDelay();
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	//QVector A( Clients[clientid]->old_state.getPosition());
	QVector B( Clients[clientid]->current_state.getPosition());
	//Vector  VA( Clients[clientid]->old_state.getVelocity());
	Vector  VB( Clients[clientid]->current_state.getVelocity());
	//Vector  AA( Clients[clientid]->old_state.getAcceleration());
	Vector  AB( Clients[clientid]->current_state.getAcceleration());
	//QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	//QVector A2( A3 - (VB + AB*delay));

	Clients[clientid]->old_state = Clients[clientid]->current_state;
	Clients[clientid]->current_state.setPosition( A3);
}

void	NetClient::init_interpolation( ObjSerial clientid)
{
	// This function is to call after the state have been updated (which should be always the case)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	unsigned int del = Clients[clientid]->current_state.getDelay();
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	QVector A( Clients[clientid]->old_state.getPosition());
	QVector B( Clients[clientid]->current_state.getPosition());
	Vector  VA( Clients[clientid]->old_state.getVelocity());
	Vector  VB( Clients[clientid]->current_state.getVelocity());
	//Vector  AA( Clients[clientid]->old_state.getAcceleration());
	Vector  AB( Clients[clientid]->current_state.getAcceleration());
	QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	QVector A2( A3 - (VB + AB*delay));

	//Clients[clientid]->spline.createSpline( A, A1, A2, A3);
}

Transformation NetClient::spline_interpolate( ObjSerial clientid, double blend)
{
	// blend = time
	// Add a linear interpolation for orientation
	Quaternion orient;
	// There should be another function called when received a new position update and creating the spline
	QVector pos( Clients[clientid]->spline.computePoint( blend));

	return Transformation( orient, pos);
}

/******************************************************************************************/
/*** WEAPON STUFF                                                                      ****/
/******************************************************************************************/

// Send a info request about the target
void	NetClient::targetRequest( Unit * target)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( target->GetSerial());
	netbuf.addInt32( this->zone);

	p.send( CMD_TARGET, this->serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			1083
#endif
			);
}

// In fireRequest we must use the provided serial because it may not be the client's serial
// but may be a turret serial
void	NetClient::fireRequest( ObjSerial serial, int mount_index, char mis)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);
	netbuf.addInt32( this->zone);
	netbuf.addChar( mis);

	p.send( CMD_FIREREQUEST, this->serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			1097
#endif
			);
}

void	NetClient::unfireRequest( ObjSerial serial, int mount_index)
{
	Packet p;
	NetBuffer netbuf;

	netbuf.addSerial( serial);
	netbuf.addInt32( mount_index);
	netbuf.addInt32( this->zone);

	p.send( CMD_UNFIREREQUEST, this->serial, netbuf.getData(), netbuf.getDataLength(), SENDRELIABLE, NULL, this->clt_sock, __FILE__, 
#ifndef _WIN32
			__LINE__
#else
			1109
#endif
			);
}

