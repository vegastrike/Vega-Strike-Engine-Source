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
                             int         fg_subnumber=0, string * netxml=NULL);

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
                                 int fg_snumber=0 );

    static Unit* createMissile( const char * filename,
                                   int faction,
                                   const string &modifications,
                                   const float damage,
                                   float phasedamage,
                                   float time,
                                   float radialeffect,
                                   float radmult,
                                   float detonation_radius );

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
				 vector<char *> dest,
				 const QVector &orbitcent,
				 Unit * parent,
				 const GFXMaterial & ourmat,
				 const std::vector <GFXLightLocal> & ligh,
				 int faction,
				 string fullname ,
				 bool inside_out=false);

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
				     float difficulty=.01 );

	static Terrain*	createTerrain( const char * file, Vector scale, float position, float radius, Matrix & t);
	static ContinuousTerrain*	createContinuousTerrain( const char * file, Vector scale, float position, Matrix & t);
};

#endif /* _UNIT_FACTORY_H_ */

