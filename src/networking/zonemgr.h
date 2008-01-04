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

class ZoneInfo
{
	friend class ZoneMgr;
	ClientList		zone_list;
	int				zone_clients;

	int zonenum;
	static unsigned short next_zonenum;
	StarSystem *star_system;
	
	ZoneInfo(StarSystem *parent) {
		this->zonenum = next_zonenum;
		this->zone_clients = 0;
		this->star_system = parent;
		next_zonenum ++;
	}
public:
	unsigned short getZoneNum() {
		return zonenum;
	}
	StarSystem *getSystem() {
		return star_system;
	}
};

typedef std::map<unsigned short, ZoneInfo> ZoneMap;

class ZoneMgr
{
    public:
        /* SnapshotSubCommand removes a double interpretation from
         * CMD_POSUPDATE and replaces CMD_FULLUPDATE.
         */
        enum SnapshotSubCommand
        {
            End=0,		 // On client side (recv) : tells the buffer ends
            PosUpdate=1, // On client side (send): containing a ship's
                       // data sent to server for position update
                       // On client side (recv) : receive minimal update
                       // data for a ship
            FullUpdate=2,// On client side (recv) : receive full update
                       // info for a ship
            DamageUpdate=8, // On client side (recv) : update changed damages.
        };
        enum AddClientId
        {
//          End=0, // Already defined... // Tells client that the buffer ends.
			AddClient=1,
            AddUnit=2,
            AddNebula=3,
            AddPlanet=4, // Not implented
            AddAsteroid=5,
            AddMissile=6
        };

    private:
		//vector<StarSystem *> starsystems;
		// List of clients in zones
		//vector<ClientList*>		zone_list;
		//vector<int>				zone_clients;
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
		bool	addPosition( ClientPtr k, NetBuffer & netbuf, Unit * un, ClientState & un_cs);

	public:
		ZoneMap zones;

		ZoneMgr();
		//ZoneMgr( int nbzones);
		//~ZoneMgr();
		// Serial is the zone id
		void	addSystem( string & sysname, string & system);
		string	getSystem( string & name);
		StarSystem* addZone( string starsys);
		ZoneInfo* GetZoneInfo( int serial);
		ClientList* GetZone( int serial);
		//void	addUnit( Unit * un, int zone);
		//void	removeUnit( Unit *un, int zone);
		Unit *	getUnit( ObjSerial unserial, unsigned short zone);

		StarSystem *	addClient( ClientPtr clt, string starsys, unsigned short & num_zone);
		void	removeClient( ClientPtr clt );
		void    broadcast( ClientPtr clt, Packet * pckt, bool isTcp, unsigned short minVer=0, unsigned short maxVer=65535 );
        void    broadcast( int zone, ObjSerial serial, Packet * pckt, bool isTcp, unsigned short minVer=0, unsigned short maxVer=65535 );
        void    broadcastNoSelf( int zone, ObjSerial serial, Packet * pckt, bool isTcp );
        void    broadcastSample( int zone, ObjSerial serial, Packet * pckt, float frequency );
        void    broadcastText( int zone, ObjSerial serial, Packet * pckt, float frequency );
        void    broadcastSnapshots( bool update_planets=false);
		void	broadcastDamage();
		double	isVisible( Quaternion orient, QVector src_pos, QVector tar_pos);

		void	displayStats();
		int		displayMemory();
};

#endif
