#ifndef __ACCTSERVER_H
#define __ACCTSERVER_H

#include <vector>

#include "configxml.h"
#include "accountsxml.h"
#include "packet.h"
#include "netui.h"

typedef vector<Account *>::iterator VI;

/** Class that manages player accounts
 * - Receives authentication request packets
 * - Returns a packet with LOGIN_ACCEPT or LOGIN_ERROR and with player name
 */

typedef list<SOCKETALT>::iterator LS;

/**
 * This class should also :
 * - manage accounts info (ships descriptions...)
 * - handle future web subscriptions
 */

class AccountServer
{
		NetUITCP			NetworkToClient;
		ServerSocket*		Network;
		// TCPNetUI *		Network;	// Network Interface
		vector<Account *>	Cltacct;	// Client accounts
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

		void		writeSave( const char * buffer);
		void		removeDeadSockets();
};

#endif
