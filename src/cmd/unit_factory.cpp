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

Unit* UnitFactory::createUnit( )
{
    return new Unit( 0 );
}

Unit* UnitFactory::createUnit( Mesh** meshes,
		               int num,
		               bool Subunit,
		               int faction )
{
    return new Unit( meshes,
                     num,
                     Subunit,
                     faction );
}

Unit* UnitFactory::createUnit( const char *filename,
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

Nebula* UnitFactory::createNebula( const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber )
{
    return new Nebula( unitfile,
                       SubU,
                       faction,
                       fg,
                       fg_snumber );
}

Missile* UnitFactory::createMissile( const char * filename,
                                     int faction,
                                     const string &modifications,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius )
{
    return new Missile( filename,
                        faction,
                        modifications,
                        damage,
                        phasedamage,
                        time,
                        radialeffect,
                        radmult,
                        detonation_radius );
}

Planet* UnitFactory::createPlanet( )
{
    return new Planet;
}

Planet* UnitFactory::createPlanet( QVector x,
                                   QVector y,
				   float vely,
				   const Vector & rotvel,
				   float pos,
				   float gravity,
				   float radius,
				   char * filename,
				   char *alpha,
				   vector<char *> dest,
				   const QVector &orbitcent,
				   Unit * parent,
				   const GFXMaterial & ourmat,
				   const std::vector <GFXLightLocal> & ligh,
				   int faction,
				   string fullname )
{
    return new Planet( x,
                       y,
                       vely,
                       rotvel,
                       pos,
                       gravity,
                       radius,
                       filename,
                       alpha,
                       dest,
                       orbitcent,
                       parent,
                       ourmat,
                       ligh,
                       faction,
                       fullname );
}

Enhancement* UnitFactory::createEnhancement( const char * filename,
                                             int faction,
					     const string &modifications,
					     Flightgroup * flightgrp,
					     int fg_subnumber )
{
    return new Enhancement( filename,
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
    return new Building( parent,
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
    return new Building( parent,
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
                                       float difficulty )
{
    return new Asteroid( filename,
                         faction,
                         fg,
                         fg_snumber,
                         difficulty );
}

Unit* UnitFactory::getMasterPartList( )
{
    if( _masterPartList == NULL )
    {
        _masterPartList = new Unit( "master_part_list",
	                            true,
				    _Universe->GetFaction("upgrades") );
    }
    return _masterPartList;
}

Unit* UnitFactory::_masterPartList = NULL;

