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

#ifndef VSNET_CMD_H
#define VSNET_CMD_H

#include <iostream>

/// Commands that are sent through the network
enum Cmd
{
	// Login commands
	CMD_SERVERTIME   ,	// Client side : Request the most up-to-date server time.
	                            // Server side : Send the current game time (double).
    CMD_LOGIN        ,	// Client side : login request sent to server with name and passwd
    CMD_LOGOUT       ,	// Client side : tells the server we are logging out
                                // Server side : remove the client from the game
    LOGIN_ERROR      ,	// Packet sent from account server to client :
                                // a login error (mostly bad name/passwd)
    LOGIN_ACCEPT     ,	// Packet sent from account server to client :
                                // login accepted
	LOGIN_DATA	     ,
    LOGIN_ALREADY    ,	// Packet sent from account server to client
                                // to tell account is already being used
    LOGIN_NEW        ,
    LOGIN_CHOOSESHIP ,
	LOGIN_UNAVAIL	 ,	// Packet sent from game server to client :
                                // account server is unavailable -> no login

	// Character creation stuff (not used yet)
    CMD_CREATECHAR   , // DELETEME?  Could come in handy from accountserver -> client...  but not so much from server...
    CMD_LOCATIONS    ,

	// Commands used to add/remove ships in systems
    CMD_ADDCLIENT    ,		// Client side :
                                    // request the server to be added in the game
									// Server side : adds the client to the game
    CMD_ENTERCLIENT  ,		// (Server -> Client) Indicates that a new
                                    // client is entering the current zone
									// also send the new client's data
			// Subcommands: ZoneMgr::AddClient,AddUnit,AddMissile,AddNebula,AddAsteroid,End
    CMD_EXITCLIENT   ,		// (Server -> Client) Indicates that a client
                                    // is leaving the current zone
    CMD_ADDEDYOU     ,		// Server -> Client
	CMD_RESPAWN      ,      // Client -> Server: Request a respawn after dying.

	// Ships movements data
    CMD_POSUPDATE    ,		// On client side (send): containing a ship's
                                    // data sent to server for position update
									// On client side (recv) : receive minimal update
                                    // data for a ship
									// On server side : sending minimal info
                                    // (just position) in case the ship is far from
									// the one we send other ships' data to
    CMD_PING         ,		// A packet sending a ping-like command just to
                                    // tell to the server
									// that the client is still alive
    CMD_SNAPSHOT     ,		// A packet with all the necessary ships' data
                                    // in order to update

	// Weapons commands
	CMD_FIREREQUEST  ,		// Fire Request
	CMD_UNFIREREQUEST,		// UnFire Request
	CMD_TARGET       ,		// Receive a target request (server side)
	CMD_CLOAK        ,
                                    // or receive target info (client side)
	CMD_SCAN         ,		// Receive a scan request (server side)
                                    // or receive target info (client side)
	CMD_DAMAGE       ,		// Send damages to apply
	CMD_SNAPDAMAGE   ,		// A snapshot of damages

	CMD_KILL         ,		// Send a kill to clients
	CMD_JUMP         ,		// Send a jump request
	CMD_ASKFILE      ,		// Ask the server for files (char=number of
                                    // files and string containing the file names)
	CMD_DOWNLOAD     ,		// May be identical to CMD_ASKFILE in the end
	CMD_CAMSHOT      ,		// This means a webcam-shot just arrived (from client to server and from server to concerned clients)
	CMD_SOUNDSAMPLE  ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)
	CMD_TXTMESSAGE   ,		// This means a text message has arrived
	CMD_STARTNETCOMM ,		// A client tells the server he starts a netcomm session
	CMD_STOPNETCOMM  ,		// A client tells the server he stops a netcomm session
	CMD_SECMESSAGE   ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)
	CMD_SECSNDSAMPLE ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)

	CMD_DOCK         ,		// Dock request or authorization from server
	CMD_UNDOCK       ,		// Undock request or authorization from server

	CMD_SNAPCARGO,			// S->C: A full update of the up-to-date cargo inventory in all ships.  Sent when logging in.
	CMD_CARGOUPGRADE,		// C->S and S->C (broadcast): buy/sell cargo or upgrade.
	CMD_CREDITS,            // S->C: updates the number of credits.  Required after a cargo/upgrade transaction.
	
	// Account management commands
	CMD_RESYNCACCOUNTS ,		// When connection to account server was lost
                                    // and established again :
									// send the account server a list of active
                                    // client to sync and log out
									// client that may have quit the game during
                                    // loss of connection
	CMD_SAVEACCOUNTS ,		// Send a client save to account server in order
                                    // to do a backup
    CMD_NEWSUBSCRIBE ,		// Account server : handle a new account creation
                                    // (login/passwd) on the web
	CMD_CONNECT,			// C->S, response S->C: Upon opening connection
	CMD_CHOOSESHIP,			// S->C, response C->S: Server needs client to choose a ship
									// before it can send LOGIN_ACCEPT.

    CMD_DISCONNECT   ,		// UDP : after a problem (not clean exit) to make
                                    // the client exit if still alive

	CMD_SAVEDATA,
	CMD_MISSION,
	
    // CMD_ACK          			// A packet to acknoledge the reception of a packet that was previously sent
};

class Subcmd {
public:
	enum {
		StringValue = 1<<1,
		FloatValue  = 1<<2,
		Objective   = 1<<3,
	};

	enum {
		SetValue    = 1<<8,
		EraseValue  = 1<<9,
	};

	enum {
		TerminateMission = 1,
		AcceptMission = 2,
	};

};

std::ostream& operator<<( std::ostream& ostr, Cmd cmd );

const char* displayCmd( Cmd cmd );

#endif

