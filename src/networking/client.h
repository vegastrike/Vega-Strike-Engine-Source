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

#include <string.h>
#include "gfx/quaternion.h"
#include "const.h"
#include "netclass.h"
#include "cubicsplines.h"
#include "configxml.h"

extern VegaConfig *vs_config;

class Unit;
struct Client;

// Structure used to transmit client updates
class	ClientState
{
	unsigned int	delay;
	ObjSerial		client_serial;
	//QVector		pos;
	//Quaternion	orient;
	Transformation	pos;
	Vector			veloc;
	Vector			accel;

	public:
		ClientState();
		ClientState( ObjSerial serial);
		ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc);
		ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, unsigned int del);
		ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, unsigned int del);

		QVector		getPosition() { return this->pos.position;}
		Quaternion	getOrientation() { return this->pos.orientation;}
		Vector		getVelocity() { return this->veloc;}
		Vector		getAcceleration() { return this->accel;}
		ObjSerial	getSerial() { return this->client_serial;}
		unsigned int	getDelay() { return this->delay;}
		void		setDelay( unsigned int del) { this->delay = del;}
		void		setSerial( ObjSerial ser) { this->client_serial = ser;}
		void		setPosition( QVector posit)
		{
			this->pos.position = posit;
			// memcpy( &this->position, &posit, sizeof( QVector));
		}

		void		display();
		int		operator==( const ClientState & ctmp);
		void	tosend();
		void	received();
		//friend class Client;
};

// Description of a client's ship (ship type, weapons, shields status...)
struct ClientDescription
{
};

// Network part of a client description
struct	Client
{
	/* Network and identification properties */
	Unit	*		game_unit;
	CubicSpline		spline;
	AddressIP		cltadr;
	SOCKETALT		sock;
	ObjSerial		serial;
	// 2 timeout vals to check a timeout for client connections
	// those vals are server times
	double		old_timeout;
	double		latest_timeout;
	// 2 timestamps vals from client time to check receiving old packet after newer ones
	double			old_timestamp;
	double			latest_timestamp;
	unsigned int	deltatime;
	int				sdl_channel;
	char			name[NAMELEN];
	char			passwd[NAMELEN];
	ClientState		old_state;
	ClientState		current_state;
	ClientDescription	current_desc;
	/* In-game parameters */
	int				zone;
	char			ingame;
	//char			localplayer;

	Client()
	{
		memset( &old_state, 0, sizeof( ClientState));
		memset( &current_state, 0, sizeof( ClientState));
		latest_timestamp=0;
		old_timestamp=0;
		latest_timeout=0;
		old_timeout=0;
		deltatime=0;
		ingame=0;
	}
};

void	LoadXMLUnit( Unit * unit, const char * filename, char * buf);
void	WriteXMLUnit( string filename, char * xmlbuf, int tsize);

#endif
