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
  Zone (StarSystems) Manager - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __ZONEMGR_H
#define __ZONEMGR_H

#include <list>
#include <iostream>
#include "gfx/quaternion.h"
#include "cmd/unit_generic.h"
#include "client.h"
#include "packet.h"

using std::list;
typedef list<Client *>::iterator LI;
typedef vector<list<Client *> >::iterator VLI;
typedef list<Unit *>::iterator LUI;

class NetUI;

class ZoneMgr
{
		//vector<StarSystem *> starsystems;
		// List of clients in zones
		vector<list<Client *> >	zone_list;
		vector<int>				zone_clients;
		// List of units in zones (but not Clients)
		vector<list<Unit *> >	zone_unitlist;
		vector<int>				zone_units;

	public:
		ZoneMgr();
		//ZoneMgr( int nbzones);
		//~ZoneMgr();
		// Serial is the zone id
		StarSystem *	addZone( string starsys);
		list<Client *>	GetZone( int serial);
		//void	addClient( Client * clt, int zone);
		void	addUnit( Unit * un, int zone);
		void	removeUnit( Unit *un, int zone);
		Unit *	getUnit( ObjSerial unserial, int zone);

		bool	addClient( Client * clt, string starsys, int & num_zone);
		void	removeClient( Client * clt);
        void    broadcast( Client * clt, Packet * pckt );
        void    broadcast( int zone, ObjSerial serial, Packet * pckt );
        void    broadcastSnapshots();
		int		getZoneClients( Client * clt, char * bufzone);
		double	isVisible( Quaternion orient, QVector src_pos, QVector tar_pos);
		void	sendZoneClients( Client * clt);
};

#endif
