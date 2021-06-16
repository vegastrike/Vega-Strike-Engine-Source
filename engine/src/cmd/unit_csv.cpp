// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * unit_csv.cpp
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


#include "unit_generic.h"
#include "csv.h"
#include "savegame.h"
#include "xml_serializer.h"
#include "gfx/sphere.h"
#include "unit_collide.h"
#include "collide2/Stdafx.h"
#include "collide2/CSopcodecollider.h"
#include "audiolib.h"
#include "unit_xml.h"
#include "gfx/quaternion.h"
#include "role_bitmask.h"
#include "unit_csv.h"
#include <algorithm>
#include "lin_time.h"
#include "unit_const_cache.h"
#include "vs_math.h"
#include "unit.h"
#include "universe.h"
#include "vsfilesystem.h"
#include "mount_size.h"
#include "weapon_info.h"

CSVRow LookupUnitRow( const string &unitname, const string &faction ) {
    string hashname = unitname+"__"+faction;
    for (vector< CSVTable* >::reverse_iterator i = unitTables.rbegin(); i != unitTables.rend(); ++i) {
        unsigned int where;
        if ( (*i)->RowExists( hashname, where ) )
            return CSVRow( (*i), where );
        else if ( (*i)->RowExists( unitname, where ) )
            return CSVRow( (*i), where );
    }
    return CSVRow();
}

extern int GetModeFromName( const char *input_buffer );
extern void pushMesh( std::vector< Mesh* >&mesh,
                      float &randomstartframe,
                      float &randomstartseconds,
                      const char *filename,
                      const float scale,
                      int faction,
                      class Flightgroup*fg,
                      int startframe,
                      double texturestarttime );
void addShieldMesh( Unit::XML*xml, const char *filename, const float scale, int faction, class Flightgroup*fg );
void addRapidMesh( Unit::XML*xml, const char *filename, const float scale, int faction, class Flightgroup*fg );

static void UpgradeUnit( Unit *un, const std::string &upgrades )
{
    string::size_type when;
    string::size_type ofs = 0;
    while ( ( when = upgrades.find( '{', ofs ) ) != string::npos ) {
        string::size_type where = upgrades.find( '}', when+1 );
        string upgrade = upgrades.substr( when+1, ( (where == string::npos) ? string::npos : where-when-1 ) );
        ofs = ( (where == string::npos) ? string::npos : where+1 );

        unsigned int mountoffset   = 0;
        unsigned int subunitoffset = 0;
        string::size_type where1   = upgrade.find( ';' );
        string::size_type where2   = upgrade.rfind( ';' );
        if (where1 != string::npos) {
            mountoffset = XMLSupport::parse_int( upgrade.substr( where1+1, where2 != where1 ? where2 : upgrade.length() ) );
            if (where2 != where1 && where2 != string::npos)
                subunitoffset = XMLSupport::parse_int( upgrade.substr( where2+1 ) );
        }
        upgrade = upgrade.substr( 0, where1 );
        if (upgrade.length() == 0)
            continue;
        const Unit *upgradee = UnitConstCache::getCachedConst( StringIntKey( upgrade, FactionUtil::GetUpgradeFaction() ) );
        if (!upgradee) {
            upgradee = UnitConstCache::setCachedConst( StringIntKey( upgrade, FactionUtil::GetUpgradeFaction() ),
                                                      new GameUnit( upgrade.c_str(),
                                                                              true,
                                                                              FactionUtil::GetUpgradeFaction() ) );
        }
        double percent = 1.0;
        un->Unit::Upgrade( upgradee,
                           mountoffset,
                           subunitoffset,
                           GetModeFromName( upgrade.c_str() ), true, percent, NULL );
    }
}

void AddMeshes( std::vector< Mesh* > &xmeshes,
                float &randomstartframe,
                float &randomstartseconds,
                float unitscale,
                const std::string &meshes,
                int faction,
                Flightgroup *fg,
                vector< unsigned int > *counts )
{
    string::size_type where, when, wheresf, wherest, ofs = 0;
    if (counts) counts->clear();
    {
        int nelem = 0;
        while ( ( ofs = meshes.find( '{', ofs ) ) != string::npos )
            nelem++, ofs++;
        if (counts) counts->reserve( nelem );
        xmeshes.reserve( nelem );
        ofs = 0;
    }
    while ( ( where = meshes.find( '{', ofs ) ) != string::npos ) {
        when    = meshes.find( '}', where+1 );         //matching closing brace
        string mesh = meshes.substr( where+1, ( (when == string::npos) ? string::npos : when-where-1 ) );
        ofs     = ( (when == string::npos) ? string::npos : when+1 );

        wheresf = mesh.find( ';' );
        string startf = "0";
        string startt = "0";
        if (wheresf != string::npos) {
            startf  = mesh.substr( wheresf+1 );
            mesh    = mesh.substr( 0, wheresf );
            wherest = startf.find( ';' );
            if (wherest != string::npos) {
                startt = startf.substr( wherest+1 );
                startf = startf.substr( 0, wherest );
            }
        }
        int   startframe = startf == "RANDOM" ? -1 : ( startf == "ASYNC" ? -1 : atoi( startf.c_str() ) );
        float starttime  = startt == "RANDOM" ? -1.0f : atof( startt.c_str() );
        unsigned int s   = xmeshes.size();
        pushMesh( xmeshes, randomstartframe, randomstartseconds, mesh.c_str(), unitscale, faction, fg, startframe, starttime );
        if (counts) counts->push_back( xmeshes.size()-s );
    }
}

static std::pair< string::size_type, string::size_type >nextElementRange( const string &inp,
                                                                     string::size_type &start,
                                                                     string::size_type end )
{
    string::size_type ostart = start;
    start = inp.find( ';', start );
    if ( start != string::npos && ( start != end && (end == string::npos || start < end) ) ) {
        ++start;
        return std::pair< string::size_type, string::size_type > ( ostart, start-1 );
    } else {
        start = end;
        return std::pair< string::size_type, string::size_type > ( ostart, end );
    }
}

static string nextElementString( const string &inp, string::size_type &start, string::size_type end )
{
    std::pair< string::size_type, string::size_type >rng = nextElementRange( inp, start, end );
    if (rng.second == string::npos)
        return inp.substr( rng.first );
    else
        return inp.substr( rng.first, rng.second-rng.first );
}

static int nextElementInt( const string &inp, string::size_type &start, string::size_type end, int def = 0 )
{
    std::pair< string::size_type, string::size_type >rng = nextElementRange( inp, start, end );
    if (rng.second == string::npos && rng.first >= inp.length())
        return def;
    else if (rng.first == rng.second)
        return def;
    else
        return atoi( inp.c_str()+rng.first );
}

static double nextElementFloat( const string &inp, string::size_type &start, string::size_type end, double def = 0 )
{
    std::pair< string::size_type, string::size_type >rng = nextElementRange( inp, start, end );
    if (rng.second == string::npos && rng.first >= inp.length())
        return def;
    else if (rng.first == rng.second)
        return def;
    else
        return atof( inp.c_str()+rng.first );
}

static double nextElementBool( const string &inp, string::size_type &start, string::size_type end, bool def = false )
{
    std::pair< string::size_type, string::size_type >rng = nextElementRange( inp, start, end );
    if (rng.second == string::npos && rng.first >= inp.length())
        return def;
    else if (rng.first == rng.second)
        return def;
    else if (rng.second == string::npos)
        return XMLSupport::parse_bool( inp.substr( rng.first ) );
    else
        return XMLSupport::parse_bool( inp.substr( rng.first, rng.second-rng.first ) );
}

static string nextElement( string &inp )
{
    string::size_type start = 0;
    std::pair< string::size_type, string::size_type >rng = nextElementRange( inp, start, string::npos );
    string ret = inp.substr( rng.first, ( (rng.second == string::npos) ? string::npos : (rng.second-rng.first) ) );
    inp.erase( 0, ( (rng.second == string::npos) ? string::npos : (rng.second+1) ) );
    return ret;
}

static bool stob( const string &inp, bool defaul )
{
    if (inp.length() != 0)
        return XMLSupport::parse_bool( inp );
    return defaul;
}

static double stof( const string &inp, double def = 0 )
{
    if (inp.length() != 0)
        return XMLSupport::parse_float( inp );
    return def;
}

static int stoi( const string &inp, int def = 0 )
{
    if (inp.length() != 0)
        return XMLSupport::parse_int( inp );
    return def;
}

extern bool CheckAccessory( Unit* );

extern int parseMountSizes( const char *str );

static void AddMounts( Unit *thus, Unit::XML &xml, const std::string &mounts )
{
    string::size_type where, when, ofs = 0;
    unsigned int first_new_mount = thus->mounts.size();
    {
        int nmountz = 0;
        while ( ( ofs = mounts.find( '{', ofs ) ) != string::npos )
            nmountz++, ofs++;
        thus->mounts.reserve( nmountz+thus->mounts.size() );
        ofs = 0;
    }
    while ( ( where = mounts.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = mounts.find( '}', where+1 ) ) != string::npos ) {
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            QVector P;
            QVector Q = QVector( 0, 1, 0 );
            QVector R = QVector( 0, 0, 1 );
            QVector pos = QVector( 0, 0, 0 );

            string  filename  = nextElementString( mounts, elemstart, elemend );
            int     ammo      = nextElementInt( mounts, elemstart, elemend, -1 );
            int     volume    = nextElementInt( mounts, elemstart, elemend );
            string  mountsize = nextElementString( mounts, elemstart, elemend );
            pos.i = nextElementFloat( mounts, elemstart, elemend );
            pos.j = nextElementFloat( mounts, elemstart, elemend );
            pos.k = nextElementFloat( mounts, elemstart, elemend );
            double xyscale = nextElementFloat( mounts, elemstart, elemend );
            double zscale  = nextElementFloat( mounts, elemstart, elemend );
            R.i   = nextElementFloat( mounts, elemstart, elemend );
            R.j   = nextElementFloat( mounts, elemstart, elemend );
            R.k   = nextElementFloat( mounts, elemstart, elemend, 1 );
            Q.i   = nextElementFloat( mounts, elemstart, elemend );
            Q.j   = nextElementFloat( mounts, elemstart, elemend, 1 );
            Q.k   = nextElementFloat( mounts, elemstart, elemend );
            float func    = nextElementFloat( mounts, elemstart, elemend, 1 );
            float maxfunc = nextElementFloat( mounts, elemstart, elemend, 1 );
            bool  banked  = nextElementBool( mounts, elemstart, elemend, false );
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
            Mount mnt( filename, ammo, volume, xml.unitscale*xyscale, xml.unitscale*zscale, func, maxfunc, banked );
            mnt.SetMountOrientation( Quaternion::from_vectors( P.Cast(), Q.Cast(), R.Cast() ) );
            mnt.SetMountPosition( xml.unitscale*pos.Cast() );
            int   mntsiz = as_integer(MOUNT_SIZE::NOWEAP);
            if ( mountsize.length() ) {
                mntsiz   = parseMountSizes( mountsize.c_str() );
                mnt.size = mntsiz;
            } else {
                mnt.size = as_integer(mnt.type->size);
            }
            thus->mounts.push_back( mnt );
        } else {ofs = string::npos; }}
    unsigned char parity = 0;
    for (unsigned int a = first_new_mount; a < thus->mounts.size(); ++a) {
        static bool half_sounds = XMLSupport::parse_bool( vs_config->getVariable( "audio", "every_other_mount", "false" ) );
        if ( (a&1) == parity ) {
            int b = a;
            if ( (a&3) == 2 && (int) a < (thus->getNumMounts()-1) ) {
                if (thus->mounts[a].type->type != WEAPON_TYPE::PROJECTILE
                    && thus->mounts[a+1].type->type != WEAPON_TYPE::PROJECTILE)
                {
                    b = a+1;
                }
            }
            thus->mounts[b].sound = AUDCreateSound( thus->mounts[b].type->sound, false );
        } else if ( (!half_sounds) || thus->mounts[a].type->type == WEAPON_TYPE::PROJECTILE ) {
            thus->mounts[a].sound = AUDCreateSound( thus->mounts[a].type->sound, false );
        }
        if (a > 0)
            if (thus->mounts[a].sound == thus->mounts[a-1].sound && thus->mounts[a].sound != -1) {
                BOOST_LOG_TRIVIAL(error) << "Sound error";
            }
    }
}

struct SubUnitStruct
{
    string  filename;
    QVector pos;
    QVector Q;
    QVector R;
    double  restricted;
    SubUnitStruct( string fn, QVector POS, QVector q, QVector r, double res )
    {
        filename = fn;
        pos = POS;
        Q   = q;
        R   = r;
        restricted = res;
    }
};

static vector< SubUnitStruct >GetSubUnits( const std::string &subunits )
{
    string::size_type where, when, ofs = 0;
    vector< SubUnitStruct >ret;
    {
        int nelem = 0;
        while ( ( ofs = subunits.find( '{', ofs ) ) != string::npos )
            nelem++, ofs++;
        ret.reserve( nelem );
        ofs = 0;
    }
    while ( ( where = subunits.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = subunits.find( '}', ofs ) ) != string::npos ) {
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            QVector pos, Q, R;

            string  filename = nextElementString( subunits, elemstart, elemend );
            pos.i = nextElementFloat( subunits, elemstart, elemend );
            pos.j = nextElementFloat( subunits, elemstart, elemend );
            pos.k = nextElementFloat( subunits, elemstart, elemend );
            R.i   = nextElementFloat( subunits, elemstart, elemend );
            R.j   = nextElementFloat( subunits, elemstart, elemend );
            R.k   = nextElementFloat( subunits, elemstart, elemend );
            Q.i   = nextElementFloat( subunits, elemstart, elemend );
            Q.j   = nextElementFloat( subunits, elemstart, elemend );
            Q.k   = nextElementFloat( subunits, elemstart, elemend );
            double restricted = cos( nextElementFloat( subunits, elemstart, elemend, 180 )*VS_PI/180.0 );

            ret.push_back( SubUnitStruct( filename, pos, Q, R, restricted ) );
        } else {
            ofs = string::npos;
        }
    }
    return ret;
}

static void AddSubUnits( Unit *thus, Unit::XML &xml, const std::string &subunits, int faction, const std::string &modification )
{
    vector< SubUnitStruct >su = GetSubUnits( subunits );
    xml.units.reserve( subunits.size()+xml.units.size() );
    for (vector< SubUnitStruct >::iterator i = su.begin(); i != su.end(); ++i) {
        string  filename = (*i).filename;
        QVector pos = (*i).pos;
        QVector Q   = (*i).Q;
        QVector R   = (*i).R;
        double  restricted = (*i).restricted;
        xml.units.push_back( new GameUnit( filename.c_str(), true, faction, modification, NULL ) );         //I set here the fg arg to NULL
        if (xml.units.back()->name == "LOAD_FAILED") {
            xml.units.back()->limits.yaw = 0;
            xml.units.back()->limits.pitch = 0;
            xml.units.back()->limits.roll = 0;
            xml.units.back()->limits.lateral = xml.units.back()->limits.retro = xml.units.back()->limits.forward =
                                                                                    xml.units.back()->limits.afterburn = 0.0;
        }
        if ( !thus->isSubUnit() )         //Useless to set recursive owner in subunits - as parent will do the same
            xml.units.back()->SetRecursiveOwner( thus );
        xml.units.back()->SetOrientation( Q, R );
        R.Normalize();
        xml.units.back()->prev_physical_state = xml.units.back()->curr_physical_state;
        xml.units.back()->SetPosition( pos*xml.unitscale );
        xml.units.back()->limits.structurelimits = R.Cast();
        xml.units.back()->limits.limitmin = restricted;
        xml.units.back()->name = filename;
        if (xml.units.back()->pImage->unitwriter != NULL)
            xml.units.back()->pImage->unitwriter->setName( filename );
        CheckAccessory( xml.units.back() );         //turns on the ceerazy rotation for the turr
    }
    for (int a = xml.units.size()-1; a >= 0; a--) {
        bool randomspawn = xml.units[a]->name.get().find( "randomspawn" ) != string::npos;
        if (randomspawn) {
            int chancetospawn = float_to_int( xml.units[a]->warpCapData() );
            if (chancetospawn > rand()%100)
                thus->SubUnits.prepend( xml.units[a] );

            else
                xml.units[a]->Kill();
        } else {
            thus->SubUnits.prepend( xml.units[a] );
        }
    }
}

void AddDocks( Unit *thus, Unit::XML &xml, const string &docks )
{
    string::size_type where, when;
    string::size_type ofs = 0;
    int overlap = 1;

    // TODO: Someone should check blame here. It doesn't look right.
    {
        int nelem = 0;
        while ( ( ofs = docks.find( '{', ofs ) ) != string::npos )
            nelem++, ofs++;
        thus->pImage->dockingports.reserve( nelem*overlap+thus->pImage->dockingports.size() );
        ofs = 0;
    }
    while ( ( where = docks.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = docks.find( '}', where+1 ) ) != string::npos ) {
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            QVector pos = QVector( 0, 0, 0 );
            int type = nextElementInt( docks, elemstart, elemend );
            pos.i = nextElementFloat( docks, elemstart, elemend );
            pos.j = nextElementFloat( docks, elemstart, elemend );
            pos.k = nextElementFloat( docks, elemstart, elemend );
            double size    = nextElementFloat( docks, elemstart, elemend );
            double minsize = nextElementFloat( docks, elemstart, elemend );
            for (int i = 0; i < overlap; i++)
                thus->pImage->dockingports.push_back( DockingPorts( pos.Cast()*xml.unitscale, size*xml.unitscale, minsize
                                                                    *xml.unitscale, DockingPorts::Type::Value(type) ) );
        } else {
            ofs = string::npos;
        }
    }
}

void AddLights( Unit *thus, Unit::XML &xml, const string &lights )
{
    static float default_halo_activation =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "default_engine_activation", ".00048828125" ) );
    string::size_type where, when;
    string::size_type ofs = 0;
    while ( ( where = lights.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = lights.find( '}', where+1 ) ) != string::npos ) {
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            string   filename = nextElementString( lights, elemstart, elemend );
            QVector  pos, scale;
            GFXColor halocolor;
            QVector P(1, 0, 0), Q( 0, 1, 0 ), R( 0, 0, 1 );
            pos.i = nextElementFloat( lights, elemstart, elemend );
            pos.j = nextElementFloat( lights, elemstart, elemend );
            pos.k = nextElementFloat( lights, elemstart, elemend );
            scale.i     = xml.unitscale*nextElementFloat( lights, elemstart, elemend, 1 );
            scale.j     = scale.k = scale.i;
            halocolor.r = nextElementFloat( lights, elemstart, elemend, 1 );
            halocolor.g = nextElementFloat( lights, elemstart, elemend, 1 );
            halocolor.b = nextElementFloat( lights, elemstart, elemend, 1 );
            halocolor.a = nextElementFloat( lights, elemstart, elemend, 1 );
            double act_speed = nextElementFloat( lights, elemstart, elemend, default_halo_activation );
            R.i   = nextElementFloat( lights, elemstart, elemend );
            R.j   = nextElementFloat( lights, elemstart, elemend );
            R.k   = nextElementFloat( lights, elemstart, elemend, 1 );
            Q.i   = nextElementFloat( lights, elemstart, elemend );
            Q.j   = nextElementFloat( lights, elemstart, elemend, 1 );
            Q.k   = nextElementFloat( lights, elemstart, elemend );

            Q.Normalize();
            if ( fabs( Q.i ) == fabs( R.i ) && fabs( Q.j ) == fabs( R.j ) && fabs( Q.k ) == fabs( R.k ) ) {
                Q = QVector(-1, 0, 0);
            }
            R.Normalize();
            CrossProduct( Q, R, P );
            CrossProduct( R, P, Q );
            Q.Normalize();
            Matrix trans(P.Cast(), Q.Cast(), R.Cast(), pos*xml.unitscale);

            thus->addHalo( filename.c_str(), trans, scale.Cast(), halocolor, "", act_speed );
        } else {
            ofs = string::npos;
        }
    }
}

static void ImportCargo( Unit *thus, const string &imports )
{
    string::size_type where, when, ofs = 0;
    {
        int nelem = 0;
        while ( ( ofs = imports.find( '{', ofs ) ) != string::npos )
            nelem++, ofs++;
        thus->cargo.reserve( nelem+thus->cargo.size() );
        ofs = 0;
    }
    while ( ( where = imports.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = imports.find( '}', where+1 ) ) != string::npos ) {
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            string filename    = nextElementString( imports, elemstart, elemend );
            double price       = nextElementFloat( imports, elemstart, elemend, 1 );
            double pricestddev = nextElementFloat( imports, elemstart, elemend );
            double quant       = nextElementFloat( imports, elemstart, elemend, 1 );
            double quantstddev = nextElementFloat( imports, elemstart, elemend );

            thus->ImportPartList( filename, price, pricestddev, quant, quantstddev );
        } else {
            ofs = string::npos;
        }
    }
}

static void AddCarg( Unit *thus, const string &cargos )
{
    string::size_type where, when, ofs = 0;
    {
        int nelem = 0;
        while ( ( ofs = cargos.find( '{', ofs ) ) != string::npos )
            nelem++, ofs++;
        thus->cargo.reserve( nelem+thus->cargo.size() );
        ofs = 0;
    }
    while ( ( where = cargos.find( '{', ofs ) ) != string::npos ) {
        if ( ( when = cargos.find( '}', where+1 ) ) != string::npos ) {
            Cargo carg;
            string::size_type elemstart = where+1, elemend = when;
            ofs = when+1;

            carg.content = nextElementString( cargos, elemstart, elemend );
            carg.category = nextElementString( cargos, elemstart, elemend );
            carg.price = nextElementFloat( cargos, elemstart, elemend );
            carg.quantity = nextElementInt( cargos, elemstart, elemend );
            carg.mass = nextElementFloat( cargos, elemstart, elemend );
            carg.volume = nextElementFloat( cargos, elemstart, elemend );
            carg.functionality    = nextElementFloat( cargos, elemstart, elemend, 1.f );
            carg.maxfunctionality = nextElementFloat( cargos, elemstart, elemend, 1.f );
            carg.description      = nextElementString( cargos, elemstart, elemend );
            carg.mission          = nextElementBool( cargos, elemstart, elemend, false );
            carg.installed        = nextElementBool( cargos, elemstart, elemend,
                carg.category.get().find("upgrades/") == 0 );

            thus->AddCargo( carg, false );
        } else {
            ofs = string::npos;
        }
    }
}

void HudDamage( float *dam, const string &damages )
{
    if (dam) {
        string::size_type elemstart = 0, elemend = string::npos;
        for (int i = 0; i < 1+MAXVDUS+UnitImages< void >::NUMGAUGES; ++i)
            dam[i] = nextElementFloat( damages, elemstart, elemend, 1 );
    }
}

string WriteHudDamage( Unit *un )
{
    string ret;
    const string semi = ";";
    if (un->pImage->cockpit_damage) {
        for (int i = 0; i < 1+MAXVDUS+UnitImages< void >::NUMGAUGES; ++i) {
            ret += XMLSupport::tostring( un->pImage->cockpit_damage[i] );
            ret += semi;
        }
    }
    return ret;
}

string WriteHudDamageFunc( Unit *un )
{
    string ret;
    const string semi = ";";
    if (un->pImage->cockpit_damage) {
        int numg = 1+MAXVDUS+UnitImages< void >::NUMGAUGES;
        for (int i = numg; i < 2*numg; ++i) {
            ret += XMLSupport::tostring( un->pImage->cockpit_damage[i] );
            ret += semi;
        }
    }
    return ret;
}



void LoadCockpit( Unit *thus, const string &cockpit )
{
    string::size_type elemstart = 0, elemend = string::npos;
    thus->pImage->cockpitImage    = nextElementString( cockpit, elemstart, elemend );
    thus->pImage->CockpitCenter.i = nextElementFloat( cockpit, elemstart, elemend );
    thus->pImage->CockpitCenter.j = nextElementFloat( cockpit, elemstart, elemend );
    thus->pImage->CockpitCenter.k = nextElementFloat( cockpit, elemstart, elemend );
}

static int AssignIf( const string &inp, float &val, float &val1, float &val2 )
{
    if ( inp.length() ) {
        val  = ::stof( inp );
        val1 = ::stof( inp );
        val2 = ::stof( inp );
        return 1;
    }
    return 0;
}

static int AssignIfDeg( const string &inp, float &val )
{
    if ( inp.length() ) {
        val = ::stof( inp )*VS_PI/180;
        return 1;
    }
    return 0;
}

float getFuelConversion()
{
    static float fuel_conversion = XMLSupport::parse_float( vs_config->getVariable( "physics", "FuelConversion", ".00144" ) );
    return fuel_conversion;
}

const std::string EMPTY_STRING( "" );

#define LOADROW_OPTIMIZER ((0x348299ab))

 /*After all, it's always used in the end*/
#define FORCE_OPTIMIZER (1)

#define OPTIMIZER_INDEX( Variable ) OPTIDX_##Variable

#define INIT_OPTIMIZER( keys, Variable ) do{OPTIMIZER_INDEX(Variable)=(keys.push_back(#Variable),(keys.size()-1));}while(0)

#define DEF_OPTIMIZER( Variable ) static unsigned int OPTIMIZER_INDEX( Variable ) = CSVTable::optimizer_undefined;

#if FORCE_OPTIMIZER

#define OPTIM_GET_DEF( row, table, variable, deflt )                                           \
    ((                                                                                         \
        (table->optimizer_indexes[OPTIMIZER_INDEX(variable)] == CSVTable::optimizer_undefined) \
        ? (deflt)                                                                              \
        : row[ table->optimizer_indexes[OPTIMIZER_INDEX(variable)] ]                           \
    ))

#else

#define OPTIM_GET( row, table, variable, deflt )                             \
    ((                                                                       \
        use_optimizer                                                        \
        ? (                                                                  \
            (                                                                \
                (OPTIMIZER_INDEX(variable) == CSVTable::optimizer_undefined) \
                || (                                                         \
                    table->optimizer_indexes[OPTIMIZER_INDEX(variable)]      \
                    == CSVTable::optimizer_undefined                         \
                   )                                                         \
            )                                                                \
            ? (deflt)                                                        \
            : row[table->optimizer_indexes[OPTIMIZER_INDEX(variable )]]      \
        )                                                                    \
        : row[#variable]                                                     \
    ))

#endif

#define OPTIM_GET( rot, table, variable ) OPTIM_GET_DEF( row, table, variable, EMPTY_STRING )

void Unit::LoadRow( CSVRow &row, string modification, string *netxml )
{
    CSVTable *table = row.getParent();
    Unit::XML xml;
    xml.unitModifications  = modification.c_str();
    xml.randomstartframe   = ( (float) rand() )/RAND_MAX;
    xml.randomstartseconds = 0;
    xml.calculated_role    = false;
    xml.damageiterator     = 0;
    xml.shieldmesh = NULL;
    xml.rapidmesh  = NULL;
    xml.hasColTree = true;
    xml.unitlevel  = 0;
    xml.unitscale  = 1;
    xml.data = xml.shieldmesh = xml.rapidmesh = NULL;     //was uninitialized memory
    string tmpstr;
    csvRow   = row[0];
    DEF_OPTIMIZER( FaceCamera );
    DEF_OPTIMIZER( Name );
    DEF_OPTIMIZER( Hud_image );
    DEF_OPTIMIZER( Combat_Role );     //legacy only
    DEF_OPTIMIZER( Unit_Role );
    DEF_OPTIMIZER( Attack_Preference );
    DEF_OPTIMIZER( Num_Animation_Stages );
    DEF_OPTIMIZER( Unit_Scale );
    DEF_OPTIMIZER( Mesh );
    DEF_OPTIMIZER( Dock );
    DEF_OPTIMIZER( Sub_Units );
    DEF_OPTIMIZER( Mounts );
    DEF_OPTIMIZER( Hold_Volume );
    DEF_OPTIMIZER( Hidden_Hold_Volume );
    DEF_OPTIMIZER( Upgrade_Storage_Volume );
    DEF_OPTIMIZER( Equipment_Space );
    DEF_OPTIMIZER( Cargo_Import );
    DEF_OPTIMIZER( Cargo );
    DEF_OPTIMIZER( Sounds );
    DEF_OPTIMIZER( Cockpit );
    DEF_OPTIMIZER( CockpitX );
    DEF_OPTIMIZER( CockpitY );
    DEF_OPTIMIZER( CockpitZ );
    DEF_OPTIMIZER( Mass );
    DEF_OPTIMIZER( Moment_Of_Inertia );
    DEF_OPTIMIZER( Fuel_Capacity );
    DEF_OPTIMIZER( Hull );
    DEF_OPTIMIZER( Armor_Front_Top_Left );
    DEF_OPTIMIZER( Armor_Front_Top_Right );
    DEF_OPTIMIZER( Armor_Back_Top_Left );
    DEF_OPTIMIZER( Armor_Back_Top_Right );
    DEF_OPTIMIZER( Armor_Front_Bottom_Left );
    DEF_OPTIMIZER( Armor_Front_Bottom_Right );
    DEF_OPTIMIZER( Armor_Back_Bottom_Left );
    DEF_OPTIMIZER( Armor_Back_Bottom_Right );
//    DEF_OPTIMIZER( Description );  // What was this supposed to describe? - compiler complained that it's not used.
    DEF_OPTIMIZER( Shield_Front_Top_Left );
    DEF_OPTIMIZER( Shield_Front_Top_Right );
    DEF_OPTIMIZER( Shield_Back_Top_Left );
    DEF_OPTIMIZER( Shield_Back_Top_Right );
    DEF_OPTIMIZER( Shield_Front_Bottom_Left );
    DEF_OPTIMIZER( Shield_Front_Bottom_Right );
    DEF_OPTIMIZER( Shield_Back_Bottom_Left );
    DEF_OPTIMIZER( Shield_Back_Bottom_Right );
    DEF_OPTIMIZER( Shield_Leak );
    DEF_OPTIMIZER( Shield_Recharge );
    DEF_OPTIMIZER( Shield_Efficiency );
    DEF_OPTIMIZER( Warp_Capacitor );
    DEF_OPTIMIZER( Warp_Min_Multiplier );
    DEF_OPTIMIZER( Warp_Max_Multiplier );
    DEF_OPTIMIZER( Primary_Capacitor );
    DEF_OPTIMIZER( Reactor_Recharge );
    DEF_OPTIMIZER( Jump_Drive_Present );
    DEF_OPTIMIZER( Jump_Drive_Delay );
    DEF_OPTIMIZER( Wormhole );
    DEF_OPTIMIZER( Collide_Subunits );
    DEF_OPTIMIZER( Outsystem_Jump_Cost );
    DEF_OPTIMIZER( Warp_Usage_Cost );
    DEF_OPTIMIZER( Afterburner_Usage_Cost );
    DEF_OPTIMIZER( Afterburner_Type );
    DEF_OPTIMIZER( Maneuver_Yaw );
    DEF_OPTIMIZER( Maneuver_Pitch );
    DEF_OPTIMIZER( Maneuver_Roll );
    DEF_OPTIMIZER( Yaw_Governor );
    DEF_OPTIMIZER( Yaw_Governor_Right );
    DEF_OPTIMIZER( Yaw_Governor_Left );
    DEF_OPTIMIZER( Pitch_Governor );
    DEF_OPTIMIZER( Pitch_Governor_Up );
    DEF_OPTIMIZER( Pitch_Governor_Down );
    DEF_OPTIMIZER( Roll_Governor );
    DEF_OPTIMIZER( Roll_Governor_Right );
    DEF_OPTIMIZER( Roll_Governor_Left );
    DEF_OPTIMIZER( Afterburner_Accel );
    DEF_OPTIMIZER( Forward_Accel );
    DEF_OPTIMIZER( Retro_Accel );
    DEF_OPTIMIZER( Left_Accel );
    DEF_OPTIMIZER( Right_Accel );
    DEF_OPTIMIZER( Top_Accel );
    DEF_OPTIMIZER( Bottom_Accel );
    DEF_OPTIMIZER( Default_Speed_Governor );
    DEF_OPTIMIZER( Afterburner_Speed_Governor );
    DEF_OPTIMIZER( ITTS );
    DEF_OPTIMIZER( Can_Lock );
    DEF_OPTIMIZER( Radar_Color );
    DEF_OPTIMIZER( Radar_Range );
    DEF_OPTIMIZER( Max_Cone );
    DEF_OPTIMIZER( Tracking_Cone );
    DEF_OPTIMIZER( Lock_Cone );
    DEF_OPTIMIZER( Cloak_Min );
    DEF_OPTIMIZER( Cloak_Glass );
    DEF_OPTIMIZER( Can_Cloak );
    DEF_OPTIMIZER( Cloak_Rate );
    DEF_OPTIMIZER( Cloak_Energy );
    DEF_OPTIMIZER( Repair_Droid );
    DEF_OPTIMIZER( ECM_Rating );
    DEF_OPTIMIZER( Heat_Sink_Rating );
    DEF_OPTIMIZER( Hud_Functionality );
    DEF_OPTIMIZER( Max_Hud_Functionality );
    DEF_OPTIMIZER( Lifesupport_Functionality );
    DEF_OPTIMIZER( Max_Lifesupport_Functionality );
    DEF_OPTIMIZER( Comm_Functionality );
    DEF_OPTIMIZER( Max_Comm_Functionality );
    DEF_OPTIMIZER( FireControl_Functionality );
    DEF_OPTIMIZER( Max_FireControl_Functionality );
    DEF_OPTIMIZER( SPECDrive_Functionality );
    DEF_OPTIMIZER( Max_SPECDrive_Functionality );
    DEF_OPTIMIZER( Slide_Start );
    DEF_OPTIMIZER( Slide_End );
    DEF_OPTIMIZER( Upgrades );
    DEF_OPTIMIZER( Tractorability );
    DEF_OPTIMIZER( Explosion );
    DEF_OPTIMIZER( Light );
    DEF_OPTIMIZER( Shield_Mesh );
    DEF_OPTIMIZER( Rapid_Mesh );
    DEF_OPTIMIZER( Use_Rapid );
    DEF_OPTIMIZER( NoDamageParticles );
    DEF_OPTIMIZER( Spec_Interdiction );
    if (table && !table->optimizer_setup) {
        static std::vector< std::string >keys;
        static bool optimizer_keys_init = false;
        if (!optimizer_keys_init) {
            optimizer_keys_init = true;
            BOOST_LOG_TRIVIAL(info) << "Initializing optimizer";
            INIT_OPTIMIZER( keys, Name );
            INIT_OPTIMIZER( keys, Hud_image );
            INIT_OPTIMIZER( keys, FaceCamera );
            INIT_OPTIMIZER( keys, Combat_Role );             //legacy only
            INIT_OPTIMIZER( keys, Unit_Role );
            INIT_OPTIMIZER( keys, Attack_Preference );
            INIT_OPTIMIZER( keys, Num_Animation_Stages );
            INIT_OPTIMIZER( keys, Unit_Scale );
            INIT_OPTIMIZER( keys, Mesh );
            INIT_OPTIMIZER( keys, Dock );
            INIT_OPTIMIZER( keys, Sub_Units );
            INIT_OPTIMIZER( keys, Mounts );
            INIT_OPTIMIZER( keys, Hold_Volume );
            INIT_OPTIMIZER( keys, Hidden_Hold_Volume );
            INIT_OPTIMIZER( keys, Upgrade_Storage_Volume );
            INIT_OPTIMIZER( keys, Equipment_Space );
            INIT_OPTIMIZER( keys, Cargo_Import );
            INIT_OPTIMIZER( keys, Cargo );
            INIT_OPTIMIZER( keys, Sounds );
            INIT_OPTIMIZER( keys, Cockpit );
            INIT_OPTIMIZER( keys, CockpitX );
            INIT_OPTIMIZER( keys, CockpitY );
            INIT_OPTIMIZER( keys, CockpitZ );
            INIT_OPTIMIZER( keys, Mass );
            INIT_OPTIMIZER( keys, Moment_Of_Inertia );
            INIT_OPTIMIZER( keys, Fuel_Capacity );
            INIT_OPTIMIZER( keys, Hull );
            INIT_OPTIMIZER( keys, Armor_Front_Top_Left );
            INIT_OPTIMIZER( keys, Armor_Front_Top_Right );
            INIT_OPTIMIZER( keys, Armor_Back_Top_Left );
            INIT_OPTIMIZER( keys, Armor_Back_Top_Right );
            INIT_OPTIMIZER( keys, Armor_Front_Bottom_Left );
            INIT_OPTIMIZER( keys, Armor_Front_Bottom_Right );
            INIT_OPTIMIZER( keys, Armor_Back_Bottom_Left );
            INIT_OPTIMIZER( keys, Armor_Back_Bottom_Right );
//            INIT_OPTIMIZER( keys, Description );      // Compiler complained that this is not used
            INIT_OPTIMIZER( keys, Shield_Front_Top_Left );
            INIT_OPTIMIZER( keys, Shield_Front_Top_Right );
            INIT_OPTIMIZER( keys, Shield_Back_Top_Left );
            INIT_OPTIMIZER( keys, Shield_Back_Top_Right );
            INIT_OPTIMIZER( keys, Shield_Front_Bottom_Left );
            INIT_OPTIMIZER( keys, Shield_Front_Bottom_Right );
            INIT_OPTIMIZER( keys, Shield_Back_Bottom_Left );
            INIT_OPTIMIZER( keys, Shield_Back_Bottom_Right );
            INIT_OPTIMIZER( keys, Shield_Leak );
            INIT_OPTIMIZER( keys, Shield_Recharge );
            INIT_OPTIMIZER( keys, Shield_Efficiency );
            INIT_OPTIMIZER( keys, Warp_Capacitor );
            INIT_OPTIMIZER( keys, Warp_Min_Multiplier );
            INIT_OPTIMIZER( keys, Warp_Max_Multiplier );
            INIT_OPTIMIZER( keys, Primary_Capacitor );
            INIT_OPTIMIZER( keys, Reactor_Recharge );
            INIT_OPTIMIZER( keys, Jump_Drive_Present );
            INIT_OPTIMIZER( keys, Jump_Drive_Delay );
            INIT_OPTIMIZER( keys, Wormhole );
            INIT_OPTIMIZER( keys, Collide_Subunits );
            INIT_OPTIMIZER( keys, Outsystem_Jump_Cost );
            INIT_OPTIMIZER( keys, Warp_Usage_Cost );
            INIT_OPTIMIZER( keys, Afterburner_Usage_Cost );
            INIT_OPTIMIZER( keys, Afterburner_Type );
            INIT_OPTIMIZER( keys, Maneuver_Yaw );
            INIT_OPTIMIZER( keys, Maneuver_Pitch );
            INIT_OPTIMIZER( keys, Maneuver_Roll );
            INIT_OPTIMIZER( keys, Yaw_Governor );
            INIT_OPTIMIZER( keys, Yaw_Governor_Right );
            INIT_OPTIMIZER( keys, Yaw_Governor_Left );
            INIT_OPTIMIZER( keys, Pitch_Governor );
            INIT_OPTIMIZER( keys, Pitch_Governor_Up );
            INIT_OPTIMIZER( keys, Pitch_Governor_Down );
            INIT_OPTIMIZER( keys, Roll_Governor );
            INIT_OPTIMIZER( keys, Roll_Governor_Right );
            INIT_OPTIMIZER( keys, Roll_Governor_Left );
            INIT_OPTIMIZER( keys, Afterburner_Accel );
            INIT_OPTIMIZER( keys, Forward_Accel );
            INIT_OPTIMIZER( keys, Retro_Accel );
            INIT_OPTIMIZER( keys, Left_Accel );
            INIT_OPTIMIZER( keys, Right_Accel );
            INIT_OPTIMIZER( keys, Top_Accel );
            INIT_OPTIMIZER( keys, Bottom_Accel );
            INIT_OPTIMIZER( keys, Default_Speed_Governor );
            INIT_OPTIMIZER( keys, Afterburner_Speed_Governor );
            INIT_OPTIMIZER( keys, ITTS );
            INIT_OPTIMIZER( keys, Can_Lock );
            INIT_OPTIMIZER( keys, Radar_Color );
            INIT_OPTIMIZER( keys, Radar_Range );
            INIT_OPTIMIZER( keys, Max_Cone );
            INIT_OPTIMIZER( keys, Tracking_Cone );
            INIT_OPTIMIZER( keys, Lock_Cone );
            INIT_OPTIMIZER( keys, Cloak_Min );
            INIT_OPTIMIZER( keys, Cloak_Glass );
            INIT_OPTIMIZER( keys, Can_Cloak );
            INIT_OPTIMIZER( keys, Cloak_Rate );
            INIT_OPTIMIZER( keys, Cloak_Energy );
            INIT_OPTIMIZER( keys, Repair_Droid );
            INIT_OPTIMIZER( keys, ECM_Rating );
            INIT_OPTIMIZER( keys, Heat_Sink_Rating );
            INIT_OPTIMIZER( keys, Hud_Functionality );
            INIT_OPTIMIZER( keys, Max_Hud_Functionality );
            INIT_OPTIMIZER( keys, Lifesupport_Functionality );
            INIT_OPTIMIZER( keys, Max_Lifesupport_Functionality );
            INIT_OPTIMIZER( keys, Comm_Functionality );
            INIT_OPTIMIZER( keys, Max_Comm_Functionality );
            INIT_OPTIMIZER( keys, FireControl_Functionality );
            INIT_OPTIMIZER( keys, Max_FireControl_Functionality );
            INIT_OPTIMIZER( keys, SPECDrive_Functionality );
            INIT_OPTIMIZER( keys, Max_SPECDrive_Functionality );
            INIT_OPTIMIZER( keys, Slide_Start );
            INIT_OPTIMIZER( keys, Slide_End );
            INIT_OPTIMIZER( keys, Upgrades );
            INIT_OPTIMIZER( keys, Tractorability );
            INIT_OPTIMIZER( keys, Explosion );
            INIT_OPTIMIZER( keys, Light );
            INIT_OPTIMIZER( keys, Shield_Mesh );
            INIT_OPTIMIZER( keys, Rapid_Mesh );
            INIT_OPTIMIZER( keys, Use_Rapid );
            INIT_OPTIMIZER( keys, NoDamageParticles );
            INIT_OPTIMIZER( keys, Spec_Interdiction );
        }
        table->SetupOptimizer( keys, LOADROW_OPTIMIZER );
    }
    //begin the geometry (and things that depend on stats)
    fullname = OPTIM_GET( row, table, Name );
    if ( ( tmpstr = OPTIM_GET( row, table, Hud_image ) ).length() != 0 ) {
        std::string fac = FactionUtil::GetFaction( faction );
        fac += "_";
        fac += tmpstr;
        pImage->pHudImage = createVSSprite( fac.c_str() );
        if ( !isVSSpriteLoaded( pImage->pHudImage ) ) {
            deleteVSSprite( pImage->pHudImage );
            pImage->pHudImage = createVSSprite( tmpstr.c_str() );
        }
    }
    if ( ( tmpstr = OPTIM_GET( row, table, FaceCamera ) ).length() != 0 )
        graphicOptions.FaceCamera = XMLSupport::parse_bool( tmpstr ) ? 1 : 0;
    std::string llegacy_combat_role( OPTIM_GET( row, table, Combat_Role ) );
    std::string lunit_role( OPTIM_GET( row, table, Unit_Role ) );
    std::string lattack_preference( OPTIM_GET( row, table, Attack_Preference ) );
    if (lunit_role.length() == 0)
        this->setUnitRole( llegacy_combat_role );
    else
        this->setUnitRole( lunit_role );
    if (lattack_preference.length() == 0)
        this->setAttackPreference( llegacy_combat_role );
    else
        this->setAttackPreference( lattack_preference );
    graphicOptions.NumAnimationPoints = stoi( OPTIM_GET( row, table, Num_Animation_Stages ), 0 );
    graphicOptions.NoDamageParticles  = stoi( OPTIM_GET( row, table, NoDamageParticles ), 0 );
    if (graphicOptions.NumAnimationPoints > 0)
        graphicOptions.Animating = 0;
    xml.unitscale     = stof( OPTIM_GET( row, table, Unit_Scale ), 1 );
    if (!xml.unitscale) xml.unitscale = 1;
    pImage->unitscale = xml.unitscale;
    AddMeshes( xml.meshes, xml.randomstartframe, xml.randomstartseconds, xml.unitscale, OPTIM_GET( row,
                                                                                                   table,
                                                                                                   Mesh ), faction,
              getFlightgroup() );
    AddDocks( this, xml, OPTIM_GET( row, table, Dock ) );
    AddSubUnits( this, xml, OPTIM_GET( row, table, Sub_Units ), faction, modification );

    meshdata   = xml.meshes;
    meshdata.push_back( NULL );
    corner_min = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
    corner_max = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );
    calculate_extent( false );
    AddMounts( this, xml, OPTIM_GET( row, table, Mounts ) );
    this->CargoVolume = ::stof( OPTIM_GET( row, table, Hold_Volume ) );
    this->HiddenCargoVolume = ::stof( OPTIM_GET( row, table, Hidden_Hold_Volume ) );
    this->UpgradeVolume     = ::stof( OPTIM_GET( row, table, Upgrade_Storage_Volume ) );
    this->equipment_volume  = ::stof( OPTIM_GET( row, table, Equipment_Space ) );
    ImportCargo( this, OPTIM_GET( row, table, Cargo_Import ) );     //if this changes change planet_generic.cpp
    AddCarg( this, OPTIM_GET( row, table, Cargo ) );

    // Replaced by below: AddSounds( this, OPTIM_GET( row, table, Sounds ) );
    this->addSounds(&nextElement, OPTIM_GET( row, table, Sounds ));


    LoadCockpit( this, OPTIM_GET( row, table, Cockpit ) );
    pImage->CockpitCenter.i = ::stof( OPTIM_GET( row, table, CockpitX ) )*xml.unitscale;
    pImage->CockpitCenter.j = ::stof( OPTIM_GET( row, table, CockpitY ) )*xml.unitscale;
    pImage->CockpitCenter.k = ::stof( OPTIM_GET( row, table, CockpitZ ) )*xml.unitscale;
    Mass = stof( OPTIM_GET( row, table, Mass ), 1.0 );
    Momentofinertia = stof( OPTIM_GET( row, table, Moment_Of_Inertia ), 1.0 );
    fuel = ::stof( OPTIM_GET( row, table, Fuel_Capacity ) );
    hull = maxhull = ::stof( OPTIM_GET( row, table, Hull ) );
    specInterdiction = ::stof( OPTIM_GET( row, table, Spec_Interdiction ) );
    armor.frontlefttop     = ::stof( OPTIM_GET( row, table, Armor_Front_Top_Left ) );
    armor.frontrighttop    = ::stof( OPTIM_GET( row, table, Armor_Front_Top_Right ) );
    armor.backlefttop      = ::stof( OPTIM_GET( row, table, Armor_Back_Top_Left ) );
    armor.backrighttop     = ::stof( OPTIM_GET( row, table, Armor_Back_Top_Right ) );
    armor.frontleftbottom  = ::stof( OPTIM_GET( row, table, Armor_Front_Bottom_Left ) );
    armor.frontrightbottom = ::stof( OPTIM_GET( row, table, Armor_Front_Bottom_Right ) );
    armor.backleftbottom   = ::stof( OPTIM_GET( row, table, Armor_Back_Bottom_Left ) );
    armor.backrightbottom  = ::stof( OPTIM_GET( row, table, Armor_Back_Bottom_Right ) );
    int    shieldcount = 0;
    Shield two;
    Shield four;
    Shield eight;
/*    It looks like the shields are initialized properly anyways
 *    memset( &two, 0, sizeof (Shield) );
 *    memset( &four, 0, sizeof (Shield) );
 *    memset( &eight, 0, sizeof (Shield) );
 */
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Front_Top_Right ),
                             two.shield2fb.front, four.shield4fbrl.front, eight.shield8.frontrighttop );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Front_Top_Left ),
                             two.shield2fb.front, four.shield4fbrl.front, eight.shield8.frontlefttop );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Back_Top_Left ),
                             two.shield2fb.back, four.shield4fbrl.back, eight.shield8.backlefttop );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Back_Top_Right ),
                             two.shield2fb.back, four.shield4fbrl.back, eight.shield8.backrighttop );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Front_Bottom_Left ),
                             two.shield2fb.front, four.shield4fbrl.left, eight.shield8.frontleftbottom );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Front_Bottom_Right ),
                             two.shield2fb.front, four.shield4fbrl.right, eight.shield8.frontrightbottom );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Back_Bottom_Left ),
                             two.shield2fb.back, four.shield4fbrl.left, eight.shield8.backleftbottom );
    shieldcount += AssignIf( OPTIM_GET( row, table, Shield_Back_Bottom_Right ),
                             two.shield2fb.back, four.shield4fbrl.right, eight.shield8.backrightbottom );
    two.shield2fb.frontmax = two.shield2fb.front;
    two.shield2fb.backmax = two.shield2fb.back;
    four.shield4fbrl.frontmax = four.shield4fbrl.front;
    four.shield4fbrl.backmax = four.shield4fbrl.back;
    four.shield4fbrl.rightmax = four.shield4fbrl.right;
    four.shield4fbrl.leftmax = four.shield4fbrl.left;
    eight.shield8.frontlefttopmax     = eight.shield8.frontlefttop;
    eight.shield8.frontrighttopmax    = eight.shield8.frontrighttop;
    eight.shield8.backrighttopmax     = eight.shield8.backrighttop;
    eight.shield8.backlefttopmax      = eight.shield8.backlefttop;
    eight.shield8.frontleftbottommax  = eight.shield8.frontleftbottom;
    eight.shield8.frontrightbottommax = eight.shield8.frontrightbottom;
    eight.shield8.backrightbottommax  = eight.shield8.backrightbottom;
    eight.shield8.backleftbottommax   = eight.shield8.backleftbottom;
    float r45  = VS_PI/4;
    float r90  = VS_PI/2;
    float r135 = 3*VS_PI/4;
    float r180 = VS_PI;
    float r225 = 5*VS_PI/4;
    float r270 = 3*VS_PI/2;
    float r315 = 7*VS_PI/4;
    float r360 = 2*VS_PI;
    int   iter;
    if (shieldcount > MAX_SHIELD_NUMBER)
        shieldcount = MAX_SHIELD_NUMBER;
 //   memset( shield.range, 0, sizeof (shield.range) );   The range is properly initialized anyways
    if (shieldcount == 8) {
        shield.number = 8;
        shield.shield.cur[0]     = shield.shield.max[0] = eight.shield8.frontlefttopmax;
        shield.range[0].thetamin = 0;
        shield.range[0].thetamax = r90;
        shield.range[0].rhomin   = 0;
        shield.range[0].rhomax   = r90;

        shield.shield.cur[1]     = shield.shield.max[1] = eight.shield8.backlefttopmax;
        shield.range[1].thetamin = r90;
        shield.range[1].thetamax = r180;
        shield.range[1].rhomin   = 0;
        shield.range[1].rhomax   = r90;

        shield.shield.cur[2]     = shield.shield.max[2] = eight.shield8.frontrighttopmax;
        shield.range[2].thetamin = r270;
        shield.range[2].thetamax = r360;
        shield.range[2].rhomin   = 0;
        shield.range[2].rhomax   = r90;

        shield.shield.cur[3]     = shield.shield.max[3] = eight.shield8.backrighttopmax;
        shield.range[3].thetamin = r180;
        shield.range[3].thetamax = r270;
        shield.range[3].rhomin   = 0;
        shield.range[3].rhomax   = r90;

        shield.shield.cur[4]     = shield.shield.max[4] = eight.shield8.frontleftbottommax;
        shield.range[4].thetamin = 0;
        shield.range[4].thetamax = r90;
        shield.range[4].rhomin   = -r90;
        shield.range[4].rhomax   = 0;

        shield.shield.cur[5]     = shield.shield.max[5] = eight.shield8.backleftbottommax;
        shield.range[5].thetamin = r90;
        shield.range[5].thetamax = r180;
        shield.range[5].rhomin   = -r90;
        shield.range[5].rhomax   = 0;

        shield.shield.cur[6]     = shield.shield.max[6] = eight.shield8.frontrightbottommax;
        shield.range[6].thetamin = r270;
        shield.range[6].thetamax = r360;
        shield.range[6].rhomin   = -r90;
        shield.range[6].rhomax   = 0;

        shield.shield.cur[7]     = shield.shield.max[7] = eight.shield8.backrightbottommax;
        shield.range[7].thetamin = r180;
        shield.range[7].thetamax = r270;
        shield.range[7].rhomin   = -r90;
        shield.range[7].rhomax   = 0;
    } else if (shieldcount == 4) {
        shield.number = 4;

        shield.shield.cur[0]     = shield.shield.max[0] = four.shield4fbrl.frontmax;
        shield.range[0].thetamin = r315;
        shield.range[0].thetamax = r360+r45;
        shield.range[0].rhomin   = -r90;
        shield.range[0].rhomax   = r90;

        shield.shield.cur[1]     = shield.shield.max[1] = four.shield4fbrl.backmax;
        shield.range[1].thetamin = r135;
        shield.range[1].thetamax = r225;
        shield.range[1].rhomin   = -r90;
        shield.range[1].rhomax   = r90;

        shield.shield.cur[2]     = shield.shield.max[2] = four.shield4fbrl.rightmax;
        shield.range[2].thetamin = r225;
        shield.range[2].thetamax = r315;
        shield.range[2].rhomin   = -r90;
        shield.range[2].rhomax   = r90;

        shield.shield.cur[3]     = shield.shield.max[3] = four.shield4fbrl.leftmax;
        shield.range[3].thetamin = r45;
        shield.range[3].thetamax = r225;
        shield.range[3].rhomin   = -r90;
        shield.range[3].rhomax   = r90;
    } else if (shieldcount == 2) {
        shield.number = 2;

        shield.shield.cur[0]     = shield.shield.max[0] = four.shield2fb.frontmax;
        shield.range[0].thetamin = r270;
        shield.range[0].thetamax = r360+r90;
        shield.range[0].rhomin   = -r90;
        shield.range[0].rhomax   = r90;

        shield.shield.cur[1]     = shield.shield.max[1] = four.shield2fb.backmax;
        shield.range[1].thetamin = r90;
        shield.range[1].thetamax = r270;
        shield.range[1].rhomin   = -r90;
        shield.range[1].rhomax   = r90;
    } else {
        //No shields
        shield.number = 0;
    }
    for (iter = 0; iter < shieldcount; ++iter) {
        std::string shieldname = "Shield_"+XMLSupport::tostring( iter );
        AssignIfDeg( row[shieldname+"_Min_Theta"], shield.range[iter].thetamin );
        AssignIfDeg( row[shieldname+"_Max_Theta"], shield.range[iter].thetamax );
        AssignIfDeg( row[shieldname+"_Min_Rho"], shield.range[iter].rhomin );
        AssignIfDeg( row[shieldname+"_Max_Rho"], shield.range[iter].rhomax );
    }
    shield.leak = (char) (::stof( OPTIM_GET( row, table, Shield_Leak ) )*100.0);
    shield.recharge   = ::stof( OPTIM_GET( row, table, Shield_Recharge ) );
    shield.efficiency = ::stof( OPTIM_GET( row, table, Shield_Efficiency ), 1.0 );

    static bool WCfuelhack = XMLSupport::parse_bool( vs_config->getVariable( "physics", "fuel_equals_warp", "false" ) );
    maxwarpenergy     = warpenergy = ::stof( OPTIM_GET( row, table, Warp_Capacitor ) );

    graphicOptions.MinWarpMultiplier = ::stof( OPTIM_GET( row, table, Warp_Min_Multiplier ), 1.0 );
    graphicOptions.MaxWarpMultiplier = ::stof( OPTIM_GET( row, table, Warp_Max_Multiplier ), 1.0 );

    maxenergy  = energy = ::stof( OPTIM_GET( row, table, Primary_Capacitor ) );
    recharge   = ::stof( OPTIM_GET( row, table, Reactor_Recharge ) );
    jump.drive = XMLSupport::parse_bool( OPTIM_GET( row, table, Jump_Drive_Present ) ) ? -1 : -2;
    jump.delay = ::stoi( OPTIM_GET( row, table, Jump_Drive_Delay ) );
    forcejump = XMLSupport::parse_bool( OPTIM_GET( row, table, Wormhole ) );
    graphicOptions.RecurseIntoSubUnitsOnCollision = stob( OPTIM_GET( row,
                                                                     table,
                                                                     Collide_Subunits ),
                                                          graphicOptions.RecurseIntoSubUnitsOnCollision ? true : false ) ? 1
                                                    : 0;
    jump.energy = ::stof( OPTIM_GET( row, table, Outsystem_Jump_Cost ) );
    jump.insysenergy = ::stof( OPTIM_GET( row, table, Warp_Usage_Cost ) );
    if (WCfuelhack) fuel = warpenergy = warpenergy+jump.energy*0.1f;       //this is required to make sure we don't trigger the "globally out of fuel" if we use all warp charges -- save some afterburner for later!!!
    afterburnenergy  = ::stof( OPTIM_GET( row, table, Afterburner_Usage_Cost ), 32767 );
    afterburntype    = ::stoi( OPTIM_GET( row, table, Afterburner_Type ) );     //type 1 == "use fuel", type 0 == "use reactor energy", type 2 ==(hopefully) "use jump fuel" 3: NO AFTERBURNER
    limits.yaw = ::stof( OPTIM_GET( row, table, Maneuver_Yaw ) )*VS_PI/180.;
    limits.pitch     = ::stof( OPTIM_GET( row, table, Maneuver_Pitch ) )*VS_PI/180.;
    limits.roll      = ::stof( OPTIM_GET( row, table, Maneuver_Roll ) )*VS_PI/180.;
    {
        std::string t, tn, tp;
        t  = OPTIM_GET( row, table, Yaw_Governor );
        tn = OPTIM_GET( row, table, Yaw_Governor_Right );
        tp = OPTIM_GET( row, table, Yaw_Governor_Left );
        computer.max_yaw_right = ::stof( tn.length() > 0 ? tn : t )*VS_PI/180.;
        computer.max_yaw_left = ::stof( tp.length() > 0 ? tp : t )*VS_PI/180.;
        t  = OPTIM_GET( row, table, Pitch_Governor );
        tn = OPTIM_GET( row, table, Pitch_Governor_Up );
        tp = OPTIM_GET( row, table, Pitch_Governor_Down );
        computer.max_pitch_up = ::stof( tn.length() > 0 ? tn : t )*VS_PI/180.;
        computer.max_pitch_down = ::stof( tp.length() > 0 ? tp : t )*VS_PI/180.;
        t  = OPTIM_GET( row, table, Roll_Governor );
        tn = OPTIM_GET( row, table, Roll_Governor_Right );
        tp = OPTIM_GET( row, table, Roll_Governor_Left );
        computer.max_roll_right = ::stof( tn.length() > 0 ? tn : t )*VS_PI/180.;
        computer.max_roll_left = ::stof( tp.length() > 0 ? tp : t )*VS_PI/180.;
    }
    static float game_accel = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_accel", "1" ) );
    static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
    limits.afterburn = ::stof( OPTIM_GET( row, table, Afterburner_Accel ) )*game_accel*game_speed;
    limits.forward   = ::stof( OPTIM_GET( row, table, Forward_Accel ) )*game_accel*game_speed;
    limits.retro     = ::stof( OPTIM_GET( row, table, Retro_Accel ) )*game_accel*game_speed;
    limits.lateral   = .5
                       *( ::stof( OPTIM_GET( row, table,
                                             Left_Accel ) )+::stof( OPTIM_GET( row, table, Right_Accel ) ) )*game_accel*game_speed;
    limits.vertical  = .5
                       *( ::stof( OPTIM_GET( row, table,
                                             Top_Accel ) )+::stof( OPTIM_GET( row, table, Bottom_Accel ) ) )*game_accel*game_speed;
    computer.max_combat_speed    = ::stof( OPTIM_GET( row, table, Default_Speed_Governor ) )*game_speed;
    computer.max_combat_ab_speed = ::stof( OPTIM_GET( row, table, Afterburner_Speed_Governor ) )*game_speed;
    computer.itts = stob( OPTIM_GET( row, table, ITTS ), true );
    computer.radar.canlock = stob( OPTIM_GET( row, table, Can_Lock ), true );
    {
        // The Radar_Color column in the units.csv has been changed from a
        // boolean value to a string. The boolean values are supported for
        // backwardscompatibility.
        // When we save this setting, it is simply converted from an integer
        // number to a string, and we need to support this as well.
        std::string iffval = OPTIM_GET( row, table, Radar_Color );
        if ((iffval.empty()) || (iffval == "FALSE") || (iffval == "0"))
        {
            computer.radar.capability = Computer::RADARLIM::Capability::IFF_NONE;
        }
        else if ((iffval == "TRUE") || (iffval == "1"))
        {
            computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_SPHERE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE;
        }
        else if (iffval == "THREAT")
        {
            computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_SPHERE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
        }
        else if (iffval == "BUBBLE_THREAT")
        {
            computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_BUBBLE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_OBJECT_RECOGNITION
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
        }
        else if (iffval == "PLANE")
        {
            computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_PLANE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE;
        }
        else if (iffval == "PLANE_THREAT")
        {
            computer.radar.capability
                = Computer::RADARLIM::Capability::IFF_PLANE
                | Computer::RADARLIM::Capability::IFF_FRIEND_FOE
                | Computer::RADARLIM::Capability::IFF_OBJECT_RECOGNITION
                | Computer::RADARLIM::Capability::IFF_THREAT_ASSESSMENT;
        }
        else
        {
            unsigned int value = stoi(iffval, 0);
            if (value == 0)
            {
                // Unknown value
                assert(false);
                computer.radar.capability = Computer::RADARLIM::Capability::IFF_NONE;
            }
            else
            {
                computer.radar.capability = value;
            }
        }
    }
    computer.radar.maxrange     = stof( OPTIM_GET( row, table, Radar_Range ), FLT_MAX );
    computer.radar.maxcone      = cos( stof( OPTIM_GET( row, table, Max_Cone ), 180 )*VS_PI/180 );
    computer.radar.trackingcone = cos( stof( OPTIM_GET( row, table, Tracking_Cone ), 180 )*VS_PI/180 );
    computer.radar.lockcone     = cos( stof( OPTIM_GET( row, table, Lock_Cone ), 180 )*VS_PI/180 );
    cloakmin = (int) (::stof( OPTIM_GET( row, table, Cloak_Min ) )*2147483136);
    if (cloakmin < 0) cloakmin = 0;
    cloakglass = XMLSupport::parse_bool( OPTIM_GET( row, table, Cloak_Glass ) );
    if ( (cloakmin&0x1) && !cloakglass )
        cloakmin -= 1;
    if ( (cloakmin&0x1) == 0 && cloakglass )
        cloakmin += 1;
    if ( !XMLSupport::parse_bool( OPTIM_GET( row, table, Can_Cloak ) ) )
        cloaking = -1;
    else
        cloaking = (int) (-2147483647)-1;
    cloakrate    = (int) ( 2147483136.*::stof( OPTIM_GET( row, table, Cloak_Rate ) ) );     //short fix
    cloakenergy  = ::stof( OPTIM_GET( row, table, Cloak_Energy ) );
    repair_droid = ::stoi( OPTIM_GET( row, table, Repair_Droid ) );
    ecm    = ::stoi( OPTIM_GET( row, table, ECM_Rating ) );

    this->HeatSink = ::stof( OPTIM_GET( row, table, Heat_Sink_Rating ) );
    if (ecm < 0) ecm *= -1;
    if (pImage->cockpit_damage) {
        HudDamage( pImage->cockpit_damage, OPTIM_GET( row, table, Hud_Functionality ) );
        HudDamage( pImage->cockpit_damage+1+MAXVDUS+UnitImages< void >::NUMGAUGES, OPTIM_GET( row, table, Max_Hud_Functionality ) );
    }
    LifeSupportFunctionality    = ::stof( OPTIM_GET_DEF( row, table, Lifesupport_Functionality, "1" ) );
    LifeSupportFunctionalityMax = ::stof( OPTIM_GET_DEF( row, table, Max_Lifesupport_Functionality, "1" ) );
    CommFunctionality = ::stof( OPTIM_GET_DEF( row, table, Comm_Functionality, "1" ) );
    CommFunctionalityMax = ::stof( OPTIM_GET_DEF( row, table, Max_Comm_Functionality, "1" ) );
    fireControlFunctionality    = ::stof( OPTIM_GET_DEF( row, table, FireControl_Functionality, "1" ) );
    fireControlFunctionalityMax = ::stof( OPTIM_GET_DEF( row, table, Max_FireControl_Functionality, "1" ) );
    SPECDriveFunctionality = ::stof( OPTIM_GET_DEF( row, table, SPECDrive_Functionality, "1" ) );
    SPECDriveFunctionalityMax   = ::stof( OPTIM_GET_DEF( row, table, Max_SPECDrive_Functionality, "1" ) );
    computer.slide_start = ::stoi( OPTIM_GET( row, table, Slide_Start ) );
    computer.slide_end   = ::stoi( OPTIM_GET( row, table, Slide_End ) );
    UpgradeUnit( this, OPTIM_GET( row, table, Upgrades ) );
    {
        std::string   tractorability = OPTIM_GET( row, table, Tractorability );
        unsigned char tflags;
        if ( !tractorability.empty() ) {
            tflags = tractorImmune;
            if (tractorability.find_first_of( "pP" ) != string::npos)
                tflags |= tractorPush;
            if (tractorability.find_first_of( "iI" ) != string::npos)
                tflags |= tractorIn;
        } else {tflags = tractorPush; } setTractorability( (enum tractorHow) tflags );
    }
    this->pImage->explosion_type = OPTIM_GET( row, table, Explosion );
    if ( pImage->explosion_type.get().length() ) {
        cache_ani( pImage->explosion_type );
    } else {
        static std::string expani = vs_config->getVariable( "graphics", "explosion_animation", "explosion_orange.ani" );
        cache_ani( expani );
    }
    AddLights( this, xml, OPTIM_GET( row, table, Light ) );
    xml.shieldmesh_str = OPTIM_GET( row, table, Shield_Mesh );
    if ( xml.shieldmesh_str.length() ) {
        addShieldMesh( &xml, xml.shieldmesh_str.c_str(), xml.unitscale, faction, getFlightgroup() );
        meshdata.back() = xml.shieldmesh;
    } else {
        static int shieldstacks = XMLSupport::parse_int( vs_config->getVariable( "graphics", "shield_detail", "16" ) );
        static std::string shieldtex = vs_config->getVariable( "graphics", "shield_texture", "shield.bmp" );
        static std::string shieldtechnique = vs_config->getVariable( "graphics", "shield_technique", "" );
        meshdata.back() = new SphereMesh( rSize(), shieldstacks, shieldstacks, shieldtex.c_str(), shieldtechnique, NULL, false, ONE, ONE );
    }
    meshdata.back()->EnableSpecialFX();
    //Begin the Pow-w-w-war Zone Collide Tree Generation
    {
        xml.rapidmesh_str = OPTIM_GET( row, table, Rapid_Mesh );
        vector< mesh_polygon >polies;

        std::string collideTreeHash = VSFileSystem::GetHashName( modification+"#"+row[0] );
        this->colTrees = collideTrees::Get( collideTreeHash );
        if (this->colTrees)
            this->colTrees->Inc();
        csOPCODECollider *colShield = NULL;
        string   tmpname   = row[0];       //key
        if (!this->colTrees) {
            string val;
            xml.hasColTree = 1;
            if ( ( val = OPTIM_GET( row, table, Use_Rapid ) ).length() )
                xml.hasColTree = XMLSupport::parse_bool( val );
            if (xml.shieldmesh) {
                if ( meshdata.back() ) {
                    meshdata.back()->GetPolys( polies );
                    colShield = new csOPCODECollider( polies );
                }
            }
            if ( xml.rapidmesh_str.length() )
                addRapidMesh( &xml, xml.rapidmesh_str.c_str(), xml.unitscale, faction, getFlightgroup() );
            else
                xml.rapidmesh = NULL;
            polies.clear();
            if (xml.rapidmesh)
                xml.rapidmesh->GetPolys( polies );
            csOPCODECollider *csrc = NULL;
            if (xml.hasColTree) {
                csrc = getCollideTree( Vector( 1, 1, 1 ),
                                       xml.rapidmesh
                                       ? &polies : NULL );
            }
            this->colTrees = new collideTrees( collideTreeHash,
                                               csrc,
                                               colShield );
            if (xml.rapidmesh && xml.hasColTree) {
                //if we have a special rapid mesh we need to generate things now
                for (unsigned int i = 1; i < collideTreesMaxTrees; ++i)
                    if (!this->colTrees->rapidColliders[i]) {
                        unsigned int which = 1<<i;
                        this->colTrees->rapidColliders[i] =
                            getCollideTree( Vector( 1, 1, which ),
                                            &polies );
                    }
            }
            if (xml.rapidmesh) {
                delete xml.rapidmesh;
                xml.rapidmesh = NULL;
            }
        }
    }
    CheckAccessory( this );     //turns on the ceerazy rotation for any accessories
    this->setAverageGunSpeed();
}

CSVRow GetUnitRow( string filename, bool subu, int faction, bool readlast, bool &rread )
{
    std::string hashname = filename+"__"+FactionUtil::GetFactionName( faction );
    for (int i = ( (int) unitTables.size() )-(readlast ? 1 : 2); i >= 0; --i) {
        unsigned int where;
        if ( unitTables[i]->RowExists( hashname, where ) ) {
            rread = true;
            return CSVRow( unitTables[i], where );
        } else if ( unitTables[i]->RowExists( filename, where ) ) {
            rread = true;
            return CSVRow( unitTables[i], where );
        }
    }
    rread = false;
    return CSVRow();
}

void Unit::WriteUnit( const char *modifications )
{
    static bool UNITTAB = XMLSupport::parse_bool( vs_config->getVariable( "physics", "UnitTable", "false" ) );
    if (UNITTAB) {
        bool bad = false;
        if (!modifications) bad = true;
        if (!bad)
            if ( !strlen( modifications ) ) {
                bad = true;
            }
        if (bad) {
            BOOST_LOG_TRIVIAL(error) << boost::format("Cannot Write out unit file %1% %2% that has no filename") % name.get().c_str() % csvRow.get().c_str();
            return;
        }
        std::string savedir = modifications;
        VSFileSystem::CreateDirectoryHome( VSFileSystem::savedunitpath+"/"+savedir );
        VSFile  f;
        VSError err = f.OpenCreateWrite( savedir+"/"+name+".csv", UnitFile );
        if (err > Ok) {
            BOOST_LOG_TRIVIAL(error) << boost::format("!!! ERROR : Writing saved unit file : %1%") % f.GetFullPath().c_str();
            return;
        }
        std::string towrite = WriteUnitString();
        f.Write( towrite.c_str(), towrite.length() );
        f.Close();
    } else {
        if (pImage->unitwriter)
            pImage->unitwriter->Write( modifications );
        for (un_iter ui = getSubUnits(); (*ui) != NULL; ++ui)
            (*ui)->WriteUnit( modifications );
    }
}

using XMLSupport::tostring;

static void mapToStringVec( vsUMap< string, string >a, vector< string > &key, vector< string > &value )
{
    for (vsUMap< string, string >::iterator i = a.begin(); i != a.end(); ++i) {
        key.push_back( i->first );
        value.push_back( i->second );
    }
}

static string tos( double val )
{
    return XMLSupport::tostring( (float)val );
}

static string tos( bool val )
{
    return XMLSupport::tostring( (int) val );
}

static string tos( int val )
{
    return XMLSupport::tostring( val );
}

string Unit::WriteUnitString()
{
    static bool UNITTAB = XMLSupport::parse_bool( vs_config->getVariable( "physics", "UnitTable", "false" ) );
    string ret = "";
    if (UNITTAB) {
        //this is the fillin part
        //fixme
        for (int i = unitTables.size()-1; i >= 0; --i) {
            unsigned int where;
            string val;
            if ( unitTables[i]->RowExists( csvRow, where ) ) {
                CSVRow row( unitTables[i], where );
                vsUMap< string, string >unit;
                for (unsigned int jj = 0; jj < row.size(); ++jj)
                    if (jj != 0)
                        unit[row.getKey( jj )] = row[jj];
                //mutable things
                unit["Equipment_Space"] = XMLSupport::tostring( equipment_volume );
                unit["Hold_Volume"] = XMLSupport::tostring( CargoVolume );
                unit["Hidden_Hold_Volume"]     = XMLSupport::tostring( HiddenCargoVolume );
                unit["Upgrade_Storage_Volume"] = XMLSupport::tostring( UpgradeVolume );
                string mountstr;
                double unitScale = stof( unit["Unit_Scale"], 1 );
                {
                    //mounts
                    for (unsigned int j = 0; j < mounts.size(); ++j) {
                        char   mnt[1024];
                        Matrix m;
                        Transformation tr( mounts[j].GetMountOrientation(),
                                          mounts[j].GetMountLocation().Cast() );
                        tr.to_matrix( m );
                        string printedname = mounts[j].type->name;
                        if (mounts[j].status == Mount::DESTROYED || mounts[j].status == Mount::UNCHOSEN)
                            printedname = "";
                        mountstr += "{"+printedname+";"+XMLSupport::tostring( mounts[j].ammo )+";"+XMLSupport::tostring(
                            mounts[j].volume )+";"+getMountSizeString( mounts[j].size );
                        sprintf( mnt, ";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                                 m.p.i/unitScale,
                                 m.p.j/unitScale,
                                 m.p.k/unitScale,
                                 (double) mounts[j].xyscale/unitScale,
                                 (double) mounts[j].zscale/unitScale,
                                 (double) m.getR().i,
                                 (double) m.getR().j,
                                 (double) m.getR().k,
                                 (double) m.getQ().i,
                                 (double) m.getQ().j,
                                 (double) m.getQ().k,
                                 (double) mounts[j].functionality,
                                 (double) mounts[j].maxfunctionality
                               );
                        mountstr += mnt;
                    }
                    unit["Mounts"] = mountstr;
                }
                {
                    //subunits
                    vector< SubUnitStruct >subunits = GetSubUnits( unit["Sub_Units"] );
                    if ( subunits.size() ) {
                        unsigned int k = 0;
                        Unit *subun;
                        for (; k < subunits.size(); ++k)
                            subunits[k].filename = "destroyed_blank";
                        k = 0;
                        for (un_iter su = this->getSubUnits(); ( subun = (*su) ) != NULL; ++su, ++k) {
                            unsigned int j = k;
                            for (; j < subunits.size(); ++j)
                                if ( (subun->Position()-subunits[j].pos).MagnitudeSquared() < .00000001 )
                                    //we've got a hit
                                    break;
                            if ( j >= subunits.size() ) j = k;
                            if ( j < subunits.size() )
                                subunits[j].filename = subun->name;
                        }
                        string str;
                        for (k = 0; k < subunits.size(); ++k) {
                            char tmp[1024];
                            sprintf( tmp, ";%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf;%lf}",
                                    subunits[k].pos.i,
                                    subunits[k].pos.j,
                                    subunits[k].pos.k,
                                    subunits[k].R.i,
                                    subunits[k].R.j,
                                    subunits[k].R.k,
                                    subunits[k].Q.i,
                                    subunits[k].Q.j,
                                    subunits[k].Q.k,
                                    ( (double) acos( subunits[k].restricted )*180./VS_PI ) );
                            str += "{"+subunits[k].filename+tmp;
                        }
                        unit["Sub_Units"] = str;
                    }
                }
                {
                    string carg;
                    for (unsigned int i = 0; i < numCargo(); ++i) {
                        Cargo *c = &GetCargo( i );
                        char   tmp[2048];
                        sprintf( tmp, ";%f;%d;%f;%f;%f;%f;;%s;%s}",
                                 c->price,
                                 c->quantity,
                                 c->mass,
                                 c->volume,
                                 c->functionality,
                                 c->maxfunctionality,
                                 c->mission ? "true" : "false",
                                 c->installed ? "true" : "false"
                               );
                        carg += "{"+c->GetContent()+";"+c->GetCategory()+tmp;
                    }
                    unit["Cargo"] = carg;
                }
                unit["Mass"] = tos( Mass );
                unit["Moment_Of_Inertia"] = tos( Momentofinertia );
                unit["Fuel_Capacity"] = tos( fuel );
                unit["Hull"] = tos( hull );
                unit["Spec_Interdiction"] = tos( specInterdiction );
                unit["Armor_Front_Top_Left"]     = tos( armor.frontlefttop );
                unit["Armor_Front_Top_Right"]    = tos( armor.frontrighttop );
                unit["Armor_Back_Top_Left"]      = tos( armor.backlefttop );
                unit["Armor_Back_Top_Right"]     = tos( armor.backrighttop );
                unit["Armor_Front_Bottom_Left"]  = tos( armor.frontleftbottom );
                unit["Armor_Front_Bottom_Right"] = tos( armor.frontrightbottom );
                unit["Armor_Back_Bottom_Left"]   = tos( armor.backleftbottom );
                unit["Armor_Back_Bottom_Right"]  = tos( armor.backrightbottom );
                {
                    unit["Shield_Front_Top_Right"]    = "";
                    unit["Shield_Front_Top_Left"]     = "";
                    unit["Shield_Back_Top_Right"]     = "";
                    unit["Shield_Back_Top_Left"]      = "";
                    unit["Shield_Front_Bottom_Right"] = "";
                    unit["Shield_Front_Bottom_Left"]  = "";
                    unit["Shield_Back_Bottom_Right"]  = "";
                    unit["Shield_Back_Bottom_Left"]   = "";
                    switch (shield.number)
                    {
                    case 8:
                        unit["Shield_Front_Top_Right"]    = tos( shield.shield8.frontrighttopmax );
                        unit["Shield_Front_Top_Left"]     = tos( shield.shield8.frontlefttopmax );
                        unit["Shield_Back_Top_Right"]     = tos( shield.shield8.backrighttopmax );
                        unit["Shield_Back_Top_Left"]      = tos( shield.shield8.backlefttopmax );
                        unit["Shield_Front_Bottom_Right"] = tos( shield.shield8.frontrightbottommax );
                        unit["Shield_Front_Bottom_Left"]  = tos( shield.shield8.frontleftbottommax );
                        unit["Shield_Back_Bottom_Right"]  = tos( shield.shield8.backrightbottommax );
                        unit["Shield_Back_Bottom_Left"]   = tos( shield.shield8.backleftbottommax );
                        break;
                    case 4:
                        unit["Shield_Front_Top_Right"]    = tos( shield.shield4fbrl.frontmax );
                        unit["Shield_Back_Top_Right"]     = tos( shield.shield4fbrl.backmax );
                        unit["Shield_Front_Bottom_Right"] = tos( shield.shield4fbrl.rightmax );
                        unit["Shield_Front_Bottom_Left"]  = tos( shield.shield4fbrl.leftmax );
                        break;
                    case 2:
                        unit["Shield_Front_Top_Right"]    = tos( shield.shield2fb.frontmax );
                        unit["Shield_Back_Top_Right"]     = tos( shield.shield2fb.backmax );
                        break;
                        //NOTE: otherwise, no shields
                    }
                }
                unit["Shield_Leak"] = tos( shield.leak/100.0 );
                unit["Shield_Recharge"] = tos( shield.recharge );
                unit["Shield_Efficiency"] = tos( shield.efficiency );
                unit["Warp_Capacitor"] = tos( maxwarpenergy );
                unit["Warp_Min_Multiplier"] = tos( graphicOptions.MinWarpMultiplier );
                unit["Warp_Max_Multiplier"] = tos( graphicOptions.MaxWarpMultiplier );
                unit["Primary_Capacitor"] = tos( maxenergy );
                unit["Reactor_Recharge"] = tos( recharge );
                unit["Jump_Drive_Present"] = tos( jump.drive >= -1 );
                unit["Jump_Drive_Delay"] = tos( jump.delay );
                unit["Wormhole"] = tos( forcejump != 0 );
                unit["Outsystem_Jump_Cost"] = tos( jump.energy );
                unit["Warp_Usage_Cost"] = tos( jump.insysenergy );
                unit["Afterburner_Usage_Cost"]        = tos( afterburnenergy );
                unit["Afterburner_Type"]              = tos( afterburntype );
                unit["Maneuver_Yaw"]                  = tos( limits.yaw*180/(VS_PI) );
                unit["Maneuver_Pitch"]                = tos( limits.pitch*180/(VS_PI) );
                unit["Maneuver_Roll"]                 = tos( limits.roll*180/(VS_PI) );
                unit["Yaw_Governor_Right"]            = tos( computer.max_yaw_right*180/VS_PI );
                unit["Yaw_Governor_Left"]             = tos( computer.max_yaw_left*180/VS_PI );
                unit["Pitch_Governor_Up"]             = tos( computer.max_pitch_up*180/VS_PI );
                unit["Pitch_Governor_Down"]           = tos( computer.max_pitch_down*180/VS_PI );
                unit["Roll_Governor_Right"]           = tos( computer.max_roll_right*180/VS_PI );
                unit["Roll_Governor_Left"]            = tos( computer.max_roll_left*180/VS_PI );
                static float game_accel = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_accel", "1" ) );
                static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
                unit["Afterburner_Accel"]             = tos( limits.afterburn/(game_accel*game_speed) );
                unit["Forward_Accel"]                 = tos( limits.forward/(game_accel*game_speed) );
                unit["Retro_Accel"]                   = tos( limits.retro/(game_accel*game_speed) );
                unit["Left_Accel"]                    = unit["Right_Accel"] = tos( limits.lateral/(game_accel*game_speed) );
                unit["Bottom_Accel"]                  = unit["Top_Accel"] = tos( limits.vertical/(game_accel*game_speed) );
                unit["Default_Speed_Governor"]        = tos( computer.max_combat_speed/game_speed );
                unit["Afterburner_Speed_Governor"]    = tos( computer.max_combat_ab_speed/game_speed );
                unit["ITTS"] = tos( computer.itts );
                unit["Can_Lock"]                      = tos( computer.radar.canlock );
                unit["Radar_Color"]                   = tos( computer.radar.capability );
                unit["Radar_Range"]                   = tos( computer.radar.maxrange );
                unit["Tracking_Cone"]                 = tos( acos( computer.radar.trackingcone )*180./VS_PI );
                unit["Max_Cone"]                      = tos( acos( computer.radar.maxcone )*180./VS_PI );
                unit["Lock_Cone"]                     = tos( acos( computer.radar.lockcone )*180./VS_PI );
                unit["Cloak_Min"]                     = tos( cloakmin/2147483136. );
                unit["Can_Cloak"]                     = tos( cloaking != -1 );
                unit["Cloak_Rate"]                    = tos( fabs( cloakrate/2147483136. ) );
                unit["Cloak_Energy"]                  = tos( cloakenergy );
                unit["Cloak_Glass"]                   = tos( cloakglass );
                unit["Repair_Droid"]                  = tos( repair_droid );
                unit["ECM_Rating"]                    = tos( ecm > 0 ? ecm : -ecm );
                unit["Hud_Functionality"]             = WriteHudDamage( this );
                unit["Max_Hud_Functionality"]         = WriteHudDamageFunc( this );
                unit["Heat_Sink_Rating"]              = tos( this->HeatSink );
                unit["Lifesupport_Functionality"]     = tos( LifeSupportFunctionality );
                unit["Max_Lifesupport_Functionality"] = tos( LifeSupportFunctionalityMax );
                unit["Comm_Functionality"]            = tos( CommFunctionality );
                unit["Max_Comm_Functionality"]        = tos( CommFunctionalityMax );
                unit["Comm_Functionality"]            = tos( CommFunctionality );
                unit["Max_Comm_Functionality"]        = tos( CommFunctionalityMax );
                unit["FireControl_Functionality"]     = tos( fireControlFunctionality );
                unit["Max_FireControl_Functionality"] = tos( fireControlFunctionalityMax );
                unit["SPECDrive_Functionality"]       = tos( SPECDriveFunctionality );
                unit["Max_SPECDrive_Functionality"]   = tos( SPECDriveFunctionalityMax );
                unit["Slide_Start"]                   = tos( computer.slide_start );
                unit["Slide_End"]                     = tos( computer.slide_end );
                unit["Cargo_Import"]                  = unit["Upgrades"] = "";                 //make sure those are empty
                {
                    std::string trac;
                    if ( isTractorable( tractorPush ) ) trac += "p";
                    if ( isTractorable( tractorIn ) ) trac += "i";
                    if ( trac.empty() ) trac = "-";
                    unit["Tractorability"] = trac;
                }
                vector< string >keys, values;
                keys.push_back( "Key" );
                values.push_back( csvRow );                 //key has to come first
                mapToStringVec( unit, keys, values );
                return writeCSV( keys, values );
            }
        }
        BOOST_LOG_TRIVIAL(error) << boost::format("Failed to locate base mesh for %1% %2% %3%") % csvRow.get().c_str() % name.get().c_str() % fullname.c_str();
    } else {
        if (pImage->unitwriter) {
            ret = pImage->unitwriter->WriteString();
        }
        for (un_iter ui = getSubUnits(); (*ui) != NULL; ++ui) {
            ret = ret+( (*ui)->WriteUnitString() );
        }
    }
    return ret;
}

void UpdateMasterPartList( Unit *ret )
{
    for (unsigned int i = 0; i < _Universe->numPlayers(); ++i) {
        Cockpit *cp = _Universe->AccessCockpit( i );
        std::vector< std::string > *addedcargoname  = &cp->savegame->getMissionStringData( "master_part_list_content" );
        std::vector< std::string > *addedcargocat   = &cp->savegame->getMissionStringData( "master_part_list_category" );
        std::vector< std::string > *addedcargovol   = &cp->savegame->getMissionStringData( "master_part_list_volume" );
        std::vector< std::string > *addedcargoprice = &cp->savegame->getMissionStringData( "master_part_list_price" );
        std::vector< std::string > *addedcargomass  = &cp->savegame->getMissionStringData( "master_part_list_mass" );
        std::vector< std::string > *addedcargodesc  = &cp->savegame->getMissionStringData( "master_part_list_description" );
        for (unsigned int j = 0; j < addedcargoname->size(); ++j) {
            Cargo carg;
            carg.content     = (*addedcargoname)[j];
            carg.category    = ( j < addedcargocat->size() ? (*addedcargocat)[j] : std::string( "Uncategorized" ) );
            carg.volume      = (j < addedcargovol->size() ? XMLSupport::parse_float( (*addedcargovol)[j] ) : 1.0);
            carg.price       = (j < addedcargoprice->size() ? XMLSupport::parse_float( (*addedcargoprice)[j] ) : 0.0);
            carg.mass        = (j < addedcargomass->size() ? XMLSupport::parse_float( (*addedcargomass)[j] ) : .01);
            carg.description = ( j < addedcargodesc->size() ? (*addedcargodesc)[j] : std::string( "No Description Added" ) );
            carg.quantity    = 1;
            ret->cargo.push_back( carg );
        }
    }
    std::sort( ret->cargo.begin(), ret->cargo.end() );
    {
        Cargo last_cargo;
        for (int i = ret->numCargo()-1; i >= 0; --i) {
            if (ret->GetCargo( i ).content == last_cargo.content
                && ret->GetCargo( i ).category == last_cargo.category)
                ret->RemoveCargo( i, ret->GetCargo( i ).quantity, true );
            else
                last_cargo = ret->GetCargo( i );
        }
    }
}



