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
#include "client.h"
#include "packet.h"

using std::list;
typedef list<Client *>::iterator LI;

class NetUI;

class ZoneMgr
{
		int nb_zones;
		//vector<StarSystem *> starsystems;
		list<Client *>		*zone_list;
		int					*zone_clients;

	public:
		ZoneMgr();
		ZoneMgr( int nbzones);
		~ZoneMgr();
		// Serial is the zone id
		list<Client *>	*GetZone( int serial);
		void	addClient( Client * clt, int zone);
		void	removeClient( Client * clt);
        void    broadcast( Client * clt, Packet * pckt );
        void    broadcastSnapshots();
		int		getZoneClients( Client * clt, char * bufzone);
		double	isVisible( Quaternion orient, QVector src_pos, QVector tar_pos);
};

#endif
