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
  NetClient - Network Client Interface - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __NetClient_H
#define __NetClient_H

#include <string>
#include "vegastrike.h"
#include "gfx/quaternion.h"
#include "netclass.h"
#include "packet.h"
#include "client.h"

//using std::vector;

//typedef vector<Client *>::iterator VI;

class	NetClient
{
		NetUI *				Network;		// Network interface
		Unit *				game_unit;		// Unit struct from the game corresponding to that client
		Packet				packet;			// Network data packet

		SocketAlt			clt_sock;		// Comm. socket
		ObjSerial			serial;			// Serial # of client
		int					nbclients;		// Number of clients in the zone
		char				keeprun;		// Bool to test client stop
		AddressIP			cltadr;			// Client IP
		Client *			Clients[MAXCLIENTS];		// Clients in the same zone
		// a vector because always accessed by their IDs

		int					enabled;		// Bool to say network is enabled
		// Time used for refresh - not sure still used
		int					old_time;
		double				cur_time;
		// Timestamps from packets
		unsigned int		old_timestamp;
		unsigned int		current_timestamp;
		unsigned int		deltatime;

		void	receiveShip();
		void	receiveData();
		void	receiveLocations();
		void	readDatafiles();
		void	createChar();
		int		recvMsg();
		void	getZoneData();
		void	disconnect();

	public:
		NetClient()
		{
			game_unit = NULL;
			old_timestamp = 0;
			current_timestamp = 0;
			old_time = 0;
			cur_time = 0;
			enabled = 0;
			nbclients = 0;
			for( int i=0; i<MAXCLIENTS; i++)
				Clients[i] = NULL;
		}
		~NetClient()
		{
			if( Network!=NULL)
				delete Network;
			for( int i=0; i<MAXCLIENTS; i++)
			{
				if( Clients[i]!=NULL)
					delete Clients[i];
			}
		}

		int		authenticate();
		void	start( char * addr, unsigned short port);
		int		init( char * addr, unsigned short port);
		void	checkKey();
		void	receivePosition();
		void	sendPosition( ClientState cs);
		void	sendAlive();

		int		loginLoop( string str_name, string str_passwd); // Loops until receiving login response
		void	addClient();
		void	removeClient();
		void	disable() { enabled=false;}
		int		isEnabled() { return enabled; }
		void	setNetworkedMode( bool mode) { enabled = mode;}
		int		checkMsg();
		ObjSerial	getSerial() { return serial; }
		void	inGame();
		int		isTime();
		void	logout();
		unsigned int	getLag() { return deltatime;}

		void			init_interpolation( ObjSerial clientid);
		Transformation	spline_interpolate( ObjSerial clientid, double blend);
};

#endif
