#include "unit_factory.h"
#include "unit_generic.h"
#include "networking/lowlevel/netbuffer.h"

Unit* UnitFactory::_masterPartList = NULL;

void UnitFactory::getUnitBuffer( NetBuffer & netbuf, const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber, string * netxml, ObjSerial netcreate)
{
		netbuf.addSerial( netcreate);
		netbuf.addString( string( filename));
		netbuf.addChar( SubUnit);
		netbuf.addInt32( faction);
		netbuf.addString( customizedUnit);
		netbuf.addString( flightgroup!=NULL?flightgroup->name:std::string("Object"));
		netbuf.addInt32( fg_subnumber);
}
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

void UnitFactory::getPlanetBuffer( NetBuffer & netbuf, QVector x,
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
			netbuf.addString( string(dest[i]));
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

void UnitFactory::getNebulaBuffer( NetBuffer & netbuf, const char * unitfile, 
                                   bool SubU, 
                                   int faction, 
                                   Flightgroup* fg,
                                   int fg_snumber, ObjSerial netcreate )
{
		netbuf.addSerial( netcreate);
		netbuf.addString( string( unitfile));
		netbuf.addChar( SubU);
		netbuf.addInt32( faction);
		netbuf.addString( fg->name);
		netbuf.addInt32( fg_snumber);
}

void UnitFactory::getMissileBuffer( NetBuffer & netbuf, const char * filename,
                                     int faction,
                                     const string &modifications,
                                     const float damage,
                                     float phasedamage,
                                     float time,
                                     float radialeffect,
                                     float radmult,
                                     float detonation_radius, ObjSerial netcreate )
{
		netbuf.addSerial( netcreate);
		netbuf.addString( string (filename));
		netbuf.addInt32( faction);
		netbuf.addString( string( modifications));
		netbuf.addFloat( float(damage));
		netbuf.addFloat( phasedamage);
		netbuf.addFloat( time);
		netbuf.addFloat( radialeffect);
		netbuf.addFloat( radmult);
		netbuf.addFloat( detonation_radius);
}

void UnitFactory::getAsteroidBuffer( NetBuffer & netbuf, const char * filename,
                                       int faction,
                                       Flightgroup* fg,
                                       int fg_snumber,
                                       float difficulty, ObjSerial netcreate )
{
		netbuf.addSerial( netcreate);
		netbuf.addString( string( filename));
		netbuf.addInt32( faction);
		netbuf.addString( fg->name);
		netbuf.addInt32( fg_snumber);
		netbuf.addFloat( difficulty);
}

