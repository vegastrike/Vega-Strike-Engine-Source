/**
 * unit_xml.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
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


#include "unit_xml.h"
#include "audiolib.h"
#include "xml_support.h"
#include <fstream>
#include <expat.h>
#include <float.h>
#include <limits.h>
#include "configxml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include <assert.h>
#include "images.h"
#include "xml_serializer.h"
#include "vsfilesystem.h"
#include "gfx/cockpit_generic.h"
#include "unit_collide.h"
#include "unit_generic.h"
#include "gfx/sphere.h"
#include "role_bitmask.h"
#include "cmd/collide2/Stdafx.h"
#include "cmd/collide2/CSopcodecollider.h"
#include "vs_math.h"
#include "unit.h"


using namespace XMLSupport;

/*ADDED FOR extensible use of unit pretty print and unit load */
UNITLOADTYPE current_unit_load_mode = DEFAULT;
extern float getFuelConversion();

string KillQuadZeros( string inp )
{
    std::string::size_type text = 0;
    while ( ( text = inp.find( ".000000", text ) ) != string::npos )
        inp = inp.substr( 0, text )+inp.substr( text+7 );
    return inp;
}

string MakeUnitXMLPretty( string str, Unit *un )
{
    string writestr;
    if (un) {
        writestr += "Name: "+un->name;
        writestr += " "+un->getFullname();
        Flightgroup *fg = un->getFlightgroup();
        if (fg)
            writestr += " Designation "+fg->name+" "+XMLSupport::tostring( un->getFgSubnumber() );
        writestr += "\n";
    }
    static std::set< string >lookfor;
    if ( lookfor.empty() ) {
        lookfor.insert( "Shie" );
        lookfor.insert( "Armo" );
//lookfor.insert ("Hull");
        lookfor.insert( "Reac" );
        lookfor.insert( "Moun" );
        lookfor.insert( "Comp" );
//lookfor.insert ("Desc");
        lookfor.insert( "Engi" );
        lookfor.insert( "Mane" );
        lookfor.insert( "Jump" );
//lookfor.insert ("Defe");
        lookfor.insert( "Stat" );
        lookfor.insert( "Engi" );
//lookfor.insert ("Hold");
        lookfor.insert( "Rada" );
    }
    std::string::size_type foundpos;
    while ( ( foundpos = str.find( "<" ) ) != string::npos ) {
        if (str.size() <= foundpos+1)
            break;
        str = str.substr( foundpos+1 );
        if (str.size() > 3) {
            char mycomp[5] = {str[0], str[1], str[2], str[3], 0};
            if ( lookfor.find( mycomp ) != lookfor.end() ) {
                int newline = str.find( ">" );
                if (newline > 0)
                    if (str[newline-1] == '/')
                        newline -= 1;
                writestr += KillQuadZeros( str.substr( 0, newline )+"\n" );
            }
        }
    }
    return writestr;
}

int GetModeFromName( const char *input_buffer )
{
    if (strlen( input_buffer ) > 3) {
        if (input_buffer[0] == 'a'
            && input_buffer[1] == 'd'
            && input_buffer[2] == 'd')
            return 1;
        if (input_buffer[0] == 'm'
            && input_buffer[1] == 'u'
            && input_buffer[2] == 'l')
            return 2;
    }
    return 0;
}

extern bool CheckAccessory( Unit* );

void Unit::beginElement( void *userData, const XML_Char *name, const XML_Char **atts )
{
    ( (Unit*) userData )->beginElement( name, AttributeList( atts ) );
}

void Unit::endElement( void *userData, const XML_Char *name )
{
    ( (Unit*) userData )->endElement( name );
}

namespace UnitXML
{
enum Names
{
    UNKNOWN,
    UNIT,
    SUBUNIT,
    MESHFILE,
    SHIELDMESH,
    RAPIDMESH,
    MOUNT,
    MESHLIGHT,
    DOCK,
    XFILE,
    X,
    Y,
    Z,
    RI,
    RJ,
    RK,
    QI,
    QJ,
    QK,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    ACTIVATIONSPEED,
    MOUNTSIZE,
    WEAPON,
    DEFENSE,
    ARMOR,
    WARPDRIVERATING,
    FORWARD,
    RETRO,
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    FRONTRIGHTTOP,
    BACKRIGHTTOP,
    FRONTLEFTTOP,
    BACKLEFTTOP,
    FRONTRIGHTBOTTOM,
    BACKRIGHTBOTTOM,
    FRONTLEFTBOTTOM,
    BACKLEFTBOTTOM,
    TOP,
    BOTTOM,
    SHIELDS,
    RECHARGE,
    LEAK,
    HULL,
    STRENGTH,
    STATS,
    MASS,
    MOMENTOFINERTIA,
    FUEL,
    THRUST,
    MANEUVER,
    YAW,
    ROLL,
    PITCH,
    ENGINE,
    COMPUTER,
    AACCEL,
    ENERGY,
    REACTOR,
    LIMIT,
    RESTRICTED,
    MAX,
    MIN,
    MAXSPEED,
    AFTERBURNER,
    SHIELDTIGHT,
    ITTS,
    AMMO,
    HUDIMAGE,
    SOUND,
    MINTARGETSIZE,
    MAXCONE,
    LOCKCONE,
    RANGE,
    ISCOLOR,
    RADAR,
    CLOAK,
    CLOAKRATE,
    CLOAKMIN,
    CLOAKENERGY,
    CLOAKGLASS,
    CLOAKWAV,
    CLOAKMP3,
    ENGINEWAV,
    ENGINEMP3,
    HULLWAV,
    HULLMP3,
    ARMORWAV,
    ARMORMP3,
    SHIELDWAV,
    SHIELDMP3,
    EXPLODEWAV,
    EXPLODEMP3,
    EXPLOSIONANI,
    COCKPIT,
    JUMP,
    DELAY,
    JUMPENERGY,
    JUMPWAV,
    NETCOM,
    NETCOMM_MINFREQ,
    NETCOMM_MAXFREQ,
    NETCOMM_SECURED,
    NETCOMM_VIDEO,
    NETCOMM_CRYPTO,
    DOCKINTERNAL,
    WORMHOLE,
    RAPID,
    AFTERBURNENERGY,
    MISSING,
    UNITSCALE,
    PRICE,
    VOLUME,
    QUANTITY,
    CARGO,
    HOLD,
    CATEGORY,
    IMPORT,
    PRICESTDDEV,
    QUANTITYSTDDEV,
    DAMAGE,
    COCKPITDAMAGE,
    REPAIRDROID,
    ECM,
    DESCRIPTION,
    UPGRADE,
    MOUNTOFFSET,
    SUBUNITOFFSET,
    SLIDE_START,
    SLIDE_END,
    TRACKINGCONE,
    MISSIONCARGO,
    MAXIMUM,
    LIGHTTYPE,
    COMBATROLE,
    RECURSESUBUNITCOLLISION,
    WARPENERGY,
    FACECAMERA,
    XYSCALE,
    INSYSENERGY,
    ZSCALE,
    NUMANIMATIONSTAGES,
    STARTFRAME,
    TEXTURESTARTTIME
};

const EnumMap::Pair element_names[37] = {
    EnumMap::Pair( "UNKNOWN",       UNKNOWN ),
    EnumMap::Pair( "Unit",          UNIT ),
    EnumMap::Pair( "SubUnit",       SUBUNIT ),
    EnumMap::Pair( "Sound",         SOUND ),
    EnumMap::Pair( "MeshFile",      MESHFILE ),
    EnumMap::Pair( "ShieldMesh",    SHIELDMESH ),
    EnumMap::Pair( "RapidMesh",     RAPIDMESH ),
    EnumMap::Pair( "Light",         MESHLIGHT ),
    EnumMap::Pair( "Defense",       DEFENSE ),
    EnumMap::Pair( "Armor",         ARMOR ),
    EnumMap::Pair( "Shields",       SHIELDS ),
    EnumMap::Pair( "Hull",          HULL ),
    EnumMap::Pair( "Stats",         STATS ),
    EnumMap::Pair( "Thrust",        THRUST ),
    EnumMap::Pair( "Maneuver",      MANEUVER ),
    EnumMap::Pair( "Engine",        ENGINE ),
    EnumMap::Pair( "Computer",      COMPUTER ),
    EnumMap::Pair( "Cloak",         CLOAK ),
    EnumMap::Pair( "Energy",        ENERGY ),
    EnumMap::Pair( "Reactor",       REACTOR ),
    EnumMap::Pair( "Restricted",    RESTRICTED ),
    EnumMap::Pair( "Yaw",           YAW ),
    EnumMap::Pair( "Pitch",         PITCH ),
    EnumMap::Pair( "Roll",          ROLL ),
    EnumMap::Pair( "Mount",         MOUNT ),
    EnumMap::Pair( "Radar",         RADAR ),
    EnumMap::Pair( "Cockpit",       COCKPIT ),
    EnumMap::Pair( "Jump",          JUMP ),
    EnumMap::Pair( "Netcomm",       NETCOM ),
    EnumMap::Pair( "Dock",          DOCK ),
    EnumMap::Pair( "Hold",          HOLD ),
    EnumMap::Pair( "Cargo",         CARGO ),
    EnumMap::Pair( "Category",      CATEGORY ),
    EnumMap::Pair( "Import",        IMPORT ),
    EnumMap::Pair( "CockpitDamage", COCKPITDAMAGE ),
    EnumMap::Pair( "Upgrade",       UPGRADE ),
    EnumMap::Pair( "Description",   DESCRIPTION ),
};

const EnumMap::Pair attribute_names[119] = {
    EnumMap::Pair( "UNKNOWN",                 UNKNOWN ),
    EnumMap::Pair( "missing",                 MISSING ),
    EnumMap::Pair( "file",                    XFILE ),
    EnumMap::Pair( "x",                       X ),
    EnumMap::Pair( "y",                       Y ),
    EnumMap::Pair( "z",                       Z ),
    EnumMap::Pair( "xyscale",                 XYSCALE ),
    EnumMap::Pair( "zscale",                  ZSCALE ),
    EnumMap::Pair( "ri",                      RI ),
    EnumMap::Pair( "rj",                      RJ ),
    EnumMap::Pair( "rk",                      RK ),
    EnumMap::Pair( "qi",                      QI ),
    EnumMap::Pair( "qj",                      QJ ),
    EnumMap::Pair( "qk",                      QK ),
    EnumMap::Pair( "activationSpeed",         ACTIVATIONSPEED ),
    EnumMap::Pair( "red",                     RED ),
    EnumMap::Pair( "green",                   GREEN ),
    EnumMap::Pair( "blue",                    BLUE ),
    EnumMap::Pair( "alpha",                   ALPHA ),
    EnumMap::Pair( "size",                    MOUNTSIZE ),
    EnumMap::Pair( "forward",                 FORWARD ),
    EnumMap::Pair( "retro",                   RETRO ),
    EnumMap::Pair( "frontrighttop",           FRONTRIGHTTOP ),
    EnumMap::Pair( "backrighttop",            BACKRIGHTTOP ),
    EnumMap::Pair( "frontlefttop",            FRONTLEFTTOP ),
    EnumMap::Pair( "backlefttop",             BACKLEFTTOP ),
    EnumMap::Pair( "frontrightbottom",        FRONTRIGHTBOTTOM ),
    EnumMap::Pair( "backrightbottom",         BACKRIGHTBOTTOM ),
    EnumMap::Pair( "frontleftbottom",         FRONTLEFTBOTTOM ),
    EnumMap::Pair( "backleftbottom",          BACKLEFTBOTTOM ),
    EnumMap::Pair( "front",                   FRONT ),
    EnumMap::Pair( "back",                    BACK ),
    EnumMap::Pair( "left",                    LEFT ),
    EnumMap::Pair( "right",                   RIGHT ),
    EnumMap::Pair( "top",                     TOP ),
    EnumMap::Pair( "bottom",                  BOTTOM ),
    EnumMap::Pair( "recharge",                RECHARGE ),
    EnumMap::Pair( "warpenergy",              WARPENERGY ),
    EnumMap::Pair( "insysenergy",             INSYSENERGY ),
    EnumMap::Pair( "leak",                    LEAK ),
    EnumMap::Pair( "strength",                STRENGTH ),
    EnumMap::Pair( "mass",                    MASS ),
    EnumMap::Pair( "momentofinertia",         MOMENTOFINERTIA ),
    EnumMap::Pair( "fuel",                    FUEL ),
    EnumMap::Pair( "yaw",                     YAW ),
    EnumMap::Pair( "pitch",                   PITCH ),
    EnumMap::Pair( "roll",                    ROLL ),
    EnumMap::Pair( "accel",                   AACCEL ),
    EnumMap::Pair( "limit",                   LIMIT ),
    EnumMap::Pair( "max",                     MAX ),
    EnumMap::Pair( "min",                     MIN ),
    EnumMap::Pair( "weapon",                  WEAPON ),
    EnumMap::Pair( "maxspeed",                MAXSPEED ),
    EnumMap::Pair( "afterburner",             AFTERBURNER ),
    EnumMap::Pair( "tightness",               SHIELDTIGHT ),
    EnumMap::Pair( "itts",                    ITTS ),
    EnumMap::Pair( "ammo",                    AMMO ),
    EnumMap::Pair( "HudImage",                HUDIMAGE ),
    EnumMap::Pair( "ExplosionAni",            EXPLOSIONANI ),
    EnumMap::Pair( "MaxCone",                 MAXCONE ),
    EnumMap::Pair( "TrackingCone",            TRACKINGCONE ),
    EnumMap::Pair( "LockCone",                LOCKCONE ),
    EnumMap::Pair( "MinTargetSize",           MINTARGETSIZE ),
    EnumMap::Pair( "Range",                   RANGE ),
    EnumMap::Pair( "EngineMp3",               ENGINEMP3 ),
    EnumMap::Pair( "EngineWav",               ENGINEWAV ),
    EnumMap::Pair( "HullMp3",                 HULLMP3 ),
    EnumMap::Pair( "HullWav",                 HULLWAV ),
    EnumMap::Pair( "ArmorMp3",                ARMORMP3 ),
    EnumMap::Pair( "ArmorWav",                ARMORWAV ),
    EnumMap::Pair( "ShieldMp3",               SHIELDMP3 ),
    EnumMap::Pair( "ShieldWav",               SHIELDWAV ),
    EnumMap::Pair( "ExplodeMp3",              EXPLODEMP3 ),
    EnumMap::Pair( "ExplodeWav",              EXPLODEWAV ),
    EnumMap::Pair( "CloakRate",               CLOAKRATE ),
    EnumMap::Pair( "CloakGlass",              CLOAKGLASS ),
    EnumMap::Pair( "CloakEnergy",             CLOAKENERGY ),
    EnumMap::Pair( "CloakMin",                CLOAKMIN ),
    EnumMap::Pair( "CloakMp3",                CLOAKMP3 ),
    EnumMap::Pair( "CloakWav",                CLOAKWAV ),
    EnumMap::Pair( "Color",                   ISCOLOR ),
    EnumMap::Pair( "Restricted",              RESTRICTED ),
    EnumMap::Pair( "Delay",                   DELAY ),
    EnumMap::Pair( "AfterburnEnergy",         AFTERBURNENERGY ),
    EnumMap::Pair( "JumpEnergy",              JUMPENERGY ),
    EnumMap::Pair( "JumpWav",                 JUMPWAV ),
    EnumMap::Pair( "min_freq",                NETCOMM_MINFREQ ),
    EnumMap::Pair( "max_freq",                NETCOMM_MAXFREQ ),
    EnumMap::Pair( "secured",                 NETCOMM_SECURED ),
    EnumMap::Pair( "video",                   NETCOMM_VIDEO ),
    EnumMap::Pair( "crypto_method",           NETCOMM_CRYPTO ),
    EnumMap::Pair( "DockInternal",            DOCKINTERNAL ),
    EnumMap::Pair( "RAPID",                   RAPID ),
    EnumMap::Pair( "Wormhole",                WORMHOLE ),
    EnumMap::Pair( "Scale",                   UNITSCALE ),
    EnumMap::Pair( "Price",                   PRICE ),
    EnumMap::Pair( "Volume",                  VOLUME ),
    EnumMap::Pair( "Quantity",                QUANTITY ),
    EnumMap::Pair( "PriceStdDev",             PRICESTDDEV ),
    EnumMap::Pair( "PriceStDev",              PRICESTDDEV ),
    EnumMap::Pair( "QuantityStdDev",          QUANTITYSTDDEV ),
    EnumMap::Pair( "Damage",                  DAMAGE ),
    EnumMap::Pair( "RepairDroid",             REPAIRDROID ),
    EnumMap::Pair( "ECM",                     ECM ),
    EnumMap::Pair( "Description",             DESCRIPTION ),
    EnumMap::Pair( "MountOffset",             MOUNTOFFSET ),
    EnumMap::Pair( "SubunitOffset",           SUBUNITOFFSET ),
    EnumMap::Pair( "SlideEnd",                SLIDE_START ),
    EnumMap::Pair( "SlideStart",              SLIDE_END ),
    EnumMap::Pair( "MissionCargo",            MISSIONCARGO ),
    EnumMap::Pair( "Maximum",                 MAXIMUM ),
    EnumMap::Pair( "LightType",               LIGHTTYPE ),
    EnumMap::Pair( "CombatRole",              COMBATROLE ),
    EnumMap::Pair( "RecurseSubunitCollision", RECURSESUBUNITCOLLISION ),
    EnumMap::Pair( "FaceCamera",              FACECAMERA ),
    EnumMap::Pair( "NumAnimationStages",      NUMANIMATIONSTAGES ),
    EnumMap::Pair( "StartFrame",              STARTFRAME ),
    EnumMap::Pair( "TextureStartTime",        TEXTURESTARTTIME ),
    EnumMap::Pair( "WarpDriveRating",         WARPDRIVERATING )
};

const EnumMap element_map( element_names, 37 );
const EnumMap attribute_map( attribute_names, 119 );
} //end of namespace

std::string delayucharStarHandler( const XMLType &input, void *mythis )
{
    static int    jumpdelaymult = XMLSupport::parse_int( vs_config->getVariable( "physics", "jump_delay_multiplier", "5" ) );
    unsigned char uc = (*input.w.uc)/jumpdelaymult;
    if (uc < 1)
        uc = 1;
    return XMLSupport::tostring( (int) uc );
}

//USED TO BE IN UNIT_FUNCTIONS*.CPP BUT NOW ON BOTH CLIENT AND SERVER SIDE
std::vector< Mesh* >MakeMesh( unsigned int mysize )
{
    std::vector< Mesh* >temp;
    for (unsigned int i = 0; i < mysize; i++)
        temp.push_back( NULL );
    return temp;
}

void addShieldMesh( Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg )
{
    static bool forceit = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "forceOneOneShieldBlend", "true" ) );
    xml->shieldmesh = Mesh::LoadMesh( filename, Vector( scale, scale, scale ), faction, fg );
    if (xml->shieldmesh && forceit) {
        xml->shieldmesh->SetBlendMode( ONE, ONE, true );
        xml->shieldmesh->setEnvMap( false, true );
        xml->shieldmesh->setLighting( true, true );
    }
}

void addRapidMesh( Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg )
{
    xml->rapidmesh = Mesh::LoadMesh( filename, Vector( scale, scale, scale ), faction, fg );
}

void pushMesh( std::vector< Mesh* > &meshes,
               float &randomstartframe,
               float &randomstartseconds,
               const char *filename,
               const float scale,
               int faction,
               class Flightgroup *fg,
               int startframe,
               double texturestarttime )
{
    vector< Mesh* >m = Mesh::LoadMeshes( filename, Vector( scale, scale, scale ), faction, fg );
    for (unsigned int i = 0; i < m.size(); ++i) {
        meshes.push_back( m[i] );
        if (startframe >= 0) {
            meshes.back()->setCurrentFrame( startframe );
        } else if (startframe == -2) {
            float r = ( (float) rand() )/RAND_MAX;
            meshes.back()->setCurrentFrame( r*meshes.back()->getFramesPerSecond() );
        } else if (startframe == -1) {
            if (randomstartseconds == 0)
                randomstartseconds = randomstartframe*meshes.back()->getNumLOD()/meshes.back()->getFramesPerSecond();
            meshes.back()->setCurrentFrame( randomstartseconds*meshes.back()->getFramesPerSecond() );
        }
        if (texturestarttime > 0) {
            meshes.back()->setTextureCumulativeTime( texturestarttime );
        } else {
            float  fps    = meshes.back()->getTextureFramesPerSecond();
            int    frames = meshes.back()->getNumTextureFrames();
            double ran    = randomstartframe;
            if (fps > 0 && frames > 1) {
                ran *= frames/fps;
            } else {
                ran *= 1000;
            }
            meshes.back()->setTextureCumulativeTime( ran );
        }
    }
}

Mount * createMount( const std::string &name, int ammo, int volume, float xyscale, float zscale, bool banked ) //short fix
{
    return new Mount( name.c_str(), ammo, volume, xyscale, zscale, 1, 1, banked );
}


extern int GetModeFromName( const char* );

extern int parseMountSizes( const char *str );

static unsigned int CLAMP_UINT( float x )
{
    return (unsigned int) ( ( (x) > 4294967295.0 ) ? (unsigned int) 4294967295U : ( (x) < 0 ? 0 : (x) ) );
}                                                                                                                             //short fix

#define ADDTAGNAME( a ) do {pImage->unitwriter->AddTag( a );} \
    while (0)
#define ADDTAG do {pImage->unitwriter->AddTag( name );} \
    while (0)
#define ADDELEMNAME( a, b, c ) do {pImage->unitwriter->AddElement( a, b, c );} \
    while (0)
#define ADDELEM( b, c ) do {pImage->unitwriter->AddElement( (*iter).name, b, c );} \
    while (0)
#define ADDDEFAULT do {pImage->unitwriter->AddElement( (*iter).name, stringHandler, XMLType( (*iter).value ) );} \
    while (0)
#define ADDELEMI( b ) do {ADDELEM( intStarHandler, XMLType( &b ) );} \
    while (0)
#define ADDELEMF( b ) do {ADDELEM( floatStarHandler, XMLType( &b ) );} \
    while (0)

void Unit::beginElement( const string &name, const AttributeList &attributes )
{
    using namespace UnitXML;
    static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
    static float game_accel = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_accel", "1" ) );
    Cargo   carg;
    float   act_speed = 0;
    int     volume    = -1; //short fix
    string  filename;
    QVector P;
    int     indx;
    QVector Q;
    QVector R;
    QVector pos;
    Vector size;
    GFXColor color;
    float   xyscale = -1;
    float   zscale = -1;
    bool    tempbool;
    unsigned int dirfrac = 0;
    float   fbrltb[6]    = {-1};
    AttributeList::const_iterator iter;
    GFXColor halocolor; //FIXME it's set, but not actually used
    int     ammo   = -1; //short fix
    MOUNT_SIZE     mntsiz = MOUNT_SIZE::NOWEAP;
    string  light_type;
    Names   elem   = (Names) element_map.lookup( name );
    switch (elem)
    {
    case SHIELDMESH:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                ADDELEM( stringHandler, (*iter).value );
                addShieldMesh( xml, (*iter).value.c_str(), xml->unitscale, faction, flightgroup );
                break;
            case SHIELDTIGHT:
                ADDDEFAULT;
                shieldtight = parse_float( (*iter).value );
                break;
            }
        }
        break;
    case RAPIDMESH:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                ADDDEFAULT;
                addRapidMesh( xml, (*iter).value.c_str(), xml->unitscale, faction, NULL );
                xml->hasColTree = true;
                break;
            case RAPID:
                ADDDEFAULT;
                xml->hasColTree = parse_bool( (*iter).value );
                break;
            }
        }
        break;
    case HOLD:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case VOLUME:
                ADDELEM( floatStarHandler, XMLType( &pImage->CargoVolume ) );
                ADDELEM( floatStarHandler, XMLType( &pImage->UpgradeVolume ) );
                pImage->UpgradeVolume = pImage->CargoVolume = parse_float( (*iter).value );
                break;
            }
        }
        pImage->unitwriter->AddTag( "Category" );
        pImage->unitwriter->AddElement( "file", Unit::cargoSerializer, XMLType( (int) 0 ) );
        pImage->unitwriter->EndTag( "Category" );
        break;
    case IMPORT:
        Q.i = Q.k = 0;
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case QUANTITY:
                carg.quantity = parse_int( (*iter).value );
                //import cargo from ze maztah liztz
                break;
            case PRICE:
                carg.price = parse_float( (*iter).value );
                break;
            case PRICESTDDEV:
                Q.i = parse_float( (*iter).value );
                break;
            case QUANTITYSTDDEV:
                Q.k = parse_float( (*iter).value );
                break;
            }
        }
        ImportPartList( xml->cargo_category, carg.price, Q.i, carg.quantity, Q.k );
        break;
    case CATEGORY:
        //this is autogenerated by the handler
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                xml->cargo_category = XMLSupport::replace_space( (*iter).value );
                break;
            }
        }
        break;
    case CARGO:
        ///handling taken care of above;
        assert( xml->unitlevel >= 2 );
        xml->unitlevel++;
        carg.category = xml->cargo_category;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case QUANTITY:
                carg.quantity    = parse_int( (*iter).value );
                break;
            case MASS:
                carg.mass        = parse_float( (*iter).value );
                break;
            case VOLUME:
                carg.volume      = parse_float( (*iter).value );
                break;
            case PRICE:
                carg.price       = parse_float( (*iter).value );
                break;
            case MISSIONCARGO:
                carg.mission     = parse_bool( (*iter).value );
                break;
            case XFILE:
                carg.content     = XMLSupport::replace_space( (*iter).value );
                break;
            case DESCRIPTION:
                carg.description = strdup( (*iter).value.c_str() );                   //mem leak...but hey--only for mpl
                break;
            }
        }
        if (carg.mass != 0)
            AddCargo( carg, false );
        break;
    case MESHFILE:
        {
            std::string file  = "box.bfxm";
            int    startframe = 0;
            double texturestarttime = 0;
            ADDTAG;
            assert( xml->unitlevel == 1 );
            xml->unitlevel++;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case XFILE:
                    ADDDEFAULT;
                    file = (*iter).value;
                    break;
                case STARTFRAME:
                    if (strtoupper( (*iter).value ) == "RANDOM")
                        startframe = -1;
                    else if (strtoupper( (*iter).value ) == "ASYNC")
                        startframe = -2;
                    else
                        startframe = parse_int( (*iter).value );
                    break;
                case TEXTURESTARTTIME:
                    if (strtoupper( (*iter).value ) == "RANDOM")
                        texturestarttime = -1;
                    else
                        texturestarttime = parse_float( (*iter).value );
                }
            }
            switch (current_unit_load_mode)
            {
            case NO_MESH:
                break;
            default:
                pushMesh( xml->meshes, xml->randomstartframe, xml->randomstartseconds,
                          file.c_str(), xml->unitscale, faction, flightgroup, startframe, texturestarttime );
            }
            break;
        }
    case UPGRADE:
        {
            assert( xml->unitlevel >= 1 );
            xml->unitlevel++;

            double percent;
            int    moffset = 0;
            int    soffset = 0;
            //don't serialize
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case XFILE:
                    filename = (*iter).value.c_str();
                    break;
                case SUBUNITOFFSET:
                    soffset  = parse_int( (*iter).value );
                    break;
                case MOUNTOFFSET:
                    moffset  = parse_int( (*iter).value );
                    break;
                }
            }
            int   upgrfac  = FactionUtil::GetUpgradeFaction();
            Unit *upgradee = new GameUnit( filename.c_str(), true, upgrfac );
            Unit::Upgrade( upgradee, moffset, soffset, GetModeFromName( filename.c_str() ), true, percent, NULL );
            upgradee->Kill();
            break;
        }
    case DOCK:
        {
            ADDTAG;
            DockingPorts::Type::Value dockType = DockingPorts::Type::DEFAULT;
            assert( xml->unitlevel == 1 );
            xml->unitlevel++;
            pos = QVector( 0, 0, 0 );
            P   = QVector( 1, 1, 1 );
            Q   = QVector( FLT_MAX, FLT_MAX, FLT_MAX );
            R   = QVector( FLT_MAX, FLT_MAX, FLT_MAX );
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case DOCKINTERNAL:
                    ADDDEFAULT;
                    dockType = DockingPorts::Type::Value(parse_int((*iter).value));
                    break;
                case X:
                    ADDDEFAULT;
                    pos.i = xml->unitscale*parse_float( (*iter).value );
                    break;
                case Y:
                    ADDDEFAULT;
                    pos.j = xml->unitscale*parse_float( (*iter).value );
                    break;
                case Z:
                    ADDDEFAULT;
                    pos.k = xml->unitscale*parse_float( (*iter).value );
                    break;
                case TOP:
                    ADDDEFAULT;
                    R.j = xml->unitscale*parse_float( (*iter).value );
                    break;
                case BOTTOM:
                    ADDDEFAULT;
                    Q.j = xml->unitscale*parse_float( (*iter).value );
                    break;
                case LEFT:
                    ADDDEFAULT;
                    Q.i = xml->unitscale*parse_float( (*iter).value );
                    break;
                case RIGHT:
                    ADDDEFAULT;
                    R.i = parse_float( (*iter).value );
                    break;
                case BACK:
                    ADDDEFAULT;
                    Q.k = xml->unitscale*parse_float( (*iter).value );
                    break;
                case FRONT:
                    ADDDEFAULT;
                    R.k = xml->unitscale*parse_float( (*iter).value );
                    break;
                case MOUNTSIZE:
                    ADDDEFAULT;
                    P.i = xml->unitscale*parse_float( (*iter).value );
                    P.j = xml->unitscale*parse_float( (*iter).value );
                    break;
                }
            }
            if (Q.i == FLT_MAX || Q.j == FLT_MAX || Q.k == FLT_MAX || R.i == FLT_MAX || R.j == FLT_MAX || R.k == FLT_MAX) {
                pImage->dockingports.push_back( DockingPorts( pos.Cast(), P.i, 0, dockType ) );
            } else {
                QVector tQ = Q.Min( R );
                QVector tR = R.Max( Q );
                pImage->dockingports.push_back( DockingPorts( tQ.Cast(), tR.Cast(), 0, dockType ) );
            }
        }
        break;
    case MESHLIGHT:
        ADDTAG;
        halocolor = vs_config->getColor( "unit", "engine", GFXColor( 1, 1, 1, 1 ) );
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        P   = QVector( 1, 0, 0 );
        Q   = QVector( 0, 1, 0 );
        R   = QVector( 0, 0, 1 );
        pos = QVector( 0, 0, 0 );
        size = Vector( 1, 1, 1 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case LIGHTTYPE:
                ADDDEFAULT;
                light_type = (*iter).value;
                break;
            case X:
                ADDDEFAULT;
                pos.i = xml->unitscale*parse_float( (*iter).value );
                break;
            case Y:
                ADDDEFAULT;
                pos.j = xml->unitscale*parse_float( (*iter).value );
                break;
            case Z:
                ADDDEFAULT;
                pos.k = xml->unitscale*parse_float( (*iter).value );
                break;
            case RED:
                ADDDEFAULT;
                color.r = parse_float( (*iter).value );
                break;
            case GREEN:
                ADDDEFAULT;
                color.g = parse_float( (*iter).value );
                break;
            case BLUE:
                ADDDEFAULT;
                color.b = parse_float( (*iter).value );
                break;
            case ALPHA:
                ADDDEFAULT;
                color.a = parse_float( (*iter).value );
                break;
            case XFILE:
                ADDDEFAULT;
                filename  = (*iter).value;
                break;
            case ACTIVATIONSPEED:
                act_speed = parse_float( (*iter).value );
                break;
            case MOUNTSIZE:
                ADDDEFAULT;
                size.i = xml->unitscale*parse_float( (*iter).value );
                size.j = xml->unitscale*parse_float( (*iter).value );
                size.k = xml->unitscale*parse_float( (*iter).value );
                break;
            case RI:
                R.i = parse_float( (*iter).value );
                break;
            case RJ:
                R.j = parse_float( (*iter).value );
                break;
            case RK:
                R.k = parse_float( (*iter).value );
                break;
            case QI:
                Q.i = parse_float( (*iter).value );
                break;
            case QJ:
                Q.j = parse_float( (*iter).value );
                break;
            case QK:
                Q.k = parse_float( (*iter).value );
                break;
            }
        }
        Q.Normalize();
        if ( fabs( Q.i ) == fabs( R.i ) && fabs( Q.j ) == fabs( R.j ) && fabs( Q.k ) == fabs( R.k ) ) {
            Q = QVector(-1, 0, 0);
        }
        R.Normalize();
        CrossProduct( Q, R, P );
        CrossProduct( R, P, Q );
        Q.Normalize();
        addHalo( filename.c_str(), Matrix( P.Cast(), Q.Cast(), R.Cast(), pos ), size, color, light_type, act_speed );
        break;
    case MOUNT:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        Q   = QVector( 0, 1, 0 );
        R   = QVector( 0, 0, 1 );
        pos = QVector( 0, 0, 0 );
        tempbool = false;
        ADDELEMNAME( "size", Unit::mountSerializer, XMLType( XMLSupport::tostring( xml->unitscale ), (int) xml->mountz.size() ) );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case VOLUME:
                volume   = XMLSupport::parse_int( (*iter).value );
                break;
            case XYSCALE:
                xyscale  = XMLSupport::parse_float( (*iter).value );
                break;
            case ZSCALE:
                zscale   = XMLSupport::parse_float( (*iter).value );
                break;
            case WEAPON:
                filename = (*iter).value;
                break;
            case AMMO:
                ammo     = XMLSupport::parse_int( (*iter).value );
                break;
            case MOUNTSIZE:
                tempbool = true;
                mntsiz   = getMountSizeFromItsValue(parseMountSizes( (*iter).value.c_str() ));
                break;
            case X:
                pos.i    = xml->unitscale*parse_float( (*iter).value );
                break;
            case Y:
                pos.j    = xml->unitscale*parse_float( (*iter).value );
                break;
            case Z:
                pos.k    = xml->unitscale*parse_float( (*iter).value );
                break;
            case RI:
                R.i = parse_float( (*iter).value );
                break;
            case RJ:
                R.j = parse_float( (*iter).value );
                break;
            case RK:
                R.k = parse_float( (*iter).value );
                break;
            case QI:
                Q.i = parse_float( (*iter).value );
                break;
            case QJ:
                Q.j = parse_float( (*iter).value );
                break;
            case QK:
                Q.k = parse_float( (*iter).value );
                break;
            }
        }
        Q.Normalize();
        if ( fabs( Q.i ) == fabs( R.i ) && fabs( Q.j ) == fabs( R.j ) && fabs( Q.k ) == fabs( R.k ) ) {
            Q.i = -1;
            Q.j = 0;
            Q.k = 0;
        }
        R.Normalize();
        CrossProduct( Q, R, P );
        CrossProduct( R, P, Q );
        Q.Normalize();
        //Transformation(Quaternion (from_vectors (P,Q,R),pos);
        indx = xml->mountz.size();
        xml->mountz.push_back( createMount( filename.c_str(), ammo, volume, xyscale, zscale, false /*no way to do banked in XML*/ ) );
        xml->mountz[indx]->SetMountOrientation( Quaternion::from_vectors( P.Cast(), Q.Cast(), R.Cast() ) );
        xml->mountz[indx]->SetMountPosition( pos.Cast() );
        if (tempbool)
            xml->mountz[indx]->size = as_integer(mntsiz);
        else
            xml->mountz[indx]->size = as_integer(xml->mountz[indx]->type->size);
        setAverageGunSpeed();
        break;
    case SUBUNIT:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        ADDELEMNAME( "file", Unit::subunitSerializer, XMLType( (int) xml->units.size() ) );
        xml->unitlevel++;
        Q   = QVector( 0, 1, 0 );
        R   = QVector( 0, 0, 1 );
        pos = QVector( 0, 0, 0 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                filename = (*iter).value;
                break;
            case X:
                ADDDEFAULT;
                pos.i = xml->unitscale*parse_float( (*iter).value );
                break;
            case Y:
                ADDDEFAULT;
                pos.j = xml->unitscale*parse_float( (*iter).value );
                break;
            case Z:
                ADDDEFAULT;
                pos.k = xml->unitscale*parse_float( (*iter).value );
                break;
            case RI:
                ADDDEFAULT;
                R.i = parse_float( (*iter).value );
                break;
            case RJ:
                ADDDEFAULT;
                R.j = parse_float( (*iter).value );
                break;
            case RK:
                ADDDEFAULT;
                R.k = parse_float( (*iter).value );
                break;
            case QI:
                ADDDEFAULT;
                Q.i = parse_float( (*iter).value );
                break;
            case QJ:
                ADDDEFAULT;
                Q.j = parse_float( (*iter).value );
                break;
            case QK:
                ADDDEFAULT;
                Q.k = parse_float( (*iter).value );
                break;
            case RESTRICTED:
                ADDDEFAULT;
                fbrltb[0] = parse_float( (*iter).value ); //minimum dot turret can have with "fore" vector
                break;
            }
        }
        indx = xml->units.size();
        xml->units.push_back( new GameUnit( filename.c_str(), true, faction, xml->unitModifications, NULL ) ); //I set here the fg arg to NULL
        if (xml->units.back()->name == "LOAD_FAILED") {
            xml->units.back()->limits.yaw = 0;
            xml->units.back()->limits.pitch = 0;
            xml->units.back()->limits.roll = 0;
            xml->units.back()->limits.lateral = xml->units.back()->limits.retro = xml->units.back()->limits.forward =
                                                                                      xml->units.back()->limits.afterburn = 0.0;
        }
        xml->units.back()->SetRecursiveOwner( this );
        xml->units[indx]->SetOrientation( Q, R );
        R.Normalize();
        xml->units[indx]->prev_physical_state = xml->units[indx]->curr_physical_state;
        xml->units[indx]->SetPosition( pos );
        xml->units[indx]->limits.structurelimits = R.Cast();
        xml->units[indx]->limits.limitmin = fbrltb[0];
        xml->units[indx]->name = filename;
        if (xml->units[indx]->pImage->unitwriter != NULL)
            xml->units[indx]->pImage->unitwriter->setName( filename );
        CheckAccessory( xml->units[indx] );         //turns on the ceerazy rotation for the turret
        break;
    case COCKPITDAMAGE:
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case DAMAGE:
                pImage->cockpit_damage[xml->damageiterator++] = parse_float( (*iter).value );
                break;
            }
        }
        break;
    case NETCOM:
        {
            string method;
            assert( xml->unitlevel == 1 );
            xml->unitlevel++;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case NETCOMM_MINFREQ:
                    break;
                case NETCOMM_MAXFREQ:
                    break;
                case NETCOMM_SECURED:
                    break;
                case NETCOMM_VIDEO:
                    break;
                case NETCOMM_CRYPTO:
                    method  = (*iter).value;
                    break;
                }
            }
            break;
        }
    case JUMP:
        {
            static float insys_jump_cost =
                XMLSupport::parse_float( vs_config->getVariable( "physics", "insystem_jump_cost", ".1" ) );
            bool foundinsysenergy = false;
            //serialization covered in LoadXML
            assert( xml->unitlevel == 1 );
            xml->unitlevel++;
            jump.drive = -1;     //activate the jump unit
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case MISSING:
                    //serialization covered in LoadXML
                    if ( parse_bool( (*iter).value ) )
                        jump.drive = -2;
                    break;
                case JUMPENERGY:
                    //serialization covered in LoadXML
                    jump.energy = parse_float( (*iter).value );               //short fix
                    if (!foundinsysenergy)
                        jump.insysenergy = jump.energy*insys_jump_cost;
                    break;
                case INSYSENERGY:
                    //serialization covered in LoadXML
                    jump.insysenergy     = parse_float( (*iter).value );               //short fix
                    foundinsysenergy     = true;
                    break;
                case WARPDRIVERATING:
                    jump.warpDriveRating = parse_float( (*iter).value );
                    break;
                case DAMAGE:
                    jump.damage = float_to_int( parse_float( (*iter).value ) );               //short fix
                    break;
                case DELAY:
                    //serialization covered in LoadXML
                    {
                        static int jumpdelaymult =
                            XMLSupport::parse_int( vs_config->getVariable( "physics", "jump_delay_multiplier", "5" ) );
                        jump.delay = parse_int( (*iter).value )*jumpdelaymult;
                        break;
                    }
                case FUEL:
                    //serialization covered in LoadXML
                    jump.energy = -parse_float( (*iter).value );               //short fix
                    break;
                case WORMHOLE:
                    //serialization covered in LoadXML
                    pImage->forcejump = parse_bool( (*iter).value );
                    if (pImage->forcejump)
                        jump.drive = -2;
                    break;
                }
            }
            break;
        }
    case SOUND:
        // TODO: refactor this
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case CLOAKWAV:
                ADDDEFAULT;
                //sound->cloak = AUDCreateSoundWAV( (*iter).value, false );
                addSound((*iter).value, SoundType::cloaking);
                break;
            case JUMPWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::jump);
                break;
            case CLOAKMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::cloaking);
                break;
            case ENGINEWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::engine);
                break;
            case ENGINEMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::engine);
                break;
            case SHIELDMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::shield);
                break;
            case SHIELDWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::shield);
                break;
            case EXPLODEMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::explosion);
                break;
            case EXPLODEWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::explosion);
                break;
            case ARMORMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::armor);
                break;
            case ARMORWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::armor);
                break;
            case HULLWAV:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::hull);
                break;
            case HULLMP3:
                ADDDEFAULT;
                addSound((*iter).value, SoundType::hull);
                break;
            }
        }

        addDefaultSounds();

        break;
    case CLOAK:
        //serialization covered elsewhere
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        pImage->cloakrate   = (int) ( .2*(2147483647) );           //short fix
        cloakmin = 1;
        pImage->cloakenergy = 0;
        cloaking = INT_MIN;         //lowest negative number  //short fix
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MISSING:
                //serialization covered in LoadXML
                if ( parse_bool( (*iter).value ) )
                    cloaking = -1;                      //short fix
                break;
            case CLOAKMIN:
                //serialization covered in LoadXML
                cloakmin = (int) ( ( (-1) > 1 )*parse_float( (*iter).value ) );                     //short fix
                break;
            case CLOAKGLASS:
                //serialization covered in LoadXML
                pImage->cloakglass = parse_bool( (*iter).value );
                break;
            case CLOAKRATE:
                //serialization covered in LoadXML
                pImage->cloakrate = (int) ( (2147483647)*parse_float( (*iter).value ) );                     //short fix
                break;
            case CLOAKENERGY:
                //serialization covered in LoadXML
                pImage->cloakenergy = parse_float( (*iter).value );
                break;
            }
        }
        if ( (cloakmin&0x1) && !pImage->cloakglass )
            cloakmin -= 1;
        if ( (cloakmin&0x1) == 0 && pImage->cloakglass )
            cloakmin += 1;
        break;
    case ARMOR:
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case FRONT:
                dirfrac = ( CLAMP_UINT( parse_float( (*iter).value ) ) )/4;
                armor.frontrighttop    += dirfrac;
                armor.frontlefttop     += dirfrac;
                armor.frontrightbottom += dirfrac;
                armor.frontleftbottom  += dirfrac;
                break;
            case BACK:
                dirfrac = ( CLAMP_UINT( parse_float( (*iter).value ) ) )/4;
                armor.backrighttop    += dirfrac;
                armor.backlefttop     += dirfrac;
                armor.backrightbottom += dirfrac;
                armor.backleftbottom  += dirfrac;
                break;
            case RIGHT:
                dirfrac = ( CLAMP_UINT( parse_float( (*iter).value ) ) )/4;
                armor.frontrighttop    += dirfrac;
                armor.backrighttop     += dirfrac;
                armor.frontrightbottom += dirfrac;
                armor.backrightbottom  += dirfrac;
                break;
            case LEFT:
                dirfrac = ( CLAMP_UINT( parse_float( (*iter).value ) ) )/4;
                armor.backlefttop     += dirfrac;
                armor.frontlefttop    += dirfrac;
                armor.backleftbottom  += dirfrac;
                armor.frontleftbottom += dirfrac;
                break;

            case FRONTRIGHTTOP:
                //serialization covered in LoadXML
                armor.frontrighttop = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case BACKRIGHTTOP:
                //serialization covered in LoadXML
                armor.backrighttop = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case FRONTLEFTTOP:
                //serialization covered in LoadXML
                armor.frontlefttop = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case BACKLEFTTOP:
                //serialization covered in LoadXML
                armor.backlefttop = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case FRONTRIGHTBOTTOM:
                //serialization covered in LoadXML
                armor.frontrightbottom = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case BACKRIGHTBOTTOM:
                //serialization covered in LoadXML
                armor.backrightbottom = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case FRONTLEFTBOTTOM:
                //serialization covered in LoadXML
                armor.frontleftbottom = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            case BACKLEFTBOTTOM:
                //serialization covered in LoadXML
                armor.backleftbottom = CLAMP_UINT( parse_float( (*iter).value ) );                   //short fix
                break;
            }
        }
        break;
    case SHIELDS:
        //serialization covered in LoadXML
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case FRONT:
                //serialization covered in LoadXML
                fbrltb[0] = parse_float( (*iter).value );
                shield.number++;
                break;
            case BACK:
                //serialization covered in LoadXML
                fbrltb[1] = parse_float( (*iter).value );
                shield.number++;
                break;
            case LEFT:
                //serialization covered in LoadXML
                fbrltb[3] = parse_float( (*iter).value );
                shield.number++;
                break;
            case RIGHT:
                //serialization covered in LoadXML
                fbrltb[2] = parse_float( (*iter).value );
                shield.number++;
                break;
            case TOP:
                //serialization covered in LoadXML
                fbrltb[4] = parse_float( (*iter).value );
                shield.number++;
                break;
            case BOTTOM:
                //serialization covered in LoadXML
                fbrltb[5] = parse_float( (*iter).value );
                shield.number++;
                break;
            case RECHARGE:
                //serialization covered in LoadXML
                shield.recharge = parse_float( (*iter).value );
                break;
            case LEAK:
                //serialization covered in LoadXML
                shield.leak = parse_int( (*iter).value );
                break;
            }
        }
        switch (shield.number)
        {
        case 2:
            shield.shield2fb.frontmax = shield.shield2fb.front = fbrltb[0];             //short fix
            shield.shield2fb.backmax = shield.shield2fb.back = fbrltb[1];             //short fix
            break;
        case 8:         //short fix
            shield.shield8.frontrighttop    = CLAMP_UINT( .25*fbrltb[0]+.25*fbrltb[2] );             //short fix
            shield.shield8.backrighttop     = CLAMP_UINT( .25*fbrltb[1]+.25*fbrltb[2] );             //short fix
            shield.shield8.frontlefttop     = CLAMP_UINT( .25*fbrltb[0]+.25*fbrltb[3] );             //short fix
            shield.shield8.backlefttop      = CLAMP_UINT( .25*fbrltb[1]+.25*fbrltb[3] );             //short fix
            shield.shield8.frontrightbottom = CLAMP_UINT( .25*fbrltb[0]+.25*fbrltb[2] );             //short fix
            shield.shield8.backrightbottom  = CLAMP_UINT( .25*fbrltb[1]+.25*fbrltb[2] );             //short fix
            shield.shield8.frontleftbottom  = CLAMP_UINT( .25*fbrltb[0]+.25*fbrltb[3] );             //short fix
            shield.shield8.backleftbottom   = CLAMP_UINT( .25*fbrltb[1]+.25*fbrltb[3] );             //short fix

            break;
        case 4:
        default:
            shield.shield4fbrl.frontmax = shield.shield4fbrl.front = (fbrltb[0]);               //short fix
            shield.shield4fbrl.backmax  = shield.shield4fbrl.back = (fbrltb[1]);               //short fix
            shield.shield4fbrl.rightmax = shield.shield4fbrl.right = (fbrltb[2]);               //short fix
            shield.shield4fbrl.leftmax  = shield.shield4fbrl.left = fbrltb[3];             //short fix
        }
        break;
    case HULL:
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        maxhull = 0;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case STRENGTH:
                hull    = parse_float( (*iter).value );
                break;
            case MAXIMUM:
                maxhull = parse_float( (*iter).value );
                break;
            }
        }
        if (maxhull == 0) {
            maxhull = hull;
            if (maxhull == 0)
                maxhull = 1;
        }
        break;
    case STATS:
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MASS:
                Mass = parse_float( (*iter).value );
                break;
            case MOMENTOFINERTIA:
                Momentofinertia = parse_float( (*iter).value );
                break;
            case FUEL:
                fuel = Mass*60*getFuelConversion();
                //FIXME! This is a hack until we get csv support
                //FIXME FIXME FIXME got support a long time ago! --chuck_starchaser
                break;
            }
        }
        break;
    case MANEUVER:
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case YAW:
                limits.yaw   = parse_float( (*iter).value )*(VS_PI/180);
                break;
            case PITCH:
                limits.pitch = parse_float( (*iter).value )*(VS_PI/180);
                break;
            case ROLL:
                limits.roll  = parse_float( (*iter).value )*(VS_PI/180);
                break;
            }
        }
        break;
    case ENGINE:
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case AACCEL:
                break;
            case FORWARD:
                limits.forward   = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case RETRO:
                limits.retro     = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case AFTERBURNER:
                limits.afterburn = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case LEFT:
                limits.lateral   = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case RIGHT:
                limits.lateral   = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case TOP:
                limits.vertical  = game_speed*game_accel*parse_float( (*iter).value );
                break;
            case BOTTOM:
                limits.vertical  = game_speed*game_accel*parse_float( (*iter).value );
                break;
            }
        }
        break;
    case COMPUTER:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MAXSPEED:
                computer.max_combat_speed = game_speed*parse_float( (*iter).value );
                ADDELEM( speedStarHandler, XMLType( &computer.max_combat_speed ) );
                break;
            case AFTERBURNER:
                computer.max_combat_ab_speed = game_speed*parse_float( (*iter).value );
                ADDELEM( speedStarHandler, XMLType( &computer.max_combat_ab_speed ) );
                break;
            case YAW:
                computer.max_yaw_right = computer.max_yaw_left = parse_float( (*iter).value )*(VS_PI/180);
                ADDELEM( angleStarHandler, XMLType( &computer.max_yaw_right ) );
                break;
            case PITCH:
                computer.max_pitch_up = computer.max_pitch_down = parse_float( (*iter).value )*(VS_PI/180);
                ADDELEM( angleStarHandler, XMLType( &computer.max_pitch_up ) );
                break;
            case ROLL:
                computer.max_roll_right = computer.max_roll_left = parse_float( (*iter).value )*(VS_PI/180);
                ADDELEM( angleStarHandler, XMLType( &computer.max_roll_right ) );
                break;
            case SLIDE_START:
                computer.slide_start = parse_int( (*iter).value );
                ADDELEM( ucharStarHandler, XMLType( &computer.slide_start ) );
                break;
            case SLIDE_END:
                computer.slide_end = parse_int( (*iter).value );
                ADDELEM( ucharStarHandler, XMLType( &computer.slide_end ) );
                break;
            }
        }
        pImage->unitwriter->AddTag( "Radar" );
        ADDELEMNAME( "itts", boolStarHandler, XMLType( &computer.itts ) );
        ADDELEMNAME( "color", charStarHandler, XMLType( &computer.radar.capability ) );
        ADDELEMNAME( "mintargetsize", charStarHandler, XMLType( &computer.radar.mintargetsize ) );
        ADDELEMNAME( "range", floatStarHandler, XMLType( &computer.radar.maxrange ) );
        ADDELEMNAME( "maxcone", floatStarHandler, XMLType( &computer.radar.maxcone ) );
        ADDELEMNAME( "TrackingCone", floatStarHandler, XMLType( &computer.radar.trackingcone ) );
        ADDELEMNAME( "lockcone", floatStarHandler, XMLType( &computer.radar.lockcone ) );
        pImage->unitwriter->EndTag( "Radar" );
        break;
    case RADAR:
        //handled above
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case ITTS:
                computer.itts = parse_bool( (*iter).value );
                break;
            case MINTARGETSIZE:
                computer.radar.mintargetsize = parse_float( (*iter).value );
                break;
            case MAXCONE:
                computer.radar.maxcone      = parse_float( (*iter).value );
                break;
            case LOCKCONE:
                computer.radar.lockcone     = parse_float( (*iter).value );
                break;
            case TRACKINGCONE:
                computer.radar.trackingcone = parse_float( (*iter).value );
                break;
            case RANGE:
                computer.radar.maxrange     = parse_float( (*iter).value );
                break;
            case ISCOLOR:
               computer.radar.capability = atoi( (*iter).value.c_str() );
                if (computer.radar.capability == 0)
                    computer.radar.capability = parse_bool( (*iter).value );
                break;
            }
        }
        break;
    case REACTOR:
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case RECHARGE:
                recharge = parse_float( (*iter).value );
                break;
            case WARPENERGY:
                maxwarpenergy = ( parse_float( (*iter).value ) );                   //short fix
                break;
            case LIMIT:
                maxenergy     = energy = parse_float( (*iter).value );
                break;
            }
        }
        break;
    case YAW:
        ADDTAG;
        xml->yprrestricted += Unit::XML::YRESTR;
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MAX:
                ADDDEFAULT;
                xml->ymax = parse_float( (*iter).value )*(VS_PI/180);
                break;
            case MIN:
                ADDDEFAULT;
                xml->ymin = parse_float( (*iter).value )*(VS_PI/180);
                break;
            }
        }
        break;

    case PITCH:
        ADDTAG;
        xml->yprrestricted += Unit::XML::PRESTR;
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MAX:
                ADDDEFAULT;
                xml->pmax = parse_float( (*iter).value )*(VS_PI/180);
                break;
            case MIN:
                ADDDEFAULT;
                xml->pmin = parse_float( (*iter).value )*(VS_PI/180);
                break;
            }
        }
        break;
    case DESCRIPTION:
        ADDTAG;
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case RECURSESUBUNITCOLLISION:
                ADDDEFAULT;
                graphicOptions.RecurseIntoSubUnitsOnCollision = XMLSupport::parse_bool( iter->value );
                break;
            case FACECAMERA:
                ADDDEFAULT;
                graphicOptions.FaceCamera = XMLSupport::parse_bool( iter->value );
                break;
            case COMBATROLE:
                ADDDEFAULT;
                xml->calculated_role = true;
                this->setCombatRole( iter->value );
                break;
            case NUMANIMATIONSTAGES:
                graphicOptions.NumAnimationPoints = XMLSupport::parse_int( iter->value );
                if (graphicOptions.NumAnimationPoints > 0)
                    graphicOptions.Animating = 0;
                break;
            }
        }
        break;
    case ROLL:
        ADDTAG;
        xml->yprrestricted += Unit::XML::RRESTR;
        assert( xml->unitlevel == 2 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case MAX:
                ADDDEFAULT;
                xml->rmax = parse_float( (*iter).value )*(VS_PI/180);
                break;
            case MIN:
                ADDDEFAULT;
                xml->rmin = parse_float( (*iter).value )*(VS_PI/180);
                break;
            }
        }
        break;
    case UNIT:
        assert( xml->unitlevel == 0 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            default:
                break;
            case UNITSCALE:
                xml->unitscale = parse_float( (*iter).value );
                break;
            case COCKPIT:
                BOOST_LOG_TRIVIAL(warning) << "Cockpit attrib deprecated use tag";
                break;
            }
        }
        break;
    case COCKPIT:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case XFILE:
                pImage->cockpitImage = (*iter).value;
                ADDELEM( stringStarHandler, XMLType( &pImage->cockpitImage ) );
                break;
            case X:
                pImage->CockpitCenter.i = xml->unitscale*parse_float( (*iter).value );
                ADDELEM( scaledFloatStarHandler, XMLType( tostring( xml->unitscale ), &pImage->CockpitCenter.i ) );
                break;
            case Y:
                pImage->CockpitCenter.j = xml->unitscale*parse_float( (*iter).value );
                ADDELEM( scaledFloatStarHandler, XMLType( tostring( xml->unitscale ), &pImage->CockpitCenter.j ) );
                break;
            case Z:
                pImage->CockpitCenter.k = xml->unitscale*parse_float( (*iter).value );
                ADDELEM( scaledFloatStarHandler, XMLType( tostring( xml->unitscale ), &pImage->CockpitCenter.k ) );
                break;
            }
        }
        break;
    case DEFENSE:
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case HUDIMAGE:
                if ( (*iter).value.length() ) {
                    pImage->pHudImage = createVSSprite( (*iter).value.c_str() );
                    xml->hudimage     = (*iter).value;
                }
                break;
            case EXPLOSIONANI:
                if ( (*iter).value.length() ) {
                    pImage->explosion_type = (*iter).value;
                    {
                        cache_ani( pImage->explosion_type );
                    }
                }
                break;
            case REPAIRDROID:
                pImage->repair_droid = (unsigned char) parse_float( (*iter).value );
                break;
            case ECM:

                pImage->ecm = (int) ( ( (-1) > 1 )*parse_float( (*iter).value ) );                     //short fix
                pImage->ecm = pImage->ecm > 0 ? -pImage->ecm : pImage->ecm;
                break;
            default:
                break;
            }
        }
        break;
    case THRUST:
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        break;
    case ENERGY:
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case AFTERBURNENERGY:
                afterburnenergy = ( parse_float( (*iter).value ) );                   //short fix
                break;
            default:
                break;
            }
        }
        break;
    case RESTRICTED:
        ADDTAG;
        assert( xml->unitlevel == 1 );
        xml->unitlevel++;
        break;
    case UNKNOWN:
        ADDTAG;
    default:
        for (iter = attributes.begin(); iter != attributes.end(); iter++)
            ADDDEFAULT;
        xml->unitlevel++;
        break;
    }
}

#undef  ADDELEMF
#undef  ADDELEMI
#undef  ADDDEFAULT
#undef  ADDELEM
#undef  ADDELEMNAME
#undef  ADDTAG
#undef  ADDTAGNAME

void Unit::endElement( const string &name )
{
    using namespace UnitXML;
    pImage->unitwriter->EndTag( name );
    Names elem = (Names) element_map.lookup( name );
    switch (elem)
    {
    case UNKNOWN:
        xml->unitlevel--;
        break;
    default:
        xml->unitlevel--;
        break;
    }
}

using namespace VSFileSystem;

void Unit::LoadXML( const char *filename, const char *modifications, string *xmlbuffer )
{}

void Unit::LoadXML( VSFileSystem::VSFile &f, const char *modifications, string *xmlbuffer )
{
    shield.number = 0;
    string filename( f.GetFilename() );
    graphicOptions.RecurseIntoSubUnitsOnCollision = !isSubUnit();
    std::string collideTreeHash = VSFileSystem::GetHashName( string( modifications )+"#"+filename );
    pImage->unitwriter = new XMLSerializer( name.get().c_str(), modifications, this );
    pImage->unitwriter->AddTag( "Unit" );
    string *myhudim = &pImage->unitwriter->randomdata[0];
    float  *myscale = &pImage->unitscale;
    pImage->unitwriter->AddElement( "scale", floatStarHandler, XMLType( myscale ) );
    {
        pImage->unitwriter->AddTag( "Jump" );
        pImage->unitwriter->AddElement( "missing", lessNeg1Handler, XMLType( &jump.drive ) );
        pImage->unitwriter->AddElement( "warpDriveRating", floatStarHandler, XMLType( &jump.warpDriveRating ) );
        pImage->unitwriter->AddElement( "jumpenergy", floatStarHandler, XMLType( &jump.energy ) ); //short fix
        pImage->unitwriter->AddElement( "insysenergy", floatStarHandler, XMLType( &jump.insysenergy ) ); //short fix
        pImage->unitwriter->AddElement( "delay", delayucharStarHandler, XMLType( &jump.delay ) );
        pImage->unitwriter->AddElement( "damage", ucharStarHandler, XMLType( &jump.damage ) );
        pImage->unitwriter->AddElement( "wormhole", ucharStarHandler, XMLType( &pImage->forcejump ) );
        pImage->unitwriter->EndTag( "Jump" );
    }
    {
        unsigned int i;
        for (i = 0; i <= (UnitImages< void >::NUMGAUGES+MAXVDUS); i++) {
            pImage->unitwriter->AddTag( "CockpitDamage" );
            pImage->unitwriter->AddElement( "damage", floatStarHandler, XMLType( &pImage->cockpit_damage[i] ) );
            pImage->unitwriter->EndTag( "CockpitDamage" );
        }
    }
    {
        pImage->unitwriter->AddTag( "Defense" );
        pImage->unitwriter->AddElement( "HudImage", stringStarHandler, XMLType( myhudim ) );
        if ( pImage->explosion_type.get().length() )
            pImage->unitwriter->AddElement( "ExplosionAni", stringStarHandler, XMLType( &pImage->explosion_type ) );
        pImage->unitwriter->AddElement( "RepairDroid", ucharStarHandler, XMLType( &pImage->repair_droid ) );
        pImage->unitwriter->AddElement( "ECM", intToFloatHandler, XMLType( &pImage->ecm ) );         //short fix
        {
            pImage->unitwriter->AddTag( "Cloak" );
            pImage->unitwriter->AddElement( "missing", cloakHandler, XMLType( &cloaking ) );
            pImage->unitwriter->AddElement( "cloakmin", intToFloatHandler, XMLType( &cloakmin ) );             //short fix
            pImage->unitwriter->AddElement( "cloakglass", ucharStarHandler, XMLType( &pImage->cloakglass ) );
            pImage->unitwriter->AddElement( "cloakrate", intToFloatHandler, XMLType( &pImage->cloakrate ) );             //short fix
            pImage->unitwriter->AddElement( "cloakenergy", floatStarHandler, XMLType( &pImage->cloakenergy ) );
            pImage->unitwriter->EndTag( "Cloak" );
        }
        {
            pImage->unitwriter->AddTag( "Armor" );
            pImage->unitwriter->AddElement( "frontrighttop", floatStarHandler, XMLType( &armor.frontrighttop ) );             //short fix
            pImage->unitwriter->AddElement( "backrighttop", floatStarHandler, XMLType( &armor.backrighttop ) );             //short fix
            pImage->unitwriter->AddElement( "frontlefttop", floatStarHandler, XMLType( &armor.frontlefttop ) );             //short fix
            pImage->unitwriter->AddElement( "backlefttop", floatStarHandler, XMLType( &armor.backlefttop ) );             //short fix
            pImage->unitwriter->AddElement( "frontrightbottom", floatStarHandler, XMLType( &armor.frontrightbottom ) );             //short fix
            pImage->unitwriter->AddElement( "backrightbottom", floatStarHandler, XMLType( &armor.backrightbottom ) );             //short fix
            pImage->unitwriter->AddElement( "frontleftbottom", floatStarHandler, XMLType( &armor.frontleftbottom ) );             //short fix
            pImage->unitwriter->AddElement( "backleftbottom", floatStarHandler, XMLType( &armor.backleftbottom ) );             //short fix
            pImage->unitwriter->EndTag( "Armor" );
        }
        {
            pImage->unitwriter->AddTag( "Shields" );
            pImage->unitwriter->AddElement( "front", shieldSerializer, XMLType( (void*) &shield ) );
            pImage->unitwriter->AddElement( "recharge", floatStarHandler, XMLType( &shield.recharge ) );
            pImage->unitwriter->AddElement( "leak", charStarHandler, XMLType( &shield.leak ) );

            pImage->unitwriter->EndTag( "Shields" );
        }
        {
            pImage->unitwriter->AddTag( "Hull" );
            pImage->unitwriter->AddElement( "strength", floatStarHandler, XMLType( &hull ) );
            pImage->unitwriter->AddElement( "maximum", floatStarHandler, XMLType( &maxhull ) );
            pImage->unitwriter->EndTag( "Hull" );
        }

        pImage->unitwriter->EndTag( "Defense" );
    }
    {
        pImage->unitwriter->AddTag( "Energy" );
        pImage->unitwriter->AddElement( "afterburnenergy", floatStarHandler, XMLType( &afterburnenergy ) );         //short fix
        pImage->unitwriter->AddTag( "Reactor" );
        pImage->unitwriter->AddElement( "recharge", floatStarHandler, XMLType( &recharge ) );
        pImage->unitwriter->AddElement( "limit", floatStarHandler, XMLType( &maxenergy ) );
        pImage->unitwriter->AddElement( "warpenergy", floatStarHandler, XMLType( &maxwarpenergy ) );         //short fix
        pImage->unitwriter->EndTag( "Reactor" );

        pImage->unitwriter->EndTag( "Energy" );
    }
    {
        pImage->unitwriter->AddTag( "Stats" );
        pImage->unitwriter->AddElement( "mass", massSerializer, XMLType( &Mass ) );
        pImage->unitwriter->AddElement( "momentofinertia", floatStarHandler, XMLType( &Momentofinertia ) );
        pImage->unitwriter->AddElement( "fuel", floatStarHandler, XMLType( &fuel ) );
        pImage->unitwriter->EndTag( "Stats" );
        pImage->unitwriter->AddTag( "Thrust" );
        {
            pImage->unitwriter->AddTag( "Maneuver" );
            pImage->unitwriter->AddElement( "yaw", angleStarHandler, XMLType( &limits.yaw ) );
            pImage->unitwriter->AddElement( "pitch", angleStarHandler, XMLType( &limits.pitch ) );
            pImage->unitwriter->AddElement( "roll", angleStarHandler, XMLType( &limits.roll ) );
            pImage->unitwriter->EndTag( "Maneuver" );
        }
        {
            pImage->unitwriter->AddTag( "Engine" );
            pImage->unitwriter->AddElement( "forward", accelStarHandler, XMLType( &limits.forward ) );
            pImage->unitwriter->AddElement( "retro", accelStarHandler, XMLType( &limits.retro ) );
            pImage->unitwriter->AddElement( "left", accelStarHandler, XMLType( &limits.lateral ) );
            pImage->unitwriter->AddElement( "right", accelStarHandler, XMLType( &limits.lateral ) );
            pImage->unitwriter->AddElement( "top", accelStarHandler, XMLType( &limits.vertical ) );
            pImage->unitwriter->AddElement( "bottom", accelStarHandler, XMLType( &limits.vertical ) );
            pImage->unitwriter->AddElement( "afterburner", accelStarHandler, XMLType( &limits.afterburn ) );
            pImage->unitwriter->EndTag( "Engine" );
        }
        pImage->unitwriter->EndTag( "Thrust" );
    }
    pImage->CockpitCenter.Set( 0, 0, 0 );
    xml = new XML();
    xml->randomstartframe   = ( (float) rand() )/RAND_MAX;
    xml->randomstartseconds = 0;
    xml->calculated_role    = false;
    xml->damageiterator     = 0;
    xml->unitModifications  = modifications;
    xml->shieldmesh = NULL;
    xml->rapidmesh  = NULL;
    xml->hasColTree = true;
    xml->unitlevel  = 0;
    xml->unitscale  = 1;
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetUserData( parser, this );
    XML_SetElementHandler( parser, &Unit::beginElement, &Unit::endElement );
    if (xmlbuffer != NULL)
        XML_Parse( parser, xmlbuffer->c_str(), xmlbuffer->length(), 1 );
    else
        XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
    XML_ParserFree( parser );
    //Load meshes into subunit
    pImage->unitwriter->EndTag( "Unit" );
    meshdata   = xml->meshes;
    meshdata.push_back( NULL );
    corner_min = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
    corner_max = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );
    warpenergy = maxwarpenergy;
    *myhudim   = xml->hudimage;
    unsigned int a;
    if ( xml->mountz.size() ) {
        //DO not destroy anymore, just affect address
        for (a = 0; a < xml->mountz.size(); a++)
            mounts.push_back( *xml->mountz[a] );
    }
    unsigned char parity = 0;
    for (a = 0; a < xml->mountz.size(); a++) {
        static bool half_sounds = XMLSupport::parse_bool( vs_config->getVariable( "audio", "every_other_mount", "false" ) );
        if (a%2 == parity) {
            int b = a;
            if ( a%4 == 2 && (int) a < (getNumMounts()-1) )
                if (mounts[a].type->type != WEAPON_TYPE::PROJECTILE && mounts[a+1].type->type != WEAPON_TYPE::PROJECTILE)
                    b = a+1;
            mounts[b].sound = AUDCreateSound( mounts[b].type->sound, mounts[b].type->type != WEAPON_TYPE::PROJECTILE );
        } else if ( (!half_sounds) || mounts[a].type->type == WEAPON_TYPE::PROJECTILE ) {
            mounts[a].sound = AUDCreateSound( mounts[a].type->sound, mounts[a].type->type != WEAPON_TYPE::PROJECTILE ); //lloping also flase in unit_customize
        }
        if (a > 0)
            if (mounts[a].sound == mounts[a-1].sound && mounts[a].sound != -1) {
                BOOST_LOG_TRIVIAL(error) << "error";
            }
    }
    for (a = 0; a < xml->units.size(); a++)
        SubUnits.prepend( xml->units[a] );
    calculate_extent( false );
    pImage->unitscale = xml->unitscale;
    string tmpname( filename );
    vector< mesh_polygon >polies;
    this->colTrees = collideTrees::Get( collideTreeHash );
    if (this->colTrees)
        this->colTrees->Inc();
    csOPCODECollider *colShield = NULL;
    if (xml->shieldmesh) {
        meshdata.back() = xml->shieldmesh;
        if (!this->colTrees) {
            if ( meshdata.back() ) {
                meshdata.back()->GetPolys( polies );
                colShield = new csOPCODECollider( polies );
            }
        }
    } else {
        static int shieldstacks = XMLSupport::parse_int( vs_config->getVariable( "graphics", "shield_detail", "16" ) );
        static std::string shieldtex = vs_config->getVariable( "graphics", "shield_texture", "shield.bmp" );
        static std::string shieldtechnique = vs_config->getVariable( "graphics", "shield_technique", "" );
        meshdata.back() = new SphereMesh( rSize(), shieldstacks, shieldstacks, shieldtex.c_str(), shieldtechnique, NULL, false, ONE, ONE );
    }
    meshdata.back()->EnableSpecialFX();
    if (!this->colTrees) {
        polies.clear();
        if (xml->rapidmesh)
            xml->rapidmesh->GetPolys( polies );
        csOPCODECollider *csrc = NULL;
        if (xml->hasColTree)
            csrc = getCollideTree( Vector( 1, 1, 1 ),
                                   xml->rapidmesh
                                   ? &polies : NULL );
        this->colTrees = new collideTrees( collideTreeHash,
                                           csrc,
                                           colShield );
        if (xml->rapidmesh && xml->hasColTree)          //if we have a speciaal rapid mesh we need to generate things now
            for (unsigned int i = 1; i < collideTreesMaxTrees; ++i)
                if (!this->colTrees->rapidColliders[i]) {
                    unsigned int which = 1<<i;
                    this->colTrees->rapidColliders[i] = getCollideTree( Vector( which, which, which ),
                                                                        &polies );
                }
    }
    if (xml->rapidmesh)
        delete xml->rapidmesh;
    delete xml;
}



