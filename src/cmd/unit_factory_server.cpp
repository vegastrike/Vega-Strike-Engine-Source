#include "unit_factory.h"
#include "unit_generic.h"
#include "gfx/cockpit_generic.h"
#include "nebula_generic.h"
#include "planet_generic.h"
#include "asteroid_generic.h"
#include "missile_generic.h"
#include "enhancement_generic.h"
#if defined( _WIN32) && !defined( __CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "networking/lowlevel/netbuffer.h"
#include "networking/netserver.h"

extern Unit * _masterPartList;
std::string getMasterPartListUnitName() {
	static std::string mpl = vs_config->getVariable("data","master_part_list","master_part_list");
	return mpl;
}

Unit* UnitFactory::getMasterPartList( )
{ 
	
char tmp[2048];
tmp[2047]=0;
getcwd (tmp,2046);

    if( _masterPartList == NULL )
    {
		std::string mpl=getMasterPartListUnitName();
        _masterPartList = new Unit(mpl.c_str(),
	                            true,
				    FactionUtil::GetFaction("upgrades") );
    }
    return _masterPartList;
}

Unit* UnitFactory::createUnit( )
{
    return new Unit( 0 );
}

Unit* UnitFactory::createUnit( const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber, string * netxml, ObjSerial netcreate)
{
	Unit * un = new Unit( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber, netxml);
	if( netcreate)
	{
		// Send a packet to clients in order to make them create this unit
		NetBuffer netbuf;
		getUnitBuffer( netbuf, filename, SubUnit, faction, customizedUnit, flightgroup, fg_subnumber, netxml, netcreate);
		// Broadcast to the current universe star system
		Server->broadcast( netbuf, _Universe->activeStarSystem()->GetZone(), CMD_CREATEUNIT);

		un->SetSerial( netcreate);
	}
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
    return new Unit( meshes,
                     Subunit,
                     faction );
}

Nebula* UnitFactory::createNebula( const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber, ObjSerial netcreate )
{
    Nebula * neb = new Nebula( unitfile,
        SubU,
	    faction,
	    fg,
	    fg_snumber);
	if( netcreate)
	{
		// Send a packet to clients in order to make them create this unit
		NetBuffer netbuf;
		getNebulaBuffer( netbuf, unitfile, SubU, faction, fg, fg_snumber, netcreate);
		Server->broadcast( netbuf, _Universe->activeStarSystem()->GetZone(), CMD_CREATENEBULA);

		neb->SetSerial( netcreate);
	}
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
    Unit * un = new Missile( filename,
         faction,
	     modifications,
	     damage,
	     phasedamage,
	     time,
	     radialeffect,
	     radmult,
	     detonation_radius);
	if( netcreate)
	{
		NetBuffer netbuf;
		getMissileBuffer( netbuf, filename, faction, modifications, damage, phasedamage, time, radialeffect, radmult, detonation_radius, netcreate);
		Server->broadcast( netbuf, _Universe->activeStarSystem()->GetZone(), CMD_CREATEMISSILE);

		un->SetSerial( netcreate);
	}
	return un;
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
				   const char * filename,
				   BLENDFUNC sr, BLENDFUNC ds,
				   vector<char *> dest,
				   const QVector &orbitcent,
				   Unit * parent,
				   const GFXMaterial & ourmat,
				   const std::vector <GFXLightLocal> & ligh,
				   int faction,
				   string fullname ,
				   bool inside_out, ObjSerial netcreate)
{
    Planet * p = new Planet( x, y, vely, rotvel, pos, gravity, radius,
		               filename, dest, orbitcent, parent, faction,
					   fullname, inside_out, 0);
	if( netcreate)
	{
		// Send a packet to clients in order to make them create this unit
		NetBuffer netbuf;
		getPlanetBuffer( netbuf, x, y, vely, rotvel, pos, gravity, radius, filename, sr, ds, dest, orbitcent, parent, ourmat, ligh, faction, fullname, inside_out, netcreate);
		Server->broadcast( netbuf, _Universe->activeStarSystem()->GetZone(), CMD_CREATEPLANET);

		p->SetSerial( netcreate);
	}
	return p;
}

Enhancement* UnitFactory::createEnhancement( const char * filename,
                                             int faction,
					     const string &modifications,
					     Flightgroup * flightgrp,
					     int fg_subnumber )
{
	return new Enhancement(filename, faction, modifications,flightgrp,fg_subnumber);
}

Building* UnitFactory::createBuilding( ContinuousTerrain * parent,
                                       bool vehicle,
				       const char * filename,
				       bool SubUnit,
				       int faction,
				       const std::string &unitModifications,
				       Flightgroup * fg )
{
	return NULL;
}

Building* UnitFactory::createBuilding( Terrain * parent,
                                       bool vehicle,
                                       const char *filename,
                                       bool SubUnit,
                                       int faction,
                                       const std::string &unitModifications,
                                       Flightgroup * fg )
{
	return NULL;
}

Asteroid* UnitFactory::createAsteroid( const char * filename,
                                       int faction,
                                       Flightgroup* fg,
                                       int fg_snumber,
                                       float difficulty, ObjSerial netcreate )
{
    Asteroid * ast = new Asteroid( filename, faction, fg, fg_snumber, difficulty);
	if( netcreate)
	{
		// Send a packet to clients in order to make them create this unit
		NetBuffer netbuf;
		getAsteroidBuffer( netbuf, filename, faction, fg, fg_snumber, difficulty, netcreate);
		Server->broadcast( netbuf, _Universe->activeStarSystem()->GetZone(), CMD_CREATEASTER);

		ast->SetSerial( netcreate);
	}
	return ast;
}

Terrain*	UnitFactory::createTerrain( const char * file, Vector scale, float position, float radius, Matrix & t)
{
	  return NULL;
}

ContinuousTerrain*	UnitFactory::createContinuousTerrain( const char * file, Vector scale, float position, Matrix & t)
{
	  return NULL;
}

