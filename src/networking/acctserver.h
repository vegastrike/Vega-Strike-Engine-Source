#ifndef __ACCTSERVER_H
#define __ACCTSERVER_H

#include <vector>
#include "configxml.h"
#include "netclass.h"
#include "accountsxml.h"
#include "packet.h"

extern VegaSimpleConfig * acct_config;

typedef vector<Account *>::iterator VI;

/** Class that manages player accounts
 * - Receives authentication request packets
 * - Returns a packet with LOGIN_ACCEPT or LOGIN_ERROR and with player name
 */

typedef list<SocketAlt>::iterator LS;

/**
 * This class should also :
 * - manage accounts info (ships descriptions...)
 * - handle future web subscriptions
 */

class AccountServer
{
		TCPNetUI *			Network;	// Network Interface
		vector<Account *>	Cltacct;	// Client accounts
		list<SocketAlt>		Socks;		// List of active sockets on which we can receive requests
		list<SocketAlt>		DeadSocks;		// List of sockets to close

		int			newaccounts;
		int			keeprun;
		SocketAlt	conn_sock;
		ObjSerial	serial_seed;
		Packet		packet;

	public:
		AccountServer();
		~AccountServer();

		void		startMsg();
		void		start();		// Starts the server
		void		save();			// Saves new accounts if there are
		void		checkMsg();		// Check for network message to receive
		void		recvMsg( SocketAlt sock);		// Receive message
		void		sendAuthorized( SocketAlt sock, Account * acct);				// Send authorization and related data
		void		sendUnauthorized( SocketAlt sock, Account * acct);				// Send unauthorizated connection
		void		sendAlreadyConnected(  SocketAlt sock, Account * acct);
		ObjSerial	getUniqueSerial();
};

#endif
