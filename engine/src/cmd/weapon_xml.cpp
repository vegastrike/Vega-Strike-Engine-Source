/**
 * weapon_xml.cpp
 *
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors.
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>

#include "star_system.h"
#include "weapon_xml.h"
#include "audiolib.h"
#include "unit_generic.h"
#include "beam.h"
#include "unit_const_cache.h"
#include "vsfilesystem.h"
#include "role_bitmask.h"
#include "endianness.h"
#include "options.h"





#if (defined (__APPLE__) == POSH_BIG_ENDIAN) || !defined (INTEL_X86)
//pre-optimization bug with "gcc 3.1 (20021003) prerelease"
int counts = time( NULL );
#else
int counts = 0;
#endif

void setWeaponInfoToBuffer( weapon_info wi, char *netbuf, int &bufsize )
{
    bufsize = sizeof (wi)+sizeof (wi.file)+sizeof (wi.name);
    netbuf  = new char[bufsize+1];
    netbuf[bufsize] = 0;
    int offset = 0;

    unsigned short file_len = wi.file.length();
    unsigned short weap_len = wi.name.length();
    char *file = new char[file_len+1];
    char *weapon_name = new char[weap_len+1];
    memcpy( file, wi.file.c_str(), file_len );
    file[file_len] = 0;
    memcpy( weapon_name, wi.name.c_str(), weap_len );
    weapon_name[weap_len] = 0;

    //Copy the struct weapon_info in the buffer
    memcpy( netbuf+offset, &wi, sizeof (wi) );
    offset += sizeof (wi);
    //Copy the size of filename in the buffer
    memcpy( netbuf+offset, &file_len, sizeof (file_len) );
    offset += sizeof (file_len);
    //Copy the filename in the buffer because in weapon_info, it is a string
    memcpy( netbuf+offset, file, file_len );
    offset += file_len;
    //Copy the size of filename in the buffer
    memcpy( netbuf+offset, &weap_len, sizeof (weap_len) );
    offset += sizeof (weap_len);
    //Copy the weapon_name in the buffer because in weapon_info, it is a string
    memcpy( netbuf+offset, weapon_name, weap_len );

    delete[] file;
    delete[] weapon_name;
}



#include "xml_support.h"
#include "physics.h"
#include <vector>

#include <expat.h>

using std::vector;
using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;


enum Names
{
    UNKNOWN,
    WEAPONS,
    BEAM,
    BALL,
    BOLT,
    PROJECTILE,
    APPEARANCE,
    ENERGY,
    DAMAGE,
    DISTANCE,
    NAME,
    SOUNDMP3,
    SOUNDWAV,
    WEAPSIZE,
    XFILE,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    SPEED,
    OFFSETX, OFFSETY, OFFSETZ,
    PULSESPEED,
    RADIALSPEED,
    RANGE,
    RADIUS,
    RATE,
    STABILITY,
    LONGRANGE,
    CONSUMPTION,
    REFIRE,
    LENGTH,
    PHASEDAMAGE,
    VOLUME,
    DETONATIONRADIUS,
    LOCKTIME,
    ROLE, ANTIROLE,
    TEXTURESTRETCH

};



const EnumMap::Pair element_names[] = {
    EnumMap::Pair( "UNKNOWN",    UNKNOWN ),   //don't add anything until below missile so it maps to enum WEAPON_TYPE
    EnumMap::Pair( "Beam",       BEAM ),
    EnumMap::Pair( "Ball",       BALL ),
    EnumMap::Pair( "Bolt",       BOLT ),
    EnumMap::Pair( "Missile",    PROJECTILE ),
    EnumMap::Pair( "Weapons",    WEAPONS ),
    EnumMap::Pair( "Appearance", APPEARANCE ),
    EnumMap::Pair( "Energy",     ENERGY ),
    EnumMap::Pair( "Damage",     DAMAGE ),
    EnumMap::Pair( "Distance",   DISTANCE )
};

const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair( "UNKNOWN",         UNKNOWN ),
    EnumMap::Pair( "Name",            NAME ),
    EnumMap::Pair( "MountSize",       WEAPSIZE ),
    EnumMap::Pair( "file",            XFILE ),
    EnumMap::Pair( "soundMp3",        SOUNDMP3 ),
    EnumMap::Pair( "soundWav",        SOUNDWAV ),
    EnumMap::Pair( "r",               RED ),
    EnumMap::Pair( "g",               GREEN ),
    EnumMap::Pair( "b",               BLUE ),
    EnumMap::Pair( "a",               ALPHA ),
    EnumMap::Pair( "Speed",           SPEED ),
    EnumMap::Pair( "Pulsespeed",      PULSESPEED ),
    EnumMap::Pair( "DetonationRange", DETONATIONRADIUS ),
    EnumMap::Pair( "LockTime",        LOCKTIME ),
    EnumMap::Pair( "Radialspeed",     RADIALSPEED ),
    EnumMap::Pair( "Range",           RANGE ),
    EnumMap::Pair( "Radius",          RADIUS ),
    EnumMap::Pair( "Rate",            RATE ),
    EnumMap::Pair( "Damage",          DAMAGE ),
    EnumMap::Pair( "PhaseDamage",     PHASEDAMAGE ),
    EnumMap::Pair( "Stability",       STABILITY ),
    EnumMap::Pair( "Longrange",       LONGRANGE ),
    EnumMap::Pair( "Consumption",     CONSUMPTION ),
    EnumMap::Pair( "Refire",          REFIRE ),
    EnumMap::Pair( "Length",          LENGTH ),
    EnumMap::Pair( "OffsetX",         OFFSETX ),
    EnumMap::Pair( "OffsetY",         OFFSETY ),
    EnumMap::Pair( "OffsetZ",         OFFSETZ ),
    EnumMap::Pair( "Volume",          VOLUME ),
    EnumMap::Pair( "Role",            ROLE ),
    EnumMap::Pair( "AntiRole",        ANTIROLE ),
    EnumMap::Pair( "TextureStretch",  TEXTURESTRETCH )
};

const EnumMap element_map( element_names, 10 );
const EnumMap attribute_map( attribute_names, 32 );
Hashtable< string, weapon_info, 257 >lookuptable;
string curname;
weapon_info tmpweapon( WEAPON_TYPE::BEAM );
int    level = -1;

void beginElementXML_Char( void *userData, const XML_Char *name, const XML_Char **atts )
{
    beginElement( userData, (const XML_Char*) name, (const XML_Char**) atts );
}

#define color_step (49)

#define Gamma_Needed( gamma, count, depth )           \
    (                                                 \
        !(                                            \
            ( count/(100*depth*gamma) )               \
            %( (6*(color_step*100+depth)/gamma-1)/3 ) \
            -100                                      \
         )                                            \
    )

void beginElement( void *userData, const char *name, const char **atts )
{

    static float  gun_speed     = game_options.gun_speed * (game_options.gun_speed_adjusted_game_speed ? game_options.game_speed : 1);
    static int    gamma = (int) ( 20*game_options.weapon_gamma );
    AttributeList attributes( atts );
    WEAPON_TYPE weaptyp;
    Names elem = (Names) element_map.lookup( string( name ) );
#ifdef TESTBEAMSONLY
    if (elem == BOLT)
        elem = BEAM;
#endif
    AttributeList::const_iterator iter;
    switch (elem)
    {
    case UNKNOWN:
        break;
    case WEAPONS:
        level++;
        break;
    case BOLT:
    case BEAM:
    case BALL:
    case PROJECTILE:
        level++;
        switch (elem)
        {
        case BOLT:
            weaptyp = WEAPON_TYPE::BOLT;
            break;
        case BEAM:
            weaptyp = WEAPON_TYPE::BEAM;
            break;
        case BALL:
            weaptyp = WEAPON_TYPE::BALL;
            break;
        case PROJECTILE:
            weaptyp = WEAPON_TYPE::PROJECTILE;
            break;
        default:
            weaptyp = WEAPON_TYPE::UNKNOWN;
            break;
        }
        tmpweapon.type = weaptyp;
        tmpweapon.file = getFilenameFromWeaponType(tmpweapon.type);
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case UNKNOWN:
                BOOST_LOG_TRIVIAL(warning) << boost::format("Unknown Weapon Element %1$s") % (*iter).name.c_str();
                break;
            case NAME:
                curname = (*iter).value;
                tmpweapon.name = curname;
                break;
                // TODO: remove completely
            /*case ROLE:
                tmpweapon.role_bits   = ROLES::readBitmask( iter->value );
                break;
            case ANTIROLE:
                tmpweapon.role_bits   = ROLES::readBitmask( iter->value );
                tmpweapon.role_bits   = ~tmpweapon.role_bits;
                break;*/
            case WEAPSIZE:
                tmpweapon.size = getMountSize( (*iter).value.c_str() );
                break;
            default:
                break;
            }
        }
        break;
    case APPEARANCE:
        level++;
        counts++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case UNKNOWN:
                BOOST_LOG_TRIVIAL(warning) << boost::format("Unknown Weapon Element %1$s") % (*iter).name.c_str();
                break;
            case XFILE:
                tmpweapon.file     = (*iter).value;
                break;
            case SOUNDMP3:
                tmpweapon.sound    = AUDCreateSoundMP3( (*iter).value, tmpweapon.type != WEAPON_TYPE::PROJECTILE );
                break;
            case SOUNDWAV:
                tmpweapon.sound    = AUDCreateSoundWAV( (*iter).value, tmpweapon.type == WEAPON_TYPE::PROJECTILE );
                break;
            case OFFSETX:
                tmpweapon.offset.i = XMLSupport::parse_floatf( iter->value );
                break;
            case OFFSETY:
                tmpweapon.offset.j = XMLSupport::parse_floatf( iter->value );
                break;
            case OFFSETZ:
                tmpweapon.offset.k = XMLSupport::parse_floatf( iter->value );
                break;
            case RED:
                tmpweapon.r = XMLSupport::parse_floatf( (*iter).value );
                break;
            case GREEN:
                tmpweapon.g = XMLSupport::parse_floatf( (*iter).value );
                break;
            case BLUE:
                tmpweapon.b = XMLSupport::parse_floatf( (*iter).value );
                break;
            case ALPHA:
                tmpweapon.a = XMLSupport::parse_floatf( (*iter).value );
                break;
            case TEXTURESTRETCH:
                tmpweapon.texture_stretch =
                    XMLSupport::parse_floatf( (*iter).value );
                break;
            default:
                break;
            }
        }
        if ( (gamma > 0) && Gamma_Needed( gamma, counts, 32 ) ) {
            //approximate the color func
            tmpweapon.b = (tmpweapon.b+color_step*5)/255.;
            tmpweapon.g = (tmpweapon.g+color_step/5)/255.;
            tmpweapon.r = (tmpweapon.r+color_step*2)/255.;
        }
        break;
    case ENERGY:
        level++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case UNKNOWN:
                BOOST_LOG_TRIVIAL(warning) << boost::format("Unknown Weapon Element %1$s") % (*iter).name.c_str();
                break;
            case CONSUMPTION:
                tmpweapon.energy_rate = XMLSupport::parse_floatf( (*iter).value );
                break;
            case RATE:
                tmpweapon.energy_rate = XMLSupport::parse_floatf( (*iter).value );
                break;
            case STABILITY:
                tmpweapon.stability  = XMLSupport::parse_floatf( (*iter).value );
                break;
            case REFIRE:
                tmpweapon.refire_rate = XMLSupport::parse_floatf( (*iter).value );
                break;
            case LOCKTIME:
                tmpweapon.lock_time   = XMLSupport::parse_floatf( (*iter).value );
                break;
            default:
                break;
            }
        }
        break;
    case DAMAGE:
        {
            level++;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case UNKNOWN:
                    BOOST_LOG_TRIVIAL(warning) << boost::format("Unknown Weapon Element %1$s") % (*iter).name.c_str();
                    break;
                case DAMAGE:
                    tmpweapon.damage = game_options.weapon_damage_efficiency*XMLSupport::parse_floatf( (*iter).value );
                    break;
                case RADIUS:
                    tmpweapon.radius = XMLSupport::parse_floatf( (*iter).value );
                    break;
                case RADIALSPEED:
                    tmpweapon.radial_speed = XMLSupport::parse_floatf( (*iter).value );
                    break;
                case PHASEDAMAGE:
                    tmpweapon.phase_damage = game_options.weapon_damage_efficiency*XMLSupport::parse_floatf( (*iter).value );
                    break;
                case RATE:
                    tmpweapon.damage    = game_options.weapon_damage_efficiency*XMLSupport::parse_floatf( (*iter).value );
                    break;
                case LONGRANGE:
                    tmpweapon.long_range = XMLSupport::parse_float( (*iter).value );
                    break;
                default:
                    break;
                }
            }
            break;
        }
    case DISTANCE:
        level++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case UNKNOWN:
                BOOST_LOG_TRIVIAL(warning) << boost::format("Unknown Weapon Element %1$s") % (*iter).name.c_str();
                break;
            case VOLUME:
                tmpweapon.volume = XMLSupport::parse_float( (*iter).value );
                break;
            case SPEED:
                tmpweapon.speed  = XMLSupport::parse_float( (*iter).value );
                if (tmpweapon.speed < 1000) {
                    tmpweapon.speed = tmpweapon.speed*(game_options.gun_speed_adjusted_game_speed ? (1.0+gun_speed/1.25) : gun_speed);
                } else {
                    if (tmpweapon.speed < 2000) {
                        tmpweapon.speed = tmpweapon.speed*( game_options.gun_speed_adjusted_game_speed ? (1.0+gun_speed/2.5) : (gun_speed) );
                    } else {
                        if (tmpweapon.speed < 4000)
                            tmpweapon.speed = tmpweapon.speed*( game_options.gun_speed_adjusted_game_speed ? (1.0+gun_speed/6.0) : (gun_speed) );
                        else if (tmpweapon.speed < 8000)
                            tmpweapon.speed = tmpweapon.speed*( game_options.gun_speed_adjusted_game_speed ? (1.0+gun_speed/17.0) : (gun_speed) );
                    }
                }
                break;
            case PULSESPEED:
                if (tmpweapon.type == WEAPON_TYPE::BEAM)
                    tmpweapon.pulse_speed = XMLSupport::parse_float( (*iter).value );
                break;
            case DETONATIONRADIUS:
                if (tmpweapon.type != WEAPON_TYPE::BEAM)
                    tmpweapon.pulse_speed = XMLSupport::parse_float( (*iter).value );
                break;
            case RADIALSPEED:
                tmpweapon.radial_speed = XMLSupport::parse_float( (*iter).value );
                break;
            case RANGE:
                tmpweapon.range = ( game_options.gun_speed_adjusted_game_speed ? (1.0+gun_speed/16.0) : (gun_speed) )*XMLSupport::parse_float(
                    (*iter).value );
                break;
            case RADIUS:
                tmpweapon.radius = XMLSupport::parse_float( (*iter).value );
                break;
            case LENGTH:
                tmpweapon.length = XMLSupport::parse_float( (*iter).value );
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

#undef Gamma_Needed

void endElement( void *userData, const XML_Char *name )
{
    Names elem = (Names) element_map.lookup( name );
    switch (elem)
    {
    case UNKNOWN:
        break;
    case WEAPONS:
        level--;
        break;
    case BEAM:
    case BOLT:
    case BALL:
    case PROJECTILE:
        level--;
        lookuptable.Put( strtoupper( curname ), new weapon_info( tmpweapon ) );
        tmpweapon = weapon_info();
        break;
    case ENERGY:
    case DAMAGE:
    case DISTANCE:
    case APPEARANCE:
        level--;
        break;
    default:
        break;
    }
}



using namespace VSFileSystem;

weapon_info * getTemplate( const string &kkey )
{
    weapon_info *wi = lookuptable.Get( strtoupper( kkey ) );
    if (wi) {
        if ( !WeaponMeshCache::getCachedMutable( wi->name ) ) {
            static FileLookupCache lookup_cache;
            string meshname = strtolower( kkey )+".bfxm";
            if (CachedFileLookup( lookup_cache, meshname, MeshFile ) <= Ok) {
                WeaponMeshCache::setCachedMutable( wi->name, wi->gun =
                                                      Mesh::LoadMesh( meshname.c_str(), Vector( 1, 1, 1 ), 0, NULL ) );
                WeaponMeshCache::setCachedMutable( wi->name+"_flare", wi->gun1 =
                                                      Mesh::LoadMesh( meshname.c_str(), Vector( 1, 1, 1 ), 0, NULL ) );
            }
        }
    }
    return wi;
}

void LoadWeapons( const char *filename )
{
    using namespace VSFileSystem;
    VSFile     f;
    VSError    err    = f.OpenReadOnly( filename, UnknownFile );
    if (err > Ok)
        return;
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetElementHandler( parser, &beginElementXML_Char, &endElement );
    XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
    f.Close();
    XML_ParserFree( parser );
}


