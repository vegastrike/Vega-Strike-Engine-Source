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

#include <config.h> // for NETCOMM & ZLIB

#include <string>
#include <vector>

#include "boost/shared_ptr.hpp"
#include "savegame.h"
#include "networking/const.h"
#include "networking/vsnet_socket.h"
#include "networking/vsnet_socketset.h"
#include "networking/vsnet_cmd.h"
#include "cmd/container.h"   // for UnitContainer
#include "gfx/quaternion.h"  // for Transformation

#include "networking/clientptr.h"

class Packet;
class Unit;
class Client;
class ClientState;
class NetUI;
class NetworkCommunication;

namespace VsnetDownload {
  namespace Client {
    class Manager;
  };
};

using std::vector;
using std::string;
extern vector<ObjSerial>	localSerials;
extern bool isLocalSerial( ObjSerial sernum);

class	NetClient
{
    class Clients
    {
        ClientMap _map;

    public:
        ClientPtr insert( int x, Client* c );
        ClientPtr get( int x );
        bool      remove( int x );
    };

        UnitContainer		game_unit;		// Unit struct from the game corresponding to that client

        string              _serverip;      // used during login
        string              _serverport;    // used during login
        SOCKETALT			clt_sock;		// Comm. socket
        SOCKETALT			acct_sock;		// Connection socket for account server
        SocketSet           _sock_set;      // Encapsulates select()
        SaveGame			save;
        ObjSerial			serial;			// Serial # of client
        int					nbclients;		// Number of clients in the zone
        int					zone;			// Zone id in universe
        char				keeprun;		// Bool to test client stop
        string				callsign;		// Callsign of the networked player
        Clients 			Clients;		// Clients in the same zone
		// This unit array has to be changed into a map too !!
        // Unit *				Units[MAXOBJECTS];			// Server controlled units in the same zone
	    // a vector because always accessed by their IDs

	    NetworkCommunication*	NetComm;

	public:
		bool IsNetcommActive() const;

	private:

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

        boost::shared_ptr<VsnetDownload::Client::Manager> _downloadManagerClient;
        static const char*                                _downloadSearchPaths[];

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
		SOCKETALT	init( const char* addr, unsigned short port);
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

		void	sendTextMessage( string message);

    private:
        bool canCompress() const;
};

#endif

