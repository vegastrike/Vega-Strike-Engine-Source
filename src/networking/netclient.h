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

#ifndef __NetClient_H
#define __NetClient_H

#include <config.h> // for NETCOMM

#include <string>
#include <vector>

#include "savegame.h"
#include "networking/const.h"
#include "networking/vsnet_socket.h"
#include "networking/vsnet_socketset.h"
#include "networking/vsnet_cmd.h"
#include "cmd/container.h"   // for UnitContainer
#include "gfx/quaternion.h"  // for Transformation

#include "networking/networkcomm.h"

class Packet;
class Unit;
class Client;
class ClientState;
class NetUI;

using std::vector;
using std::string;
extern vector<ObjSerial>	localSerials;
extern bool isLocalSerial( ObjSerial sernum);

class	NetClient
{
		UnitContainer		game_unit;		// Unit struct from the game corresponding to that client

		SOCKETALT			clt_sock;		// Comm. socket
		SOCKETALT			acct_sock;		// Connection socket for account server
        SocketSet           _sock_set;      // Encapsulates select()
		SaveGame			save;
		ObjSerial			serial;			// Serial # of client
		int					nbclients;		// Number of clients in the zone
		int					zone;			// Zone id in universe
		char				keeprun;		// Bool to test client stop
		string				callsign;		// Callsign of the networked player
		Client *			Clients[MAXCLIENTS];		// Clients in the same zone
		Unit *				Units[MAXOBJECTS];			// Server controlled units in the same zone
		// a vector because always accessed by their IDs

#ifdef NETCOMM
		NetworkCommunication *	NetComm;
		bool					netcomm_active;
	public:
		bool					IsNetcommActive();
	private:
#endif

		int					enabled;		// Bool to say network is enabled
		// Time used for refresh - not sure still used
		int					old_time;
		double				cur_time;
		// Timestamps from packets
		unsigned int		old_timestamp;
		unsigned int		current_timestamp;
		unsigned int		deltatime;
		bool				jumpok;
		bool				ingame;
		float				current_freq;
		float				selected_freq;

		void	receiveData();
		void	readDatafiles();
		void	createChar();
		int		recvMsg( Packet* outpacket );
		void	disconnect();
		int		checkAcctMsg( );

		void	receiveSave( const Packet* packet );
		void	receiveLocations( const Packet* packet );
		void	getZoneData( const Packet* packet );
		void	receivePosition( const Packet* packet );
		void	addClient( const Packet* packet );
		void	removeClient( const Packet* packet );

	public:
		NetClient();
		~NetClient();

		int		authenticate();
		bool	PacketLoop( Cmd command );
		vector<string>	loginLoop( string str_callsign, string str_passwd); // Loops until receiving login response
		vector<string>	loginAcctLoop( string str_callsign, string str_passwd);
		SOCKETALT	init( char * addr, unsigned short port);
		SOCKETALT	init_acct( char * addr, unsigned short port);
		void	start( char * addr, unsigned short port);
		void	checkKey();

		void	setCallsign( char * calls) { this->callsign = string( calls);}
		void	setCallsign( string calls) { this->callsign = calls;}
		string	getCallsign() {return this->callsign;}
		void	setUnit( Unit * un) { game_unit.SetUnit( un);}
		Unit *	getUnit() { return game_unit.GetUnit();}

		/********************* Network stuff **********************/
		// Get the lag time between us and the server
		unsigned int	getLag() { return deltatime;}
		// Check if it is time to send our update
		int		isTime();
		// Warn the server we are leaving the game
		void	logout();
		// Check if there are info incoming over the network
		int		checkMsg( Packet* outpacket );
		// Send a position update
		void	sendPosition( const ClientState* cs );
		// Send a PING-like packet to say we are still alive (UDP)
		void	sendAlive();
		void	inGame();		// Tells the server we are ready to go in game
		bool	isInGame() { return this->ingame;}

		// void	disable() { enabled=false;}
		// int		isEnabled() { return enabled; }
		// void	setNetworkedMode( bool mode) { enabled = mode;}

		/********************* Prediction stuff **********************/
		void			predict( ObjSerial clientid);
		void			init_interpolation( ObjSerial clientid);
		Transformation	spline_interpolate( ObjSerial clientid, double blend);

		/********************* Weapon stuff **********************/
		// Functions called when we receive a firing order from the server (other clients or ai or us)
		void	scanRequest( Unit * target);
		void	fireRequest( ObjSerial serial, int mount_index, char mis);
		void	unfireRequest( ObjSerial serial, int mount_index);

		bool	jumpRequest( string newsystem);

		/********************* Communication stuff **********************/
	private:
	public:
		void	startCommunication();
		void	stopCommunication();
		void	sendWebcamPicture();
		void	increaseFrequency();
		void	decreaseFrequency();
		float	getSelectedFrequency();
		float	getCurrentFrequency();
};

#endif

