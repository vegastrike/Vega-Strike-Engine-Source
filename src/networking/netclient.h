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

#include <string>
#include <vector>

#include "const.h"
#include "vsnet_socket.h"
#include "gfx/quaternion.h"  // for Transformation
#include "cmd/container.h"
#include "cmd/weapon_xml.h"
#include "savegame.h"

class Packet;
class Unit;
class Client;
class ClientState;
class NetUI;

using std::vector;
using std::string;
extern vector<ObjSerial>	localSerials;
extern bool isLocalSerial( ObjSerial sernum);

typedef vector<Client *>::iterator VI;

class	NetClient
{
		UnitContainer		game_unit;		// Unit struct from the game corresponding to that client

		SOCKETALT			clt_sock;		// Comm. socket
		SaveGame			save;
		ObjSerial			serial;			// Serial # of client
		int					nbclients;		// Number of clients in the zone
		char				keeprun;		// Bool to test client stop
		string				callsign;		// Callsign of the networked player
		vector<Client *>	Clients;		// Clients or other Units in the same zone
		// a vector because always accessed by their IDs

		int					enabled;		// Bool to say network is enabled
		// Time used for refresh - not sure still used
		int					old_time;
		double				cur_time;
		// Timestamps from packets
		unsigned int		old_timestamp;
		unsigned int		current_timestamp;
		unsigned int		deltatime;

		void	receiveData();
		void	readDatafiles();
		void	createChar();
		int		recvMsg( char* netbuffer, Packet* outpacket );
		void	disconnect();

	public:
		NetClient():save("")
		{
			game_unit = NULL;
			old_timestamp = 0;
			current_timestamp = 0;
			old_time = 0;
			cur_time = 0;
			enabled = 0;
			nbclients = 0;
			serial = 0;
			for( int i=0; i<MAXCLIENTS; i++)
				Clients[i] = NULL;
		}
		~NetClient();

		int		authenticate();
		vector<string>	loginLoop( string str_name, string str_passwd); // Loops until receiving login response
		SOCKETALT	init( char * addr, unsigned short port);
		void	start( char * addr, unsigned short port);
		void	checkKey();


		ObjSerial	getSerial() { return serial; }
		void	inGame();
		// Tell if we are currently in the game
		bool	isInGame() { return (serial!=0);}

		/********************* Network stuff **********************/
		// Get the lag time between us and the server
		unsigned int	getLag() { return deltatime;}
		// Check if it is time to send our update
		int		isTime();
		// Warn the server we are leaving the game
		void	logout();
		// Check if there are info incoming over the network
		int		checkMsg( char* netbuffer, Packet* packet );
		// Send a position update
		void	sendPosition( const ClientState* cs );
		// Send a PING-like packet to say we are still alive (UDP)
		void	sendAlive();

		// void	disable() { enabled=false;}
		// int		isEnabled() { return enabled; }
		// void	setNetworkedMode( bool mode) { enabled = mode;}

		/********************* Prediction stuff **********************/
		void			predict( ObjSerial clientid);
		void			init_interpolation( ObjSerial clientid);
		Transformation	spline_interpolate( ObjSerial clientid, double blend);

		void	setCallsign( char * calls) { this->callsign = string( calls);}
		void	setCallsign( string calls) { this->callsign = calls;}
		string	getCallsign() {return this->callsign;}
		void	setUnit( Unit * un) { game_unit.SetUnit( un);}
		Unit *	getUnit() { return game_unit.GetUnit();}

		/********************* Weapon stuff **********************/
		void	FireBeam();
		void	FireBolt( weapon_info wi, Matrix mat, Vector velocity);
		void	FireProjectile( weapon_info wi, Unit * target, Matrix mat, Vector velocity, Transformation t);

    private:
		void	receiveSave( const Packet* packet );
		void	receiveLocations( const Packet* packet );
		void	getZoneData( const Packet* packet );
		void	receivePosition( const Packet* packet );
		void	addClient( const Packet* packet );
		void	removeClient( const Packet* packet );
};

#endif

