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

#include <iostream>
#include <stdio.h>
#include <unistd.h>
/*
#ifndef HAVE_SDL
	#include <sys/select.h>
	#include <sys/time.h>
#endif
*/
#include "vs_globals.h"
#include "endianness.h"
#include "../config_xml.h"
#include "client.h"
#include "const.h"
#include "packet.h"
#include "netclient.h"
#include "universe_util.h"
#include "cmd/unit_factory.h"
#include "gfx/matrix.h"
#include "lin_time.h"
#include "vs_path.h"
//#include "incnet.h"

using std::cout;
using std::endl;
using std::cin;

double NETWORK_ATOM;

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

/*************************************************************/
/**** Authenticate the client                             ****/
/*************************************************************/

int		NetClient::authenticate()
{
	Packet	packet2;
	int tmplen = NAMELEN*2;
	char *	buffer = new char[tmplen+1];
	char	name[NAMELEN], passwd[NAMELEN];
	string  str_name, str_passwd;

	// Get the name and password from vegastrike.config
	// Maybe someday use a default Guest account if no callsign or password is provided thus allowing
	// Player to wander but not interact with the universe
	str_name = vs_config->getVariable ("player","callsign","");
	str_passwd = vs_config->getVariable ("player","password","");
	memcpy( name, str_name.c_str(), str_name.length());
	memcpy( passwd, str_passwd.c_str(), str_passwd.length());
	if( str_name.length() && str_passwd.length())
	{
		memcpy( buffer, name, str_name.length());
		memcpy( buffer+NAMELEN, passwd, str_passwd.length());
		buffer[tmplen] = '\0';

		packet2.create( CMD_LOGIN, 0, buffer, tmplen, SENDRELIABLE, &this->cltadr, this->clt_sock);
		sendQueue.add( packet2);
		/*
		if( NetInt->sendbuf( this->clt_sock, (char *) &packet2, packet2.getSendLength(), &this->cltadr) == -1)
		{
			perror( "Error send login ");
			cleanup();
		}
		*/
		delete buffer;
		cout<<"Send login for player <"<<str_name<<">:<"<<str_passwd<<"> - buffer length : "<<packet2.getLength()<<endl;
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

char *	NetClient::loginLoop( string str_name, string str_passwd)
{
	Packet	packet2;
	int tmplen = NAMELEN*2;
	char *	buffer = new char[tmplen+1];
	// HAVE TO DELETE netbuf after return in calling function
	char *	netbuf = new char[MAXBUFFER];
	char	name[NAMELEN], passwd[NAMELEN];

	memset( buffer, 0, tmplen+1);
	memset( name, 0, NAMELEN);
	memset( passwd, 0, NAMELEN);
	memcpy( buffer, str_name.c_str(), str_name.length());
	memcpy( buffer+NAMELEN, str_passwd.c_str(), str_passwd.length());

	packet2.create( CMD_LOGIN, 0, buffer, tmplen, SENDRELIABLE, &this->cltadr, this->clt_sock);
	cout<<"Send login for player <"<<str_name<<">:<"<<str_passwd<<"> - buffer length : "<<packet2.getLength()<<endl;
	sendQueue.add( packet2);
	sendQueue.send( this->NetInt);
	/*
	if( NetInt->sendbuf( this->clt_sock, (char *) &packet2, packet2.getSendLength(), &this->cltadr) == -1)
	{
		perror( "Error send login ");
		cleanup();
	}
	*/
	delete buffer;
	// Now the loop
	int timeout=0, recv=0, ret=0;
	UpdateTime();
	while( !timeout && !recv)
	{
		// If we have no response in 10 seconds -> fails
		if( GetElapsedTime() > 10)
		{
			cout<<"Timed out"<<endl;
			timeout = 1;
		}
		ret=this->checkMsg(netbuf);
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
	if( ret>0 && packet.getCommand()!=LOGIN_ERROR)
	{
		this->callsign = str_name;
		// Get the save parts in the buffer
		const char * xml = netbuf + NAMELEN*2 + sizeof( unsigned int);
		unsigned int xml_size = ntohl( *( (unsigned int *)(netbuf+NAMELEN*2)));
		const char * save = netbuf + NAMELEN*2 + sizeof( unsigned int)*2 + xml_size;
		unsigned int save_size = ntohl( *( (unsigned int *)(netbuf+ NAMELEN*2 + sizeof( unsigned int) + xml_size)));
		cout<<"XML size = "<<xml_size<<endl;
		cout<<"Save size = "<<save_size<<endl;
		// Write temp save files
		//changehome();
		WriteXMLUnit( homedir+"/save/"+str_name+".xml", netbuf+2*NAMELEN+sizeof( unsigned int), xml_size);
		WriteXMLUnit( homedir+"/save/"+str_name+".save", netbuf+2*NAMELEN+sizeof( unsigned int)*2+xml_size, save_size);
	}
	else
	{
		delete netbuf;
		netbuf=NULL;
	}
	return netbuf;
}

/*************************************************************/
/**** Initialize the client network                       ****/
/*************************************************************/

int		NetClient::init( char * addr, unsigned short port)
{
	string strnetatom;
	strnetatom = vs_config->getVariable( "network", "network_atom", "");
	if( strnetatom=="")
		NETWORK_ATOM = 0.2;
	else
		NETWORK_ATOM = (double) atoi( strnetatom.c_str());

	NetInt = new NetUI();
	this->clt_sock = NetInt->createSocket( addr, port, 0);

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

void	NetClient::start( char * addr, unsigned short port)
{
	keeprun = 1;

	cout<<"Loading data files..."<<endl;
	this->readDatafiles();

	cout<<"Initializing network connection..."<<endl;
	this->clt_sock = NetInt->createSocket( addr, port, 0);

	if( this->authenticate() == -1)
	{
		perror( "Error login in ");
		cleanup();
	}

	cout<<"Initiating client loop"<<endl;
	while( keeprun)
	{
		this->checkKey();
		this->checkMsg();
		micro_sleep( 30000);
	}

	this->disconnect();
}

void	NetClient::checkKey()
{
	/*
	Packet	packet2;
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
				keeprun = 0;
			else if( serial!=0)
			{
				packet2.create( CMD_POSUPDATE, this->serial, &c, sizeof(char));
				packet2.tosend();
				NetInt->sendbuf( this->clt_sock, (char *) &packet2, packet2.getSendLength(), &this->cltadr);
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

void	NetClient::sendMsg()
{
	sendQueue.send( this->NetInt);
}

/**************************************************************/
/**** Check if server has sent something                   ****/
/**************************************************************/

int		NetClient::checkMsg( char * netbuffer)
{
	int nb=0;
	int ret=0;

	NetInt->resetSets();
	NetInt->watchSocket( NetInt->sock);
	nb = NetInt->activeSockets();
	if( nb)
	{
		if( NetInt->isActive( NetInt->sock))
		{
			//cout<<"Network Activity !"<<endl;
			ret = recvMsg( netbuffer);
		}
	}
	//cout<<"CHECK MSG - Length received : "<<ret<<endl;
	return ret;
}

/**************************************************************/
/**** Receive a message from the server                    ****/
/**************************************************************/

int		NetClient::recvMsg( char * netbuffer)
{
	unsigned int len2;
	ObjSerial	packet_serial=0;
	int len=0;
	int nbpackets=0;

	// Receive data
	nbpackets = recvQueue.receive( this->NetInt, this->clt_sock, this->cltadr, this->serial);

	/*
	if( (len=NetInt->recvbuf( this->clt_sock, (char *)&packet, len2, &this->cltadr))<=0)
	*/
	if( nbpackets <= 0)
	{
		perror( "Error recv -1 ");
		NetInt->closeSocket( this->clt_sock);
	}
	else
	{
	while( !recvQueue.empty())
	{
		packet = recvQueue.getNextPacket();
		//packet.received();
		packet_serial = packet.getSerial();
#ifdef _UDP_PROTO
		// Test if we didn't receive an old packet
		if( packet.getTimestamp() >= current_timestamp)
		{
#endif
			old_timestamp = current_timestamp;
			current_timestamp = packet.getTimestamp();
			deltatime = current_timestamp - old_timestamp;
			//packet.displayHex();
			//cout<<"Received command : "<<(char) packet.getCommand()<<endl;
			packet.displayCmd( packet.getCommand());
			switch( packet.getCommand())
			{
				// Login accept
				case LOGIN_ACCEPT :
					cout<<">>> "<<this->serial<<" >>> LOGIN ACCEPTED =( serial n°"<<packet_serial<<" )= --------------------------------------"<<endl;
					// Should receive player's data (savegame) from server if there is a save
					this->serial = packet.getSerial();
					localSerials.push_back( this->serial);
					//cout<<"Received LOGIN_ACCEPT with serial : "<<this->serial<<endl;
					memcpy( netbuffer, packet.getData(), packet.getLength());
					//this->receiveSave( netbuffer);
					// Set current timestamp
					this->current_timestamp = packet.getTimestamp();
					cout<<"<<< LOGIN ACCEPTED ------------------------------------------------------------------------------"<<endl;
				break;
				// Login failed
				case LOGIN_ERROR :
					cout<<">>> LOGIN ERROR =( DENIED )= ------------------------------------------------"<<endl;
					//cout<<"Received LOGIN_ERROR"<<endl;
					this->disconnect();
					return -1;
					cout<<"<<< LOGIN ERROR ---------------------------------------------------"<<endl;
				break;
				// Create a character
				case CMD_CREATECHAR :
					// Should begin character/ship creation process
					//this->createChar();
				break;
				// Receive start locations
				case CMD_LOCATIONS :
					// Should receive possible starting locations list
					this->receiveLocations();
				break;
				case CMD_SNAPSHOT :
					// Should update another client's position
					//cout<<"Received a SNAPSHOT from server"<<endl;
					this->receivePosition();
				break;
				case CMD_ENTERCLIENT :
					cout<<">>> "<<this->serial<<" >>> ENTERING CLIENT =( serial n°"<<packet_serial<<" )= --------------------------------------"<<endl;
					this->addClient();
					cout<<"<<< ENTERING CLIENT ------------------------------------------------------------------------------"<<endl;
				break;
				case CMD_EXITCLIENT :
					cout<<">>> "<<this->serial<<" >>> EXITING CLIENT =( serial n°"<<packet_serial<<" )= --------------------------------------"<<endl;
					this->removeClient();
					cout<<"<<< EXITING CLIENT ------------------------------------------------------------------------------"<<endl;
				break;
				case CMD_ADDEDYOU :
					cout<<">>> "<<this->serial<<" >>> ADDED IN GAME =( serial n°"<<packet_serial<<" )= --------------------------------------"<<endl;
					this->getZoneData();
					cout<<"<<< ADDED IN GAME ------------------------------------------------------------------------------"<<endl;
				break;
				case CMD_DISCONNECT :
					/*** TO REDO IN A CLEAN WAY ***/
					cout<<">>> "<<this->serial<<" >>> DISCONNECTED -> Client killed =( serial n°"<<packet_serial<<" )= --------------------------------------"<<endl;
					exit(1);
				break;
				case CMD_ACK :
					/*** RECEIVED AN ACK FOR A PACKET : comparison on packet timestamp and the client serial in it ***/
					/*** We must make sure those 2 conditions are enough ***/
					cout<<">>> ACK =( "<<packet.getTimestamp()<<" )= ---------------------------------------------------"<<endl;
					sendQueue.ack( packet);
				break;
				default :
					cout<<">>> "<<this->serial<<" >>> UNKNOWN COMMAND =( "<<hex<<packet.getCommand()<<" )= --------------------------------------"<<endl;
					keeprun = 0;
					this->disconnect();
			}
#ifdef _UDP_PROTO
		}
		else
		{
			cout<<"Received an old packet\n";
		}
#endif
	}
	}
	return nbpackets;
}

/*************************************************************/
/**** Get the zone data from server                       ****/
/*************************************************************/

void	NetClient::getZoneData()
{
	unsigned short nbclts;
	ClientState cs;
	ClientDescription cd;
	int		state_size=sizeof( ClientState);
	int		desc_size=sizeof( ClientDescription);
	ObjSerial nser, nser2 = 0;
	int		offset=0;


	offset = sizeof( unsigned short);
	char * buf = packet.getData();
	nbclts = *((unsigned short *) buf);
	nbclts = ntohs( nbclts);
	for( int i=0; i<nbclts; i++)
	{
		memcpy( &cs, buf+offset, state_size);
		offset += state_size;
		memcpy( &cd, buf+offset, desc_size);
		offset += desc_size;
		//cs = (ClientState *) packet.getData()+offset;
		cs.received();
		nser2 = cs.getSerial();
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
			Clients[nser]->game_unit = UniverseUtil::launch (string(""),"avenger",string(""),string( "unit"), string("default"),1,0, cs.getPosition(), string(""));
			Clients[nser]->game_unit->PrimeOrders();
			Clients[nser]->game_unit->SetNetworkMode( true);

			// Assign new coordinates to client
			Clients[nser]->game_unit->SetOrientation( cs.getOrientation());
			Clients[nser]->game_unit->SetVelocity( cs.getVelocity());
			Clients[nser]->game_unit->SetNetworkMode( true);
			// In that case, we want cubic spline based interpolation
			//init_interpolation( nser);
		}
		// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
		else if( nser!=this->serial)
		{
			Clients[nser] = new Client;
			cout<<"IT'S ME OR ANOTHER LOCAL PLAYER ";
			Clients[nser]->game_unit = getNetworkUnit( nser);
			assert( Clients[nser]->game_unit != NULL);
			cs.display();
		}
	}
}

/*************************************************************/
/**** Adds an entering client in the actual zone          ****/
/*************************************************************/

void	NetClient::addClient()
{
	ObjSerial	cltserial = packet.getSerial();
	if( Clients[cltserial] != NULL)
	{
		cout<<"Error, client exists !!"<<endl;
		exit( 1);
	}

	Clients[cltserial] = new Client;
	nbclients++;
	cout<<"New client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;
	cout<<"At : ";

	// Assign the data

	// Copy the client state in its structure
	Clients[cltserial]->serial = packet.getSerial();
	ClientState cs;
	memcpy( &cs, packet.getData(), sizeof( ClientState));
	//memcpy( &Clients[cltserial]->current_state, &cs, sizeof( ClientState));
	Clients[cltserial]->current_state = cs;

	cs.display();
	// Test if not a local player
	if( !isLocalSerial( cltserial))
	{
		// CREATES THE UNIT... GET XML DATA OF UNIT FROM SERVER
		Clients[cltserial]->game_unit = UniverseUtil::launch (string(""),"avenger",string(""),string( "unit"), string("default"),1,0, cs.getPosition(), string(""));
		Clients[cltserial]->game_unit->PrimeOrders();
		Clients[cltserial]->game_unit->SetNetworkMode( true);
		//cout<<"Addclient 4"<<endl;

		// Assign new coordinates to client
		Clients[cltserial]->game_unit->SetOrientation( cs.getOrientation());
		Clients[cltserial]->game_unit->SetVelocity( cs.getVelocity());
		Clients[cltserial]->game_unit->SetNetworkMode( true);
		// In that case, we want cubic spline based interpolation
		//init_interpolation( cltserial);
	}
	// If this is a local player (but not the current), we must affect its Unit to Client[sernum]
	else if( cltserial!=this->serial)
	{
		Clients[cltserial]->game_unit = getNetworkUnit( cltserial);
		assert( Clients[cltserial]->game_unit != NULL);
	}
}

/*************************************************************/
/**** Removes an exiting client of the actual zone        ****/
/*************************************************************/

void	NetClient::removeClient()
{
	ObjSerial	cltserial = packet.getSerial();
	//ObjSerial	cltserial = ntohs( cltserial2);

	//cout<<"Serial = "<<cltserial2;
	cout<<" & HTONS(Serial) = "<<cltserial<<endl;
	if( Clients[cltserial] == NULL)
	{
		cout<<"Error, client does not exists !!"<<endl;
		exit( 1);
	}

	// Removes the unit from starsystem, destroys it and delete client
	_Universe->activeStarSystem()->RemoveUnit(Clients[cltserial]->game_unit);
	nbclients--;
	delete Clients[cltserial];
	cout<<"Leaving client n°"<<cltserial<<" - now "<<nbclients<<" clients in system"<<endl;

	Clients[cltserial] = NULL;
}

/*************************************************************/
/**** Send an update to server                            ****/
/*************************************************************/

void	NetClient::sendPosition( ClientState cs)
{
	// Serial in ClientState is updated in UpdatePhysics code at ClientState creation (with pos, veloc...)
	Packet pckt;
	ClientState cstmp(cs);
	int		update_size = sizeof( ClientState);
	//char * buffer = new char[update_size];

	// Send the client state
	cout<<"Sending position == ";
	cstmp.display();
	cstmp.tosend();
	pckt.create( CMD_POSUPDATE, this->serial, (char *) &cstmp, update_size, SENDANDFORGET, &this->cltadr, this->clt_sock);
	sendQueue.add( pckt);
	/*
	if( NetInt->sendbuf( this->clt_sock, (char *) &pckt, pckt.getSendLength(), &this->cltadr) == -1)
	{
		perror( "Error send position ");
		cleanup();
	}
	*/
	//cs.received();
	//cout<<"Sent STATE : ";
	//cs.display();
	//delete buffer;
}

/**************************************************************/
/**** Update another client position                       ****/
/**************************************************************/

void	NetClient::receivePosition()
{
	// When receiving a snapshot, packet serial is considered as the number of client updates
	ClientState cs;
	//QVector		tmppos;
	char *		databuf;
	ObjSerial	sernum=0;
	int		nbclts=0, nbclts2=0, i, j, offset=0;;
	int		cssize = sizeof( ClientState);
	//int		smallsize = sizeof( ObjSerial) + sizeof( QVector);
	int		qfsize = sizeof( double);
	unsigned char	cmd;

	nbclts = packet.getSerial();
	//nbclts = ntohs( nbclts2);
	cout<<"Received update for "<<nbclts<<" clients - LENGTH="<<packet.getLength();
	cout<<endl;
	databuf = packet.getData();
	for( i=0, j=0; (i+j)<nbclts;)
	{
		cmd = *(databuf+offset);
		offset += sizeof( unsigned char);
		//packet.displayCmd( cmd);
		if( cmd == CMD_FULLUPDATE)
		{
			memcpy( &cs, (databuf+offset), cssize);
			cs.received();
			// Do what needed with update
			cout<<"Received FULLSTATE ";
			// Tell we received the ClientState so we can convert byte order from network to host
			//cs.display();
			sernum = cs.getSerial();
			// Test if this is a local player
			// Is it is, ignore position update
			if( Clients[sernum]!=NULL && !_Universe->isPlayerStarship( Clients[sernum]->game_unit))
			{
				// Backup old state
				Clients[sernum]->old_state = Clients[sernum]->current_state;
				//memcpy( &(Clients[sernum]->old_state), &(Clients[sernum]->current_state), sizeof( ClientState));
				// Update concerned client directly in network client list
				Clients[sernum]->current_state = cs;
				// memcpy( &(Clients[sernum]->current_state), &cs, sizeof( ClientState));

				// Set the orientation by extracting the matrix from quaternion
				Clients[sernum]->game_unit->SetOrientation( cs.getOrientation());
				Clients[sernum]->game_unit->SetVelocity( cs.getVelocity());
				// Use SetCurPosition or SetPosAndCumPos ??
				Clients[sernum]->game_unit->SetCurPosition( cs.getPosition());
				// In that case, we want cubic spline based interpolation
				//predict( sernum);
				//init_interpolation( sernum);
			}
			offset += cssize;
			i++;
		}
		else if( cmd == CMD_POSUPDATE)
		{
			// Set the serial #
			sernum = *((ObjSerial *) databuf+offset);
			sernum = ntohs( sernum);
			cout<<"Received POSUPDATE for serial "<<sernum<<" -> ";
			offset += sizeof( ObjSerial);
			if( Clients[sernum]!=NULL && !_Universe->isPlayerStarship( Clients[sernum]->game_unit))
			{
				// Backup old state
				//memcpy( &(Clients[sernum]->old_state), &(Clients[sernum]->current_state), sizeof( ClientState));
				Clients[sernum]->old_state = Clients[sernum]->current_state;
				// Set the new received position in current_state
				QVector tmppos( VSSwapHostDoubleToLittle( (double) *(databuf+offset)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize+qfsize)));
				//tmppos = (QVector) *(databuf+offset);
				Clients[sernum]->current_state.setPosition( tmppos);
				// Use SetCurPosition or SetPosAndCumPos ??
				Clients[sernum]->game_unit->SetCurPosition( tmppos);
				Clients[sernum]->current_state.display();
				//predict( sernum);
			}
			else
			{
				ClientState cs2;
				QVector tmppos( VSSwapHostDoubleToLittle( (double) *(databuf+offset)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize)), VSSwapHostDoubleToLittle( (double) *(databuf+offset+qfsize+qfsize)));
				cs2.setPosition( tmppos);
				cs2.display();
				cout<<"ME OR LOCAL PLAYER = IGNORING"<<endl;
			}
			offset += sizeof( QVector);
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

	ClientState cs( this->serial, this->game_unit->curr_physical_state, this->game_unit->Velocity, Vector(0,0,0), 0);
	// HERE SEND INITIAL CLIENTSTATE !!
	packet2.create( CMD_ADDCLIENT, this->serial, (char *)&cs, sizeof( ClientState), SENDRELIABLE, &this->cltadr, this->clt_sock);
	sendQueue.add( packet2);
	/*
	if( NetInt->sendbuf( this->clt_sock, (char *) &packet2, packet2.getSendLength(), &this->cltadr) == -1)
	{
		perror( "Error sending ingame info");
		exit(1);
	}
	*/
	cout<<"Sending ingame with serial n°"<<this->serial<<endl;
}

/*************************************************************/
/**** Says we are still alive                             ****/
/*************************************************************/

void	NetClient::sendAlive()
{
#ifdef _UDP_PROTO
	Packet	p;
	p.create( CMD_PING, this->serial, NULL, 0, SENDANDFORGET, &this->cltadr, this->clt_sock);
	sendQueue.add( p);
	/*
	if( NetInt->sendbuf( this->clt_sock, (char *) &p, p.getSendLength(), &this->cltadr) == -1)
	{
		perror( "Error send PING ");
		//exit(1);
	}
	*/
#endif
}

/*************************************************************/
/**** Receive the ship and char from server               ****/
/*************************************************************/

void	NetClient::receiveSave()
{
		char * xml = packet.getData() + NAMELEN*2 + sizeof( int);
		int xml_size = ntohl(*(packet.getData()+ NAMELEN*2));

		// HERE SHOULD LOAD Savegame desciription from the save in the packet
		char * save = packet.getData() + NAMELEN*2 + sizeof( int)*2 + xml_size;
		int save_size = *(packet.getData()+ NAMELEN*2 + sizeof( int) + xml_size);
		cout<<"RECV SAVES : XML="<<xml_size<<" bytes - SAVE="<<save_size<<" bytes"<<endl;
		cout<<"Welcome back in VegaStrike, "<<packet.getData()<<endl;
}

/*************************************************************/
/**** Receive that start locations                        ****/
/*************************************************************/

// Receives possible start locations (first a short representing number of locations)
// Then for each number, a desc

void	NetClient::receiveLocations()
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
	//NetInt->disconnect( "Closing network");
}

void	NetClient::logout()
{
	keeprun = 0;
	Packet p;
	p.create( CMD_LOGOUT, this->serial, NULL, 0, SENDRELIABLE, &this->cltadr, this->clt_sock);
	sendQueue.add( p);
	/*
	if( NetInt->sendbuf( this->clt_sock, (char *) &p, p.getSendLength(), &this->cltadr) == -1)
	{
		perror( "Error send logout ");
		//exit(1);
	}
	*/
	NetInt->disconnect( "Closing network", 0);
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

	memcpy( &Clients[clientid]->old_state, &Clients[clientid]->current_state, sizeof( ClientState));
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
