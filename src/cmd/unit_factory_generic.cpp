#include "unit_factory.h"
#include "unit_generic.h"
#include "universe_util.h"
#include "planet_generic.h"
#include "asteroid_generic.h"
#include "missile_generic.h"
#include "nebula_generic.h"
#include "networking/zonemgr.h"
#include "networking/lowlevel/netbuffer.h"

Unit* UnitFactory::_masterPartList = NULL;
Unit* UnitFactory::getMasterPartList( )
{

    if( _masterPartList == NULL )
    {
      static bool making=true;
      if (making) {
        making=false;
        _masterPartList = Unit::makeMasterPartList();
        making=true;
      }        
    }
    return _masterPartList;
}

void UnitFactory::addUnitBuffer( NetBuffer & netbuf, const string &filename,
		               const string &name,
		               const string &fullname,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               const Transformation &curr_physical_state,
		               Flightgroup *flightgroup,
		               int         fg_subnumber, string * netxml, ObjSerial netcreate)
{
		netbuf.addChar(ZoneMgr::AddUnit);
		netbuf.addSerial( netcreate);
		netbuf.addString( filename);
		netbuf.addString( name);
		netbuf.addString( fullname);
		netbuf.addChar( SubUnit);
		netbuf.addInt32( faction);
		netbuf.addString( customizedUnit);
		netbuf.addString( flightgroup!=NULL?flightgroup->name:std::string("Object"));
		netbuf.addInt32( fg_subnumber);
		netbuf.addTransformation( curr_physical_state);
}

void UnitFactory::addUnitBuffer( NetBuffer & netbuf, const Unit *un, string *netxml)
{
	addUnitBuffer( netbuf, un->getFilename(), un->name.get(), un->fullname, un->isSubUnit(), un->faction,
		"" /* Not sure... maybe netxml will take care of this? */, un->curr_physical_state,
	   un->getFlightgroup(), un->getFgSubnumber(), netxml /*For ENTERCLIENT, will generate a saved game netxml*/, un->GetSerial());
}
								

Unit *UnitFactory::parseUnitBuffer(NetBuffer &netbuf)
{
	ObjSerial serial = netbuf.getSerial();
	string file( netbuf.getString());
	string name( netbuf.getString());
	string fullname( netbuf.getString());
	bool sub = netbuf.getChar();
	int faction = netbuf.getInt32();
	string fname( netbuf.getString());
	string custom( netbuf.getString());
	int fg_num = netbuf.getInt32();

	cerr<<"NETCREATE UNIT : "<<file<<endl;
	
	string facname = FactionUtil::GetFactionName( faction);
	Flightgroup * fg = mission[0].findFlightgroup( fname, facname);
	
	Unit *un = createUnit( file.c_str(), sub, faction, custom, fg, fg_num, NULL, serial);
	un->curr_physical_state = netbuf.getTransformation();
	un->name = name;
	un->fullname = fullname;
	return un;
}

void UnitFactory::addPlanetBuffer( NetBuffer & netbuf, QVector x,
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
		netbuf.addChar(ZoneMgr::AddPlanet);
		netbuf.addSerial( netcreate);
		netbuf.addQVector( x);
		netbuf.addQVector( y);
		netbuf.addFloat( vely);
		netbuf.addVector( Vector( rotvel));
		netbuf.addFloat( pos);
		netbuf.addFloat( gravity);
		netbuf.addFloat( radius);

		netbuf.addString( string( filename));
		netbuf.addChar( sr);
		netbuf.addChar( ds);

		netbuf.addShort( dest.size());
		for( unsigned int i=0; i<dest.size(); i++)
			netbuf.addString( dest[i] );
		netbuf.addQVector( QVector( orbitcent));
		netbuf.addSerial( parent->GetSerial());
		netbuf.addGFXMaterial( ourmat);
		netbuf.addShort( ligh.size());
		for( unsigned int j=0; j<ligh.size(); j++)
			netbuf.addGFXLightLocal( ligh[j]);

		netbuf.addInt32( faction);
		netbuf.addString( fullname);
		netbuf.addChar( inside_out);
}

Planet *UnitFactory::parsePlanetBuffer(NetBuffer &netbuf)
{
	ObjSerial serial = netbuf.getSerial();
	QVector x = netbuf.getQVector();
	QVector y = netbuf.getQVector();
	float vely = netbuf.getFloat();
	const Vector rotvel( netbuf.getVector());
	float pos = netbuf.getFloat();
	float gravity = netbuf.getFloat();
	float radius = netbuf.getFloat();

	string file( netbuf.getString());
	char sr = netbuf.getChar();
	char ds = netbuf.getChar();

	vector<string> dest;
	unsigned short nbdest = netbuf.getShort();
	int i=0;
	for( i=0; i<nbdest; i++)
	{
		string tmp( netbuf.getString());
		char * ctmp = new char[tmp.length()+1];
		ctmp[tmp.length()] = 0;
		memcpy( ctmp, tmp.c_str(), tmp.length());
		dest.push_back( ctmp);
	}

	const QVector orbitcent( netbuf.getQVector());
	Unit *un = UniverseUtil::GetUnitFromSerial( netbuf.getSerial());
	GFXMaterial mat = netbuf.getGFXMaterial();
				
	vector<GFXLightLocal> lights;
	unsigned short nblight = netbuf.getShort();
	for( i=0; i<nblight; i++)
		lights.push_back( netbuf.getGFXLightLocal());

	int faction = netbuf.getInt32();
	string fullname( netbuf.getString());
	char insideout = netbuf.getChar();

	cerr<<"NETCREATE PLANET : "<<file<<endl;

	return UnitFactory::createPlanet( x, y, vely, rotvel, pos, gravity, radius, file.c_str(),
		(BLENDFUNC)sr, (BLENDFUNC)ds, dest, orbitcent, un, mat, lights, faction, fullname, insideout, serial);
}

void UnitFactory::addNebulaBuffer( NetBuffer & netbuf, const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber, ObjSerial netcreate )
{
		netbuf.addChar(ZoneMgr::AddNebula);
		netbuf.addSerial( netcreate);
		netbuf.addString( string( unitfile));
		netbuf.addChar( SubU);
		netbuf.addInt32( faction);
		netbuf.addString( fg->name);
		netbuf.addInt32( fg_snumber);
}

void UnitFactory::addNebulaBuffer( NetBuffer & netbuf, const Nebula *neb) {
	addNebulaBuffer( netbuf, neb->getFilename().c_str(), neb->isSubUnit(), neb->faction, neb->getFlightgroup(), neb->getFgSubnumber(), neb->GetSerial());
}

Nebula *UnitFactory::parseNebulaBuffer(NetBuffer &netbuf)
{
	ObjSerial serial = netbuf.getSerial();
	string file( netbuf.getString());
	bool sub = netbuf.getChar();
	int faction = netbuf.getInt32();
	string fname( netbuf.getString());
	int fg_num = netbuf.getInt32();

	cerr<<"NETCREATE NEBULA : "<<file<<endl;

	string facname = FactionUtil::GetFactionName( faction);
	Flightgroup * fg = mission[0].findFlightgroup( fname, facname);
	return UnitFactory::createNebula( file.c_str(), sub, faction, fg, fg_num, serial);
}

void UnitFactory::addMissileBuffer( NetBuffer & netbuf, const string &filename,
		               const string &name,
		               const string &fullname,
                                     int faction,
                                     const string &modifications,
		                             const Transformation &curr_physical_state,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius, ObjSerial netcreate )
{
		netbuf.addChar(ZoneMgr::AddMissile);
		netbuf.addSerial( netcreate);
		netbuf.addString( filename);
		netbuf.addString( name);
		netbuf.addString( fullname);
		
		netbuf.addInt32( faction);
		netbuf.addString( modifications);
		netbuf.addFloat( damage);
		netbuf.addFloat( phasedamage);
		netbuf.addFloat( time);
		netbuf.addFloat( radialeffect);
		netbuf.addFloat( radmult);
		netbuf.addFloat( detonation_radius);
		netbuf.addTransformation( curr_physical_state);
}

void UnitFactory::addMissileBuffer( NetBuffer & netbuf, const Missile *mis) {
	addMissileBuffer( netbuf, mis->getFilename().c_str(), mis->name, mis->getFullname(), mis->faction,
		"" /* modifications */, mis->curr_physical_state, mis->damage, mis->phasedamage, mis->time,
		mis->radial_effect, mis->radial_multiplier, mis->detonation_radius, mis->GetSerial());
}


Missile *UnitFactory::parseMissileBuffer(NetBuffer &netbuf)
{
	ObjSerial serial = netbuf.getSerial();
	string file( netbuf.getString());
	string name( netbuf.getString());
	string fullname( netbuf.getString());
	int faction = netbuf.getInt32();
	string mods( netbuf.getString());
	const float damage( netbuf.getFloat());
	float phasedamage = netbuf.getFloat();
	float time = netbuf.getFloat();
	float radialeffect = netbuf.getFloat();
	float radmult = netbuf.getFloat();
	float detonation_radius = netbuf.getFloat();
	const string modifs( mods);

	cerr<<"NETCREATE MISSILE : "<<file<<endl;

	Missile *mis = createMissile( file.c_str(), faction, modifs, damage, phasedamage, time, radialeffect, radmult, detonation_radius, serial);
	mis->curr_physical_state = netbuf.getTransformation();
	mis->name = name;
	mis->fullname = fullname;
	return mis;
}

void UnitFactory::addAsteroidBuffer( NetBuffer & netbuf, const char * filename,
                                       int faction,
                                       Flightgroup* fg,
                                       int fg_snumber,
                                       float difficulty, ObjSerial netcreate )
{
		netbuf.addChar(ZoneMgr::AddAsteroid);
		netbuf.addSerial( netcreate);
		netbuf.addString( string( filename));
		netbuf.addInt32( faction);
		netbuf.addString( fg->name);
		netbuf.addInt32( fg_snumber);
		netbuf.addFloat( difficulty);
}

void UnitFactory::addAsteroidBuffer( NetBuffer &netbuf, const Asteroid *aster )
{
	addAsteroidBuffer( netbuf, aster->getFilename().c_str(), aster->faction, aster->getFlightgroup(), aster->getFgSubnumber(),
					   0/*difficulty--only determines angular velocity*/, aster->GetSerial());
}

Asteroid *UnitFactory::parseAsteroidBuffer(NetBuffer &netbuf)
{
	ObjSerial serial = netbuf.getSerial();
	string file( netbuf.getString());
	int faction = netbuf.getInt32();
	string fname( netbuf.getString());
	int fg_snumber = netbuf.getInt32();
	float diff = netbuf.getFloat();

	cerr<<"NETCREATE ASTEROID : "<<file<<endl;

	string facname = FactionUtil::GetFactionName( faction);
	Flightgroup * fg = mission[0].findFlightgroup( fname, facname);
	
	return UnitFactory::createAsteroid( file.c_str(), faction, fg, fg_snumber, diff, serial);
}

void UnitFactory::addBuffer(NetBuffer &netbuf, const Unit *un, bool allowSystemTypes, string *netxml)
{
	switch (un->isUnit()) {
	case UNITPTR:
		addUnitBuffer(netbuf, un, netxml);
		break;
	case MISSILEPTR:
		addMissileBuffer(netbuf, static_cast<const Missile*> (un));
		break;
	/*
	case PLANETPTR:
		if (allowSystemTypes) addPlanetBuffer(netbuf, static_cast<Planet*> (un));
		break;
	*/
	case NEBULAPTR:
		if (allowSystemTypes) addNebulaBuffer(netbuf, static_cast<const Nebula*> (un));
		break;
	case ASTEROIDPTR:
		if (allowSystemTypes) addAsteroidBuffer(netbuf, static_cast<const Asteroid*> (un));
		break;
	default:
		break;
	}
}

void UnitFactory::endBuffer( NetBuffer &netbuf) {
	netbuf.addChar(ZoneMgr::End);
}
