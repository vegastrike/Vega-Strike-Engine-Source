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

#ifndef VSNET_CLIENTSTATE_H
#define VSNET_CLIENTSTATE_H

#include <iostream>
#include <string.h>
#include "gfx/quaternion.h"
#include "networking/const.h"
#include "configxml.h"

class Unit;
class NetBuffer;

// Structure used to transmit client updates
class	ClientState
{
	//float			delay;
	ObjSerial		client_serial;
	Transformation	pos;
	Vector			veloc;
	Vector			accel;

	public:
		ClientState();
		ClientState( ObjSerial serial);
		ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc);
		ClientState( ObjSerial serial, QVector posit, Quaternion orientat, Vector velocity, Vector acc, unsigned int del);
		ClientState( ObjSerial serial, Transformation trans, Vector velocity, Vector acc, unsigned int del);
		ClientState( Unit * un);

		QVector		getPosition() { return this->pos.position;}
		Quaternion	getOrientation() { return this->pos.orientation;}
		Vector		getVelocity() { return this->veloc;}
		Vector		getAcceleration() { return this->accel;}
		ObjSerial	getSerial() const { return this->client_serial;}
		//float		getDelay() { return this->delay;}
		//void		setDelay( float del) { this->delay = del;}
		void		setSerial( ObjSerial ser) { this->client_serial = ser;}
		void		setPosition( QVector posit) { this->pos.position = posit;}
		void		setOrientation( Quaternion orient) { this->pos.orientation = orient;}
		void		setAcceleration( Vector acc) { this->accel = acc;}
		void		setVelocity( Vector vel) { this->veloc = vel;}

		void	display() const;
		int		operator==( const ClientState & ctmp);
		void	netswap();

		friend	class NetBuffer;
};

#endif

