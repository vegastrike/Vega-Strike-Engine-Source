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

#include <config.h> // for HAVE_ZLIB

#include <vector>
#include <list>
// WARNING : using std::queue gives conflicts under Solaris so we must use std::queue explicitly each time we need to
#include <queue>
#include <map>
#include <string.h>
#include <stdlib.h>

#include <boost/shared_ptr.hpp>
#include "configxml.h"
#include "networking/accountsxml.h"
#include "networking/const.h"
#include "networking/lowlevel/packet.h"
#include "networking/lowlevel/netui.h"
#include "networking/lowlevel/vsnet_socketset.h"
#include "networking/client.h"
#include "networking/clientptr.h"
#include "gfx/quaternion.h"
#include "cmd/unit_generic.h"
#include "boost/smart_ptr.hpp"

struct GFXColor;

extern VegaConfig *vs_config;
//extern VegaSimpleConfig *server_config;
using std::vector;
// typedef list<Client *>::iterator LI;
typedef vector<Account *>::iterator VI;

extern ObjSerial SERVER_NETVERSION;

struct ServerSocket;
class SocketSet;

namespace VsnetDownload {
  namespace Server {
    class Manager;
  };
};
class VsnetHTTPSocket;
/** Class Netserver : runs the "game server" */
class NetServer
{
        SocketSet       _sock_set;              // Capsule for select()

        ServerSocket*   tcpNetwork;
        SOCKETALT       *udpNetwork;
		Packet			packet;					// Network data packet
		Packet			packeta;				// Network data packet for account server

		SaveGame*		globalsave;				// Savegame class used to save dynamic universe
		int				nbclients;				// Active client connections number
		int				nbaccts;				// Number of registered accounts

		VsnetHTTPSocket*		acct_sock;				// Connection socket for account server
		int				keeprun;				// Tells if we should keep running server
		int				snapchanged;			// Tells the snapshot has changed and can be sent
		
		int				acctserver;				// Tells if we use an account server
		// If we don't use an account server:
		string				server_password;
		
		timeval				srvtimeout;			// timer
		std::queue<int>			unused_players;

		// queue of units added to system but not sent to clients yet.
		std::vector<UnitContainer> newUnits;

//		vector<Account *>	Cltacct;			// Client accounts
        ClientList          allClients;         // Active TCP and UDP client connections
		ClientList          discList;			// Client connections to be disconnected
		ClientList          logoutList;			// Client connections that logged out

	    struct WaitListEntry
	    {
              enum EntryTypes{CONNECTING, JUMPING};
              EntryTypes type;
              bool tcp;
              ClientPtr t; // Client connections waiting for login response
              AddressIP u; // Client addresses waiting for login response
	    };

        std::map<string, WaitListEntry> waitList;

        boost::shared_ptr<VsnetDownload::Server::Manager> _downloadManagerServer;
        static const char*                                _downloadSearchDirs[];

	public:
		class ZoneMgr		*zonemgr;				// Zones/star systems Manager
	private:
	
		bool			updateTimestamps( ClientPtr clt, Packet & p);
		//void			loadConfig();					// Loads configuration from server.xml
		void			posUpdate( ClientPtr clt);		// Update a client position
		void			addClient( ClientPtr clt);		// Add the client in the game
		void			chooseShip( ClientPtr clt, Packet &p);	// Client has chosen a ship to fly in.
		void			localLogin( ClientPtr clt, Packet &p);	// No account server... client will choose a ship.
		bool			loginAccept( std::string inetbuf,ClientPtr clt, int newacct, char flags);
		void			serverTimeInitUDP( ClientPtr clt, NetBuffer &netbuf);
		void			removeClient( ClientPtr clt);		// Remove the client from the game
		ClientPtr       newConnection_udp( const AddressIP& ipadr);
		ClientPtr       newConnection_tcp( );
		void			checkAcctMsg( SocketSet& set );			// Check for account server message to receive
		void			checkMsg( SocketSet& set );				// Check for network message to receive
		void			checkKey( SocketSet& set);						// Check for keyboard input
		void			recvMsg_tcp( ClientPtr clt);		// Receive network messages
		void			recvMsg_udp( );					// Receive network messages
		void			processPacket( ClientPtr clt, unsigned char cmd, const AddressIP& ipadr, Packet& packet ); // Process received packet info
		void			startMsg();						// Startup showing text
		void			disconnect( ClientPtr clt, const char* debug_from_file, int debug_from_line );		// Disconnect a client
		void			logout( ClientPtr clt);			// Clean disconnect a client
        void            checkTimedoutClients_udp();     // Check for timed out clients  

        ClientPtr       addNewClient( SOCKETALT &sock );  // Adds a new client to listen for.
		void			sendLoginError( ClientPtr clt);
		void			sendLoginAlready( ClientPtr clt);
		void			sendLoginAccept( ClientPtr clt, Cockpit *cp);
  //returns false if unwilling to load star system
		void			sendLoginUnavailable( ClientPtr clt);

		// loadCockpit will fail if a cockpit already exists when client state is CONNECTED.
		Cockpit *		loadCockpit(ClientPtr clt );
		bool			loadFromSavegame( ClientPtr clt, Cockpit *cp );
		bool			loadFromNewGame( ClientPtr clt, Cockpit *cp, string shipname );
		ClientPtr       getClientFromSerial( ObjSerial serial);
	public:
		NetServer();
		~NetServer();

		void	start( int argc, char ** argv);
		void	save();

		void	broadcast( NetBuffer & netbuf, ObjSerial serial, unsigned short zone, Cmd command, bool isTcp );

		void	broadcastUnit( Unit* un, unsigned short zone);
		void	sendNewUnitQueue();
		// WEAPON STUFF
		void	BroadcastCargoUpgrade( ObjSerial sender, ObjSerial buyer, ObjSerial seller, const std::string &cargo,
							float price, float mass, float volume, bool mission, unsigned int quantity,
							int mountOffset, int subunitOffset, unsigned short zone);
		void	BroadcastTarget( ObjSerial serial, ObjSerial oldtarg, ObjSerial target, unsigned short zone);
		void	BroadcastUnfire( ObjSerial serial, const vector<int> &weapon_indicies, unsigned short zone);
		void	BroadcastFire( ObjSerial serial, const vector<int> &weapon_indicies, ObjSerial missile_serial, float energy, unsigned short zone);
		//void	sendDamages( ObjSerial serial, int zone, string shields, float recharge, char leak, unsigned short ab, unsigned short af, unsigned short al, unsigned short ar, float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color);
	
		bool			saveAccount(int cpnum );
	
		void	sendCustom( int cp, string cmd, string args, string id);
		void	sendDamages( ObjSerial serial, unsigned short zone, float hull, const Shield &shields, const Armor &armor,
							float ppercentage, float spercentage, float amt, Vector & pnt, Vector & normal, GFXColor & color);
		//void	sendDamages( ObjSerial serial, Vector & pnt, Vector & normal, float amt, GFXColor & color, float phasedamage);
		void	sendKill( ObjSerial serial, unsigned short zone);
		void	sendJump( Unit * src, Unit *jumppoint,std::string destination);
		void	sendJumpFinal( ClientPtr clt, string server, unsigned short port );
		void	sendForcePosition( ClientPtr clt );

		void	invalidateSnapshot( ) { snapchanged = 1; }

		// DOCKING STUFF
		void	sendDockAuthorize( ObjSerial serial, ObjSerial utdwserial, int docknum, unsigned short zone);
		void	sendDockDeny( ObjSerial serial, unsigned short zone);
		void	sendUnDock( ObjSerial serial, ObjSerial utdwserial, unsigned short zone);
		void	sendCredits( ObjSerial serial, float creds);
		void	addUnitCargoSnapshot( const Unit *un, NetBuffer &netbuf);
		void	sendCargoSnapshot( ObjSerial serial, const UnitCollection &unitlist);

		void	sendMessage(string from, string to, string message, float delay);
		void	sendSaveData( int cp, unsigned short packetType, int pos, string *key,
							  Mission *miss, // Mission number for objectives.
							  string *strValue, float *floatValue);
		void	sendMission( int cp, unsigned short packetType, string mission, int pos);

		void	addSystem( string & sysname, string & system);
		friend class ZoneMgr;

		void			closeAllSockets();				// Disconnect all clients for shutdown
};

//void	str_cat( char *res, char c, char *s);

#endif

