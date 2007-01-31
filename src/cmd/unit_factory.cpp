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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "unit_factory.h"
#include "unit.h"
#include "nebula.h"
#include "missile.h"
#include "enhancement.h"
#include "planet.h"
#include "asteroid.h"
#include "building.h"
#include "terrain.h"
#include "cont_terrain.h"
extern Unit * _masterPartList;
std::string getMasterPartListUnitName() {
	static std::string mpl = vs_config->getVariable("data","master_part_list","master_part_list");
	return mpl;
}


Unit* UnitFactory::createUnit( )
{
    return new GameUnit<Unit>( 0 );
}

Unit* UnitFactory::createUnit( const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber, string * netxml, ObjSerial netcreate)
{
    Unit * un = new GameUnit<Unit>( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber, netxml);
	if( netcreate)
		un->SetSerial( netcreate);
	return un;
}
Unit* UnitFactory::createServerSideUnit( const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber )
{
    return new Unit( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber );
}

Unit* UnitFactory::createUnit( vector <Mesh*> & meshes,
		               bool Subunit,
		               int faction )
{
    return new GameUnit<Unit>( meshes,
                     Subunit,
                     faction );
}

Nebula* UnitFactory::createNebula( const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber, ObjSerial netcreate )
{
    Nebula * neb = new GameNebula( unitfile,
                       SubU,
                       faction,
                       fg,
                       fg_snumber );
	if( netcreate)
		neb->SetSerial( netcreate);
	return neb;
}

Unit* UnitFactory::createMissile( const char * filename,
                                     int faction,
                                     const string &modifications,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius, ObjSerial netcreate )
{
  double startime= queryTime();
    Unit * un =new GameMissile( filename,
                        faction,
                        modifications,
                        damage,
                        phasedamage,
                        time,
                        radialeffect,
                        radmult,
                        detonation_radius );
	if( netcreate)
		un->SetSerial( netcreate);
	return un;
  double endtime= queryTime();
  printf ("mcr %f ",endtime-startime);
}

Planet* UnitFactory::createPlanet( )
{
    return new GamePlanet;
}

Planet* UnitFactory::createPlanet( QVector x,
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
				   bool inside_out, ObjSerial netcreate)
{
    Planet * p = new GamePlanet( x,
                       y,
                       vely,
                       rotvel,
                       pos,
                       gravity,
                       radius,
                       filename,
		       sr,ds,
                       dest,
                       orbitcent,
                       parent,
                       ourmat,
                       ligh,
                       faction,
                       fullname , 
		       inside_out);
	if( netcreate)
		p->SetSerial( netcreate);
	return p;
}

Enhancement* UnitFactory::createEnhancement( const char * filename,
                                             int faction,
					     const string &modifications,
					     Flightgroup * flightgrp,
					     int fg_subnumber )
{
    return new GameEnhancement( filename,
                            faction,
                            modifications,
                            flightgrp,
                            fg_subnumber );
}

Building* UnitFactory::createBuilding( ContinuousTerrain * parent,
                                       bool vehicle,
				       const char * filename,
				       bool SubUnit,
				       int faction,
				       const std::string &unitModifications,
				       Flightgroup * fg )
{
    return new GameBuilding( parent,
                         vehicle,
			 filename,
			 SubUnit,
			 faction,
			 unitModifications,
			 fg );
}

Building* UnitFactory::createBuilding( Terrain * parent,
                                       bool vehicle,
                                       const char *filename,
                                       bool SubUnit,
                                       int faction,
                                       const std::string &unitModifications,
                                       Flightgroup * fg )
{
    return new GameBuilding( parent,
                         vehicle,
                         filename,
                         SubUnit,
                         faction,
                         unitModifications,
                         fg );
}

Asteroid* UnitFactory::createAsteroid( const char * filename,
                                       int faction,
                                       Flightgroup* fg,
                                       int fg_snumber,
                                       float difficulty, ObjSerial netcreate )
{
    Asteroid * ast = new GameAsteroid( filename,
                         faction,
                         fg,
                         fg_snumber,
                         difficulty );
	if( netcreate)
		ast->SetSerial( netcreate);
	return ast;
}

Terrain*	UnitFactory::createTerrain( const char * file, Vector scale, float position, float radius, Matrix & t)
{
	  Terrain * tt;
	  tt = new Terrain (file,scale,position,radius);
	  tt->SetTransformation( t);
	  return tt;
}

ContinuousTerrain*	UnitFactory::createContinuousTerrain( const char * file, Vector scale, float position, Matrix & t)
{
	  ContinuousTerrain * ct;
	  ct = new ContinuousTerrain (file,scale,position);
	  ct->SetTransformation (t);
	  return ct;
}

