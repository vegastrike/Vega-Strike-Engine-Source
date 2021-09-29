/**
* unit_factory_server.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include "unit_factory.h"
#include "unit_generic.h"
#include "gfx/cockpit_generic.h"
#include "nebula_generic.h"
#include "planet_generic.h"
#include "asteroid_generic.h"
#include "missile_generic.h"
#include "enhancement_generic.h"
#if defined (_WIN32) && !defined (__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "networking/lowlevel/netbuffer.h"
#include "networking/zonemgr.h"
#include "networking/netserver.h"

Unit* UnitFactory::createUnit()
{
    return new Unit( 0 );
}

Unit* UnitFactory::createUnit( const char *filename,
                               bool SubUnit,
                               int faction,
                               std::string customizedUnit,
                               Flightgroup *flightgroup,
                               int fg_subnumber,
                               string *netxml,
                               ObjSerial netcreate )
{
    _Universe->netLock( true );
    Unit *un = new Unit( filename,
                         SubUnit,
                         faction,
                         customizedUnit,
                         flightgroup,
                         fg_subnumber, netxml );
    _Universe->netLock( false );
    if (netcreate)
        //Send a packet to clients in order to make them create this unit
        un->SetSerial( netcreate );
/*		if (!_Universe->netLocked()) {
 *                       NetBuffer netbuf;
 *
 *                       // NETFIXME: addBuffer for all subunits?
 *                       addUnitBuffer(netbuf, un, netxml);
 *                       endBuffer(netbuf);
 *                       // Broadcast to the current universe star system
 *                       VSServer->broadcast( netbuf, 0, _Universe->activeStarSystem()->GetZone(), CMD_ENTERCLIENT, true);
 *               }
 *               VSServer->invalidateSnapshot();
 */
    return un;
}
Unit* UnitFactory::createServerSideUnit( const char *filename,
                                         bool SubUnit,
                                         int faction,
                                         std::string customizedUnit,
                                         Flightgroup *flightgroup,
                                         int fg_subnumber )
{
    return new Unit( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber );
}

Unit* UnitFactory::createUnit( vector< Mesh* > &meshes, bool Subunit, int faction )
{
    return new Unit( meshes,
                     Subunit,
                     faction );
}

Nebula* UnitFactory::createNebula( const char *unitfile,
                                   bool SubU,
                                   int faction,
                                   Flightgroup *fg,
                                   int fg_snumber,
                                   ObjSerial netcreate )
{
    _Universe->netLock( true );
    Nebula *neb = new Nebula( unitfile,
                              SubU,
                              faction,
                              fg,
                              fg_snumber );
    _Universe->netLock( false );
    if (netcreate)
        neb->SetSerial( netcreate );
/*
 *               if (!_Universe->netLocked()) {
 *                       NetBuffer netbuf;
 *                       addNebulaBuffer(netbuf, neb);
 *                       endBuffer(netbuf);
 *                       VSServer->broadcast( netbuf, 0, _Universe->activeStarSystem()->GetZone(), CMD_ENTERCLIENT, true);
 *               }
 *               VSServer->invalidateSnapshot();
 */
    return neb;
}

Missile* UnitFactory::createMissile( const char *filename,
                                     int faction,
                                     const string &modifications,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius,
                                     ObjSerial netcreate )
{
    _Universe->netLock( true );
    Missile *un = new Missile( filename,
                               faction,
                               modifications,
                               damage,
                               phasedamage,
                               time,
                               radialeffect,
                               radmult,
                               detonation_radius );
    _Universe->netLock( false );
    if (netcreate)
        un->SetSerial( netcreate );
/*
 *               if (!_Universe->netLocked()) {
 *                       NetBuffer netbuf;
 *                       addMissileBuffer( netbuf, un );
 *                       endBuffer( netbuf );
 *                       VSServer->broadcast( netbuf, 0, _Universe->activeStarSystem()->GetZone(), CMD_ENTERCLIENT, true);
 *               }
 *               VSServer->invalidateSnapshot();
 */
    return un;
}

Planet* UnitFactory::createPlanet()
{
    return new Planet;
}

Planet* UnitFactory::createPlanet( QVector x,
                                   QVector y,
                                   float vely,
                                   const Vector &rotvel,
                                   float pos,
                                   float gravity,
                                   float radius,
                                   const std::string &filename,
                                   const std::string &technique,
                                   const std::string &unitname,
                                   BLENDFUNC sr,
                                   BLENDFUNC ds,
                                   const vector< string > &dest,
                                   const QVector &orbitcent,
                                   Unit *parent,
                                   const GFXMaterial &ourmat,
                                   const std::vector< GFXLightLocal > &ligh,
                                   int faction,
                                   string fullname,
                                   bool inside_out,
                                   ObjSerial netcreate )
{
    _Universe->netLock( true );
    Planet *p = new Planet( x, y, vely, rotvel, pos, gravity, radius,
                           filename, technique, unitname, dest, orbitcent, parent, faction,
                           fullname, inside_out, ligh.size() );
    _Universe->netLock( false );
    if (netcreate)
        p->SetSerial( netcreate );
/*
 *               // False: Only allow creation through system files?  Doesn't make sense to be able to dynamically generate these.
 *               // Could cause inconsistencies with new clients that just read system files.
 *               if ( false && !_Universe->netLocked()) {
 *                       NetBuffer netbuf;
 *                       // Send a packet to clients in order to make them create this unit
 *
 *                       addPlanetBuffer( netbuf, x, y, vely, rotvel, pos, gravity, radius, filename, sr, ds, dest, orbitcent, parent, ourmat, ligh, faction, fullname, inside_out, netcreate);
 *                       endBuffer( netbuf );
 *                       VSServer->broadcast( netbuf, 0, _Universe->activeStarSystem()->GetZone(), CMD_ENTERCLIENT, true);
 *               }
 *               VSServer->invalidateSnapshot();
 */
    return p;
}

Enhancement* UnitFactory::createEnhancement( const char *filename,
                                             int faction,
                                             const string &modifications,
                                             Flightgroup *flightgrp,
                                             int fg_subnumber )
{
    return new Enhancement( filename, faction, modifications, flightgrp, fg_subnumber );
}

Building* UnitFactory::createBuilding( ContinuousTerrain *parent,
                                       bool vehicle,
                                       const char *filename,
                                       bool SubUnit,
                                       int faction,
                                       const std::string &unitModifications,
                                       Flightgroup *fg )
{
    return NULL;
}

Building* UnitFactory::createBuilding( Terrain *parent,
                                       bool vehicle,
                                       const char *filename,
                                       bool SubUnit,
                                       int faction,
                                       const std::string &unitModifications,
                                       Flightgroup *fg )
{
    return NULL;
}

Asteroid* UnitFactory::createAsteroid( const char *filename,
                                       int faction,
                                       Flightgroup *fg,
                                       int fg_snumber,
                                       float difficulty,
                                       ObjSerial netcreate )
{
    _Universe->netLock( true );
    Asteroid *ast = new Asteroid( filename, faction, fg, fg_snumber, difficulty );
    _Universe->netLock( false );
    if (netcreate)
        //Only allow creating through system files?  Doesn't make sense to be able to dynamically generate these.
        ast->SetSerial( netcreate );
/*
 *               if ( !_Universe->netLocked()) {
 *                       NetBuffer netbuf;
 *                       addAsteroidBuffer( netbuf, ast);
 *                       endBuffer(netbuf);
 *                       // NETFIXME: addBuffer for all subunits?
 *                       VSServer->broadcast( netbuf, 0, _Universe->activeStarSystem()->GetZone(), CMD_ENTERCLIENT, true);
 *               }
 *               VSServer->invalidateSnapshot();
 */
    return ast;
}

Terrain* UnitFactory::createTerrain( const char *file, Vector scale, float position, float radius, Matrix &t )
{
    return NULL;
}

ContinuousTerrain* UnitFactory::createContinuousTerrain( const char *file, Vector scale, float position, Matrix &t )
{
    return NULL;
}

void UnitFactory::broadcastUnit( Unit *unit, unsigned short zone )
{
    if ( !_Universe->netLocked() && unit->GetSerial() )
        if (SERVER) VSServer->broadcastUnit( unit, zone );
}

