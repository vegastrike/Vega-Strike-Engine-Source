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
	LOGIN_UNAVAIL	 ,	// Packet sent from game server to client :
                                // account server is unavailable -> no login

	// Character creation stuff (not used yet)
    CMD_NEWCHAR      ,		// NOT USED : should get 
                                    // the new character description for new account
    CMD_INITIATE     ,		// NOT USED : should send
                                    // starting locations for new account
    CMD_CREATECHAR   ,
    CMD_LOCATIONS    ,

	// Commands used to add/remove ships in systems
    CMD_LETSGO       ,		// NOT USED
    CMD_ADDCLIENT    ,		// Client side :
                                    // request the server to be added in the game
									// Server side : adds the client to the game
    CMD_ENTERCLIENT  ,		// (Server -> Client) Indicates that a new
                                    // client is entering the current zone
									// also send the new client's data
    CMD_EXITCLIENT   ,		// (Server -> Client) Indicates that a client
                                    // is leaving the current zone
    CMD_ADDEDYOU     ,		// NOT USED

	// Ships movements data
    CMD_POSUPDATE    ,		// On client side (send): containing a ship's
                                    // data sent to server for position update
									// On client side (recv) : receive minimal update
                                    // data for a ship
									// On server side : sending minimal info
                                    // (just position) in case the ship is far from
									// the one we send other ships' data to
    CMD_UPDATECLT    ,
    CMD_FULLUPDATE   ,		// On client side (recv) : receive full update
                                    // info for a ship
    CMD_PING         ,		// A packet sending a ping-like command just to
                                    // tell to the server
									// that the client is still alive
    CMD_SNAPSHOT     ,		// A packet with all the necessary ships' data
                                    // in order to update

	// Weapons commands
	CMD_FIREREQUEST  ,		// Fire Request
	CMD_UNFIREREQUEST,		// UnFire Request
	CMD_BOLT         ,		// Fire a bolt
	CMD_BEAM         ,		// Fire a beam
	CMD_PROJECTILE   ,      // Fire a projectile
	CMD_BALL         ,		// Fire a ball ?
	CMD_TARGET       ,		// Receive a target request (server side)
                                    // or receive target info (client side)
	CMD_SCAN         ,		// Receive a scan request (server side)
                                    // or receive target info (client side)
	CMD_DAMAGE       ,		// Send damages to apply
	CMD_DAMAGE1      ,		// Send damages to apply
	CMD_SNAPDAMAGE   ,		// A snapshot of damages

	CMD_KILL         ,		// Send a kill to clients
	CMD_JUMP         ,		// Send a jump request
	CMD_ASKFILE      ,		// Ask the server for files (char=number of
                                    // files and string containing the file names)
	CMD_DOWNLOAD     ,		// May be identical to CMD_ASKFILE in the end
	CMD_CAMSHOT      ,		// This means a webcam-shot just arrived (from client to server and from server to concerned clients)
	CMD_SOUNDSAMPLE  ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)
	CMD_TXTMESSAGE   ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)
	CMD_STARTNETCOMM ,		// A client tells the server he starts a netcomm session
	CMD_STOPNETCOMM  ,		// A client tells the server he stops a netcomm session
	CMD_SECMESSAGE   ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)
	CMD_SECSNDSAMPLE ,		// This means a sound sample just arrived (from client to server and from server to concerned clients or from client to clients)

	CMD_DOCK         ,		// Dock request or authorization from server
	CMD_UNDOCK       ,		// Undock request or authorization from server

	CMD_CREATEUNIT   ,		// From server to clients : server unit creation
	CMD_CREATENEBULA ,		// From server to clients : server unit creation
	CMD_CREATEPLANET ,		// From server to clients : server unit creation
	CMD_CREATEASTER  ,		// From server to clients : server unit creation
	CMD_CREATEMISSILE,		// From server to clients : server unit creation

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

    CMD_DISCONNECT   ,		// UDP : after a problem (not clean exit) to make
                                    // the client exit if still alive

    // CMD_ACK          			// A packet to acknoledge the reception of a packet that was previously sent
};

enum CmdFlags
{
    CMD_CAN_COMPRESS = 0x1
};

std::ostream& operator<<( std::ostream& ostr, Cmd cmd );

const char* displayCmd( Cmd cmd );

#endif

