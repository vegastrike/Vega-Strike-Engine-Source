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
  NetServer - Network Server Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __NETSERVER_H
#define __NETSERVER_H

#include <vector>
#include <list>
#include <queue>
#include <string.h>
#include <stdlib.h>

#include "configxml.h"
#include "accountsxml.h"
#include "const.h"
#include "netui.h"
#include "zonemgr.h"
#include "client.h"
#include "savegame.h"
struct GFXColor;

extern VegaConfig *vs_config;
//extern VegaSimpleConfig *server_config;
using std::vector;
typedef list<Client *>::iterator LI;
typedef vector<Account *>::iterator VI;

struct ServerSocket;

/** Class Netserver : runs the "game server" */
class NetServer
{
        ServerSocket*   tcpNetwork;
        ServerSocket*   udpNetwork;
		Packet			packet;					// Network data packet
		Packet			packeta;				// Network data packet for account server

		SaveGame*		globalsave;				// Savegame class used to save dynamic universe
		ZoneMgr			*zonemgr;				// Zones/star systems Manager
		int				nbclients;				// Active client connections number
		int				nbaccts;				// Number of registered accounts

		SOCKETALT		acct_sock;				// Connection socket for account server
		int				keeprun;				// Tells if we should keep running server
		int				snapchanged;			// Tells the snapshot has changed and can be sent
		int				acctserver;				// Tells if we use an account server

		timeval				srvtimeout;			// timer

		vector<Account *>	Cltacct;			// Client accounts
        list<Client *>      tcpClients;         // Active TCP client connections
        list<Client *>      udpClients;         // Active UDP client connections
		list<Client *>		discList;			// Client connections to be disconnected
		list<Client *>		logoutList;			// Client connections that logged out

	    struct WaitListEntry
	    {
	        bool tcp;
	        Client*   t; // Client connections waiting for login response
		    AddressIP u; // Client addresses waiting for login response
	    };

        queue<WaitListEntry> waitList;

		//void			loadConfig();					// Loads configuration from server.xml
		void			authenticate( Client * clt, AddressIP sernum, Packet& packet );	// Authenticate a connected client
		void			posUpdate( Client * clt);		// Update a client position
		void			addClient( Client * clt);		// Add the client in the game
		Client *		newConnection_udp( const AddressIP& ipadr);
		Client *		newConnection_tcp( SocketSet& set );
		void			prepareCheckAcctMsg( SocketSet& set );	// Check for account server message to receive
		void			checkAcctMsg( SocketSet& set );			// Check for account server message to receive
		void			prepareCheckMsg( SocketSet& set );		// Check for network message to receive
		void			checkMsg( SocketSet& set );				// Check for network message to receive
		void			checkKey();						// Check for keyboard input
		void			recvMsg_tcp( Client * clt);		// Receive network messages
		void			recvMsg_udp( );					// Receive network messages
		void			processPacket( Client * clt, unsigned char cmd, const AddressIP& ipadr, Packet& packet ); // Process received packet info
		void			recvNewChar( Client * clt);		// Receive a new character
		void			sendLocations( Client * clt);	// Send the start locations
		void			startMsg();						// Startup showing text
		void			disconnect( Client * clt, const char* debug_from_file, int debug_from_line );		// Disconnect a client
		void			logout( Client * clt);			// Clean disconnect a client
		void			closeAllSockets();				// Disconnect all clients for shutdown
        void            checkTimedoutClients_udp();     // Check for timed out clients  

        Client *        addNewClient( SOCKETALT sock, bool is_tcp );  // Adds a new client to listen for
		void			sendLoginError( Client * clt, AddressIP ipadr);
		void			sendLoginAlready( Client * clt, AddressIP ipadr);
		void			sendLoginAccept( Client * clt, AddressIP ipadr, int acctnew);
		void			sendLoginUnavailable( Client * clt, AddressIP ipadr);

	public:
		NetServer();
		~NetServer();

		void	start( int argc, char ** argv);
		void	save();

		// WEAPON STUFF
		void	BroadcastUnfire( ObjSerial serial, int weapon_index, unsigned short zone);
		void	BroadcastFire( ObjSerial serial, int weapon_index, ObjSerial missile_serial, unsigned short zone);
		//void	sendDamages( ObjSerial serial, int zone, string shields, float recharge, char leak, unsigned short ab, unsigned short af, unsigned short al, unsigned short ar, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color);
		void	sendDamages( ObjSerial serial, unsigned short zone, Unit::Shield shields, Unit::Armor armor, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color);
		//void	sendDamages( ObjSerial serial, Vector & pnt, Vector & normal, float amt, GFXColor & color, float phasedamage);
		void	sendKill( ObjSerial serial, unsigned short zone);

		friend class ZoneMgr;
};

//void	str_cat( char *res, char c, char *s);

#endif

