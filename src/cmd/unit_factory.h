/*
 * Copyright (C) 2002 Carsten Griwodz
 *
 * http://vegastrike.sourceforge.net/
 *
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
#ifndef _UNIT_FACTORY_H_
#define _UNIT_FACTORY_H_
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string>

#include "cmd/planet_generic.h"
#include "networking/lowlevel/netbuffer.h"

//class GameUnit;
class Mesh;
class Flightgroup;
class Nebula;
class Missile;
class Enhancement;
class Building;
class Asteroid;
class Terrain;
class ContinuousTerrain;

class UnitFactory
{
protected:
    static Unit* _masterPartList;
public:
    static Unit* getMasterPartList( );

	static Unit* createUnit( );

    static Unit* createUnit( const char *filename,
                             bool        SubUnit,
                             int         faction,
                             std::string customizedUnit=string(""),
                             Flightgroup *flightgroup=NULL,
                             int         fg_subnumber=0, string * netxml=NULL, ObjSerial netcreate=0);


    static Unit* createServerSideUnit( const char *filename,
                             bool        SubUnit,
                             int         faction,
                             std::string customizedUnit=string(""),
                             Flightgroup *flightgroup=NULL,
                             int         fg_subnumber=0 );

    static Unit* createUnit( std::vector<Mesh*> &meshes,
			     bool Subunit,
			     int faction);

    static Nebula* createNebula( const char * unitfile, 
                                 bool SubU, 
                                 int faction, 
                                 Flightgroup* fg=NULL, 
                                 int fg_snumber=0, ObjSerial netcreate=0 );

    static Missile* createMissile( const char * filename,
                                   int faction,
                                   const string &modifications,
                                   const float damage,
                                   float phasedamage,
                                   float time,
                                   float radialeffect,
                                   float radmult,
                                   float detonation_radius, ObjSerial netcreate=0 );

    static Planet* createPlanet( );

    static Planet* createPlanet( QVector x,
                                 QVector y,
				 float vely,
				 const Vector & rotvel,
				 float pos,
				 float gravity,
				 float radius,
				 const char * filename,
				 BLENDFUNC blendsrc,
				 BLENDFUNC blenddst,
				 const vector<string> &dest,
				 const QVector &orbitcent,
				 Unit * parent,
				 const GFXMaterial & ourmat,
				 const std::vector <GFXLightLocal> & ligh,
				 int faction,
				 string fullname ,
				 bool inside_out=false, ObjSerial netcreate=0);

    static Enhancement* createEnhancement( const char * filename,
                                           int faction,
					   const string &modifications,
					   Flightgroup * flightgrp=NULL,
					   int fg_subnumber=0 );

    static Building* createBuilding( ContinuousTerrain * parent,
                                     bool vehicle,
				     const char * filename,
				     bool SubUnit,
				     int faction,
				     const std::string &unitModifications=std::string(""),
				     Flightgroup * fg=NULL );

    static Building* createBuilding( Terrain * parent,
                                     bool vehicle,
				     const char *filename,
				     bool SubUnit,
				     int faction,
				     const std::string &unitModifications=std::string(""),
				     Flightgroup * fg=NULL );

    static Asteroid* createAsteroid( const char * filename,
                                     int faction,
				     Flightgroup* fg=NULL,
				     int fg_snumber=0,
				     float difficulty=.01, ObjSerial netcreate=0 );

	static Terrain*	createTerrain( const char * file, Vector scale, float position, float radius, Matrix & t);
	static ContinuousTerrain*	createContinuousTerrain( const char * file, Vector scale, float position, Matrix & t);

	// Function used to put in a NetBuffer the necessary info to create the unit so that it can be send over network
	static void addUnitBuffer( NetBuffer & netbuf, const string &filename,
		               const string &name, const string &fullname,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               const Transformation &curr_physical_state,
		               Flightgroup *flightgroup,
		               int         fg_subnumber, string * netxml, ObjSerial netcreate);
	static void addUnitBuffer( NetBuffer & netbuf, const Unit *un, string *netxml=NULL);
	static Unit *parseUnitBuffer(NetBuffer &netbuf);
	
	static void addPlanetBuffer( NetBuffer & netbuf, QVector x,
                                   QVector y,
				   float vely,
				   const Vector & rotvel,
				   float pos,
				   float gravity,
				   float radius,
				   const char * filename,
				   BLENDFUNC sr, BLENDFUNC ds,
				   const vector<string> &dest,
				   const QVector &orbitcent,
				   Unit * parent,
				   const GFXMaterial & ourmat,
				   const std::vector <GFXLightLocal> & ligh,
				   int faction,
				   string fullname ,
				   bool inside_out, ObjSerial netcreate);
	// Not implemented: lots of orbit stuff is dissipated into private subclasses upon creation.
	// static void addPlanetBuffer( NetBuffer & netbuf, const Planet *p);
	static Planet *parsePlanetBuffer(NetBuffer &netbuf);
	
	static void addNebulaBuffer( NetBuffer & netbuf, const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber, ObjSerial netcreate );
	static void addNebulaBuffer( NetBuffer & netbuf, const Nebula *neb);
	static Nebula *parseNebulaBuffer(NetBuffer &netbuf);

	static void addMissileBuffer( NetBuffer & netbuf, const char * filename,
                                     int faction,
                                     const string &modifications,
                                     const Transformation &curr_physical_state,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius, ObjSerial netcreate );
	static void addMissileBuffer( NetBuffer & netbuf, const Missile *mis);
	static Missile *parseMissileBuffer(NetBuffer &netbuf);

	static void addAsteroidBuffer( NetBuffer & netbuf, const char * filename,
                                       int faction,
                                       Flightgroup* fg,
                                       int fg_snumber,
                                       float difficulty, ObjSerial netcreate );
	static void addAsteroidBuffer( NetBuffer & netbuf, const Asteroid *ast);
	static Asteroid *parseAsteroidBuffer(NetBuffer &netbuf);
	
	static void addBuffer(NetBuffer &netbuf, const Unit *un, bool allowSystemTypes, string *netxml=NULL);
	
	static void endBuffer(NetBuffer &netbuf); // Tells the client that we are done sending units... is this necessary?

	static void broadcastUnit(const Unit *un, unsigned short zone);
};

#endif /* _UNIT_FACTORY_H_ */

