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
#include "netclass.h"
#include "zonemgr.h"
#include "client.h"

extern VegaConfig *vs_config;
//extern VegaSimpleConfig *server_config;
using std::vector;
typedef list<Client *>::iterator LI;
typedef vector<Account *>::iterator VI;

/** Class Netserver : runs the server */
class NetServer
{
		NetUI *			Network;				// Network Interface
		TCPNetUI *		NetAcct;				// TCP Network Interface for requesting accounts server
		Packet			packet;					// Network data packet
		Packet			packeta;				// Network data packet for account server

		ZoneMgr			*zonemgr;				// Zones/star systems Manager
		int				nbclients;				// Active client connections number
		int				nbaccts;				// Number of registered accounts

		SocketAlt		conn_sock;				// Connection socket for game server
		SocketAlt		acct_sock;				// Connection socket for account server
		int				keeprun;				// Tells if we should keep running server
		int				snapchanged;			// Tells the snapshot has changed and can be sent
		int				acctserver;				// Tells if we use an account server
		ObjSerial		serial_seed;

		timeval				srvtimeout;			// timer

		vector<Account *>	Cltacct;			// Client accounts
		list<Client *>		Clients;			// Active client connections
		list<Client *>		discList;			// Client connections to be disconnected
		list<Client *>		logoutList;			// Client connections that logged out
#ifdef _TCP_PROTO
		queue<Client *>		waitList;			// Client connections waiting for login response
#else
		queue<AddressIP>	waitList;			// Client addresses waiting for login response
#endif

		//void			loadConfig();					// Loads configuration from server.xml
		void			authenticate( Client * clt, AddressIP sernum);	// Authenticate a connected client
		void			posUpdate( Client * clt);		// Update a client position
		void			addClient( Client * clt);		// Add the client in the game
		Client *		newConnection( AddressIP * ipadr);
		void			checkAcctMsg();					// Check for account server message to receive
		void			checkMsg();						// Check for network message to receive
		void			checkKey();						// Check for keyboard input
		void			recvMsg( Client * clt);			// Receive network messages
		void			processPacket( Client * clt, unsigned char cmd, AddressIP ipadr); // Process received packet info
		void			recvNewChar( Client * clt);		// Receive a new character
		void			sendLocations( Client * clt);	// Send the start locations
		void			startMsg();						// Startup showing text
		void			disconnect( Client * clt);		// Disconnect a client
		void			logout( Client * clt);			// Clean disconnect a client
		void			closeAllSockets();				// Disconnect all clients for shutdown
		void			checkTimedoutClients();			// Check for timed out clients	

		Client *		addNewClient( SocketAlt sock);	// Adds a new client to listen for
		ObjSerial		getUniqueSerial();				// Get a pseudo-unique serial
		void			sendLoginError( Client * clt, AddressIP ipadr);
		void			sendLoginAlready( Client * clt, AddressIP ipadr);
		void			sendLoginAccept( Client * clt, AddressIP ipadr, int acctnew);

	public:
		NetServer();
		~NetServer();

		void	start();
		void	save();

		friend class ZoneMgr;
};

//void	str_cat( char *res, char c, char *s);

#endif
