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
#include "networking/client.h"
#include "networking/lowlevel/packet.h"
#include "boost/smart_ptr.hpp"
#include "networking/clientptr.h"

using std::list;

typedef list<Unit *>::iterator LUI;

typedef std::map<std::string,std::string>            SystemMap;
typedef std::pair<std::string,std::string>           SystemPair;
typedef std::map<std::string,std::string>::iterator  SystemIt;

class NetUI;
class NetBuffer;

class ZoneMgr
{
    public:
        /* SnapshotSubCommand removes a double interpretation from
         * CMD_POSUPDATE and replaces CMD_FULLUPDATE.
         */
        enum SnapshotSubCommand
        {
            PosUpdate, // On client side (send): containing a ship's
                       // data sent to server for position update
                       // On client side (recv) : receive minimal update
                       // data for a ship
            FullUpdate // On client side (recv) : receive full update
                       // info for a ship
        };

    private:
		//vector<StarSystem *> starsystems;
		// List of clients in zones
		vector<ClientWeakList*> zone_list;
		vector<int>				zone_clients;
		// List of units in zones (but not Clients)
		//vector<list<Unit *> >	zone_unitlist;
		//vector<int>				zone_units;

	    class Systems
	    {
	        SystemMap _map;

	    public:
			std::string insert( std::string sysname, std::string systemxml );
			std::string get( std::string sysname );
	        bool        remove( std::string sysname );
	    };
		Systems			Systems;

		void	addDamage( NetBuffer & netbuf, Unit * un);
		bool	addPosition( NetBuffer & netbuf, Unit * un, ClientState & un_cs);

	public:

		ZoneMgr();
		//ZoneMgr( int nbzones);
		//~ZoneMgr();
		// Serial is the zone id
		void	addSystem( string & sysname, string & system);
		StarSystem* addZone( string starsys);
		ClientWeakList* GetZone( int serial);
		//void	addUnit( Unit * un, int zone);
		//void	removeUnit( Unit *un, int zone);
		Unit *	getUnit( ObjSerial unserial, unsigned short zone);

		StarSystem *	addClient( ClientWeakPtr clt, string starsys, unsigned short & num_zone);
		void	removeClient( ClientPtr clt );
        void    broadcast( ClientWeakPtr clt, Packet * pckt );
        void    broadcast( int zone, ObjSerial serial, Packet * pckt );
        void    broadcastNoSelf( int zone, ObjSerial serial, Packet * pckt );
        void    broadcastSample( int zone, ObjSerial serial, Packet * pckt, float frequency );
        void    broadcastText( int zone, ObjSerial serial, Packet * pckt, float frequency );
        void    broadcastSnapshots( bool update_planets=false);
		void	broadcastDamage();
		int		getZoneClients( ClientWeakPtr clt, char * bufzone);
		double	isVisible( Quaternion orient, QVector src_pos, QVector tar_pos);
		void	sendZoneClients( ClientWeakPtr clt );

		void	displayStats();
		int		displayMemory();
};

#endif
