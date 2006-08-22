#ifndef __ACCTSERVER_H
#define __ACCTSERVER_H

#define NO_UNIT_STUFF
#include <vector>

#include "configxml.h"
#include "networking/accountsxml.h"
#include "networking/lowlevel/packet.h"
#include "networking/lowlevel/netui.h"
#include "networking/lowlevel/vsnet_socketset.h"

typedef vector<Account *>::iterator VI;

/** Class that manages player accounts
 * - Receives authentication request packets
 * - Returns a packet with LOGIN_ACCEPT or LOGIN_ERROR and with player name
 */

/**
 * This class should also :
 * - manage accounts info (ships descriptions...)
 * - handle future web subscriptions
 */

class AccountServer
{
        SocketSet           _sock_set;

		NetUITCP			NetworkToClient;
		ServerSocket*		Network;
		// TCPNetUI *		Network;	// Network Interface
		list<SOCKETALT>		Socks;		// List of active sockets on which we can receive requests
		list<SOCKETALT>		DeadSocks;	// List of sockets to close

		int			newaccounts;
		int			keeprun;
		// SOCKETALT	conn_sock;
		Packet		packet;

		double	savetime;
		double	curtime;

	public:
		AccountServer();
		~AccountServer();

		void		startMsg();
		void		start();		// Starts the server
		void		save();			// Saves new accounts if there are
		void		recvMsg( SOCKETALT sock);		// Receive message
		void		sendAuthorized( SOCKETALT sock, Account * acct);				// Send authorization and related data
		void		sendUnauthorized( SOCKETALT sock, Account * acct);				// Send unauthorizated connection
		void		sendAlreadyConnected(  SOCKETALT sock, Account * acct);
		void		sendServerData(  SOCKETALT sock, Account * acct);

	void		writeSave( const char * buffer, unsigned int bufferlen);
		void		removeDeadSockets();
};

#endif
