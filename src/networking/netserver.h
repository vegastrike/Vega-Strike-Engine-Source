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

#include "boost/shared_ptr.hpp"
#include "configxml.h"
#include "accountsxml.h"
#include "const.h"
#include "netui.h"
#include "zonemgr.h"
#include "client.h"
#include "savegame.h"
#include "networking/vsnet_socketset.h"
struct GFXColor;

extern VegaConfig *vs_config;
//extern VegaSimpleConfig *server_config;
using std::vector;
// typedef list<Client *>::iterator LI;
typedef vector<Account *>::iterator VI;

struct ServerSocket;
class SocketSet;

namespace VsnetDownload {
  namespace Server {
    class Manager;
  };
};

/** Class Netserver : runs the "game server" */
class NetServer
{
        SocketSet       _sock_set;              // Capsule for select()

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
        ClientList          allClients;         // Active TCP and UDP client connections
		ClientList          discList;			// Client connections to be disconnected
		ClientList          logoutList;			// Client connections that logged out

	    struct WaitListEntry
	    {
	        bool tcp;
	        ClientPtr t; // Client connections waiting for login response
		    AddressIP u; // Client addresses waiting for login response
	    };

        queue<WaitListEntry> waitList;

        boost::shared_ptr<VsnetDownload::Server::Manager> _downloadManagerServer;

		bool			updateTimestamps( ClientPtr clt, Packet & p);
		//void			loadConfig();					// Loads configuration from server.xml
		void			authenticate( ClientPtr clt, AddressIP sernum, Packet& packet );	// Authenticate a connected client
		void			posUpdate( ClientPtr clt);		// Update a client position
		void			addClient( ClientPtr clt);		// Add the client in the game
		void			removeClient( ClientPtr clt);		// Remove the client from the game
		void			checkSystem( ClientPtr clt);		// Check if the client has the good system file
		ClientPtr       newConnection_udp( const AddressIP& ipadr);
		ClientPtr       newConnection_tcp( );
		void			checkAcctMsg( SocketSet& set );			// Check for account server message to receive
		void			checkMsg( SocketSet& set );				// Check for network message to receive
		void			checkKey( SocketSet& set);						// Check for keyboard input
		void			recvMsg_tcp( ClientPtr clt);		// Receive network messages
		void			recvMsg_udp( );					// Receive network messages
		void			processPacket( ClientPtr clt, unsigned char cmd, const AddressIP& ipadr, Packet& packet ); // Process received packet info
		void			recvNewChar( ClientPtr clt);		// Receive a new character
		void			sendLocations( ClientPtr clt);	// Send the start locations
		void			startMsg();						// Startup showing text
		void			disconnect( ClientPtr clt, const char* debug_from_file, int debug_from_line );		// Disconnect a client
		void			logout( ClientPtr clt);			// Clean disconnect a client
		void			closeAllSockets();				// Disconnect all clients for shutdown
        void            checkTimedoutClients_udp();     // Check for timed out clients  

        ClientPtr       addNewClient( SOCKETALT sock, bool is_tcp );  // Adds a new client to listen for
		void			sendLoginError( ClientPtr clt, AddressIP ipadr);
		void			sendLoginAlready( ClientPtr clt, AddressIP ipadr);
		void			sendLoginAccept( ClientPtr clt, AddressIP ipadr, int acctnew);
		void			sendLoginUnavailable( ClientPtr clt, AddressIP ipadr);
		void			getMD5( string filename, unsigned char * md5digest);
		ClientPtr       getClientFromSerial( ObjSerial serial);

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
		void	sendJump( ObjSerial serial, bool ok);

		friend class ZoneMgr;
};

//void	str_cat( char *res, char c, char *s);

#endif

