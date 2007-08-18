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
 * You should have recvbufd a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  client structures - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __CLIENTSTRUCTS_H
#define __CLIENTSTRUCTS_H

#include <iostream>
#include <string.h>
#include "gfx/quaternion.h"
#include "networking/const.h"
#include "configxml.h"
#include "networking/lowlevel/vsnet_clientstate.h"
#include "networking/lowlevel/vsnet_address.h"
#include "networking/lowlevel/vsnet_socket.h"
//#include "cmd/unit_generic.h"
#include "cmd/container.h"
#include "savegame.h"

class Unit;
class UnitContainer;
class Prediction;

extern VegaConfig *vs_config;

void	displayError( ObjSerial error_code);
enum accountServerCommands{
  ACCT_LOGIN='l',
  ACCT_NEWCHAR='c',
  ACCT_NEWSUBSCRIBE='u',
  ACCT_LOGIN_DATA='d',
  ACCT_LOGIN_ERROR='e',
  ACCT_LOGIN_ALREADY='f',
  ACCT_LOGIN_NEW='n',
  ACCT_LOGIN_ACCEPT='a',
  ACCT_LOGOUT='o',
  ACCT_RESYNC='r',
  ACCT_SAVE='s',
  ACCT_SAVE_LOGOUT='S',
  ACCT_SUCCESS='!',
};
// Network part of a client description
class	Client
{
public:
	/* Network and identification properties */
	UnitContainer	game_unit;

	// NETFIXME: Move to server subclass.
	AddressIP		cltadr;
	AddressIP		cltudpadr;
	SOCKETALT *		lossy_socket;
	SOCKETALT		tcp_sock;

	// 2 timeout vals to check a timeout for client connections
	// those vals are server times
	double			old_timeout;
	double			latest_timeout;
	double			elapsed_since_packet;
	string			callsign;
	string			name;
	string			passwd;
	/* In-game parameters */
	bool			ingame;
	enum Loginstate {CONNECTED, LOGGEDIN, SAVEDGAME, INGAME};
	int loginstate;
	
	char			webcam;
	char			portaudio;
	char			secured;
	char			jumpok;
	string			jumpfile;
	ObjSerial		netversion;
	vector<string>	savegame;
	float			comm_freq;
	ClientState     last_packet;        // Last FullUpdate packet recieved.

    string			_disconnectReason;

	Prediction*		prediction;
	
	Client();
	Client( SOCKETALT& s );
	~Client();

    void         setLatestTimestamp( unsigned int ts );
	void         setUDP(SOCKETALT *udpSock, AddressIP &cltudpadr);
	void         setTCP();
    void         clearLatestTimestamp( );
    unsigned int getLatestTimestamp( ) const;
    double       getDeltatime( ) const;
	double       getNextDeltatime( ) const;

	friend std::ostream& operator<<( std::ostream& ostr, const Client& c );

private:
	// 2 timestamps vals from client time to check receiving old packet after
    // newer ones (allowed in case of UDP only)
	unsigned int	_old_timestamp;
	unsigned int	_latest_timestamp;
	double			_deltatime;
	double			_next_deltatime;

private:
	Client( const Client& );
	Client& operator=( const Client& );

	void Init();
};

std::ostream& operator<<( std::ostream& ostr, const Client& c );

#endif
