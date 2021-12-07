/**
 * universe_util_generic.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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


#include <math.h>
#include <sys/stat.h>
#include "lin_time.h"
#include "cmd/script/mission.h"
#include "universe_util.h"
#include "cmd/unit_generic.h"
#include "cmd/collection.h"
#include "star_system_generic.h"
#include <string>
#include <set>
#include "savegame.h"
#include "save_util.h"
#include "cmd/unit_csv.h"
#include "gfx/cockpit_generic.h"
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "cmd/unit_util.h"
#include "cmd/csv.h"
#include "linecollide.h"
#include "cmd/unit_collide.h"
#include "cmd/unit_find.h"
#include "unit.h"

#include "python/init.h"
#include <Python.h>
#include "options.h"

#include "star_system.h"
#include "universe.h"

#include <boost/filesystem.hpp>
#include <boost/chrono/time_point.hpp>
#include <boost/chrono/io/time_point_io.hpp>
#include <boost/chrono/chrono.hpp>

#include <iostream>

#include <sstream>
#include <chrono>
#include <locale>
#include <cstdint>

extern Unit& GetUnitMasterPartList();
extern int num_delayed_missions();
using std::string;
using std::set;

//less to write
#define activeSys _Universe->activeStarSystem()
using namespace VSFileSystem;

namespace UniverseUtil
{

void PythonUnitIter::advanceSignificant()
{
    advance();
    while ( !isDone() && !UnitUtil::isSignificant( **this ) )
        advance();
}

void PythonUnitIter::advanceInsignificant()
{
    advance();
    while ( !isDone() && UnitUtil::isSignificant( **this ) )
        advance();
}

void PythonUnitIter::advancePlanet()
{
    advance();
    while ( !isDone() && !(**this)->isPlanet() )
        advance();
}

void PythonUnitIter::advanceJumppoint()
{
    advance();
    while ( !isDone() && !(**this)->isJumppoint() )
        advance();
}

void PythonUnitIter::advanceN( int n )
{
    while (!isDone() && n > 0) {
        advance();
        --n;
    }
}

void PythonUnitIter::advanceNSignificant( int n )
{
    if ( !isDone() && !UnitUtil::isSignificant( **this ) )
        advanceSignificant();
    while ( !isDone() && (n > 0) ) {
        advanceSignificant();
        --n;
    }
}

void PythonUnitIter::advanceNInsignificant( int n )
{
    if ( !isDone() && UnitUtil::isSignificant( **this ) )
        advanceInsignificant();
    while ( !isDone() && (n > 0) ) {
        advanceInsignificant();
        --n;
    }
}

void PythonUnitIter::advanceNPlanet( int n )
{
    if ( !isDone() && !(**this)->isPlanet() )
        advancePlanet();
    while (!isDone() && n > 0) {
        advancePlanet();
        --n;
    }
}

void PythonUnitIter::advanceNJumppoint( int n )
{
    if ( !isDone() && !(**this)->isJumppoint() )
        advanceJumppoint();
    while (!isDone() && n > 0) {
        advanceJumppoint();
        --n;
    }
}


std::string vsConfig( std::string category, std::string option, std::string def )
{
    return vs_config->getVariable( category, option, def );
}

Unit * launchJumppoint( string name_string,
                        string faction_string,
                        string type_string,
                        string unittype_string,
                        string ai_string,
                        int nr_of_ships,
                        int nr_of_waves,
                        QVector pos,
                        string squadlogo,
                        string destinations )
{
    int clstype = _UnitType::unit;
    if (unittype_string == "planet")
        clstype = _UnitType::planet;
    else if (unittype_string == "asteroid")
        clstype = _UnitType::asteroid;
    else if (unittype_string == "nebula")
        clstype = _UnitType::nebula;
    CreateFlightgroup cf;
    cf.fg = Flightgroup::newFlightgroup( name_string,
                                         type_string,
                                         faction_string,
                                         ai_string,
                                         nr_of_ships,
                                         nr_of_waves,
                                         squadlogo,
                                         "",
                                         mission );
    cf.unittype   = CreateFlightgroup::UNIT;
    cf.terrain_nr = -1;
    cf.waves = nr_of_waves;
    cf.nr_ships   = nr_of_ships;
    cf.fg->pos    = pos;
    cf.rot[0]     = cf.rot[1] = cf.rot[2] = 0.0f;
    Unit *tmp = mission->call_unit_launch( &cf, clstype, destinations );
    mission->number_of_ships += nr_of_ships;

    return tmp;
}
Cargo getRandCargo( int quantity, string category )
{
    Cargo *ret = NULL;
    Unit  *mpl = &GetUnitMasterPartList();
    unsigned int max = mpl->numCargo();
    if ( !category.empty() ) {
        size_t Begin, End;
        mpl->GetSortedCargoCat( category, Begin, End );
        if (Begin < End) {
            unsigned int i = Begin+( rand()%(End-Begin) );
            ret = &mpl->GetCargo( i );
        } else {
            BOOST_LOG_TRIVIAL(info) << boost::format("Cargo category %1% not found") % category;
        }
    } else if ( mpl->numCargo() ) {
        for (unsigned int i = 0; i < 500; ++i) {
            ret = &mpl->GetCargo( rand()%max );
            if (ret->GetContent().find( "mission" ) == string::npos)
                break;
        }
    }
    if (ret) {
        Cargo tempret = *ret;
        tempret.quantity = quantity;
        return tempret;                          //uses copy
    } else {
        Cargo newret;
        newret.quantity = 0;
        return newret;
    }
}

float GetGameTime()
{
    return mission->getGametime();
}

float getStarTime()
{
    return (float)_Universe->current_stardate.GetCurrentStarTime();
}

string getStarDate()
{
    return _Universe->current_stardate.GetFullTrekDate();
}

void SetTimeCompression()
{
    setTimeCompression( 1.0 );
}
static UnitContainer scratch_unit;
static QVector scratch_vector;

Unit * GetMasterPartList()
{
    return getMasterPartList();
}
Unit * getScratchUnit()
{
    return scratch_unit.GetUnit();
}
void setScratchUnit( Unit *un )
{
    scratch_unit.SetUnit( un );
}

QVector getScratchVector()
{
    return scratch_vector;
}
void setScratchVector( QVector un )
{
    scratch_vector = un;
}

void pushSystem( string name )
{
    StarSystem *ss = _Universe->GenerateStarSystem( name.c_str(), "", Vector( 0, 0, 0 ) );
    _Universe->pushActiveStarSystem( ss );
}
void popSystem()
{
    _Universe->popActiveStarSystem();
}
string getSystemFile()
{
    if (!activeSys) return "";
    return activeSys->getFileName();
}

string getSystemName()
{
    if (!activeSys) return "";
    return activeSys->getName();
}
///tells the respective flightgroups in this system to start shooting at each other
void TargetEachOther( string fgname, string faction, string enfgname, string enfaction )
{
    int   fac   = FactionUtil::GetFactionIndex( faction );
    int   enfac = FactionUtil::GetFactionIndex( enfaction );
    Unit *un;
    Unit *en    = NULL;
    Unit *al    = NULL;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *i) && ( (!en) || (!al) ); ++i) {
        if (un->faction == enfac && UnitUtil::getFlightgroupName( un ) == enfgname)
            if ( (NULL == en) || (rand()%3 == 0) )
                en = un;
        if (un->faction == fac && UnitUtil::getFlightgroupName( un ) == fgname)
            al = un;
    }
    if (en && al) {
        UnitUtil::setFlightgroupLeader( al, al );
        al->Target( en );
        //attack target, darent change target!
        UnitUtil::setFgDirective( al, "A." );
        UnitUtil::setFlightgroupLeader( en, en );
        en->Target( al );
        //help me out here!
        UnitUtil::setFgDirective( en, "h" );
    }
}

///tells the respective flightgroups in this system to stop killing each other urgently...they may still attack--just not warping and stuff
void StopTargettingEachOther( string fgname, string faction, string enfgname, string enfaction )
{
    int   fac   = FactionUtil::GetFactionIndex( faction );
    int   enfac = FactionUtil::GetFactionIndex( enfaction );
    Unit *un;
    int   clear = 0;
    for (un_iter i = _Universe->activeStarSystem()->getUnitList().createIterator(); (un = *i) && clear != 3; ++i) {
        if ( (un->faction == enfac && UnitUtil::getFlightgroupName( un ) == enfgname) ) {
            clear |= 1;
            UnitUtil::setFgDirective( un, "b" );
        } else if (un->faction == fac && UnitUtil::getFlightgroupName( un ) == fgname) {
            clear |= 2;
            UnitUtil::setFgDirective( un, "b" );
            //check to see that its' in this flightgroup or something :-)
        }
    }
}

bool systemInMemory( string nam )
{
    unsigned int nass = _Universe->star_system.size();
    for (unsigned int i = 0; i < nass; ++i)
        if (_Universe->star_system[i]->getFileName() == nam)
            return true;
    return false;
}

float GetRelation( std::string myfaction, std::string theirfaction )
{
    int   myfac    = FactionUtil::GetFactionIndex( myfaction );
    int   theirfac = FactionUtil::GetFactionIndex( theirfaction );
    int   cp = _Universe->CurrentCockpit();
    Unit *un = _Universe->AccessCockpit()->GetParent();
    if (!un) return FactionUtil::GetIntRelation( myfac, theirfac );
    if (myfac == theirfac)
        return 0;
    else if (myfac == un->faction)
        return getRelationModifierInt( cp, theirfac );
    else if (theirfac == un->faction)
        return getRelationModifierInt( cp, myfac );
    else
        return FactionUtil::GetIntRelation( myfac, theirfac );
}
void AdjustRelation( std::string myfaction, std::string theirfaction, float factor, float rank )
{
    int   myfac = FactionUtil::GetFactionIndex( myfaction );
    int   theirfac   = FactionUtil::GetFactionIndex( theirfaction );
    float realfactor = factor*rank;
    int   cp = _Universe->CurrentCockpit();
    Unit *un = _Universe->AccessCockpit()->GetParent();
    if (!un) return;
    if (myfac == theirfac)
        return;
    else if (myfac == un->faction)
        return adjustRelationModifierInt( cp, theirfac, realfactor );
    else if (theirfac == un->faction)
        return adjustRelationModifierInt( cp, myfac, realfactor );
}

float getRelationModifierInt( int which_cp, int faction )
{
    string saveVar = "Relation_to_"+FactionUtil::GetFactionName( faction );
    if (getSaveDataLength( which_cp, saveVar ) == 0)
        return 0.;
    float  val     = getSaveData( which_cp, saveVar, 0 );
    if (val > 1) val = 1;
    return val;
}
float getRelationModifier( int which_cp, string faction )
{
    return getRelationModifierInt( which_cp, FactionUtil::GetFactionIndex( faction ) );
}
float getFGRelationModifier( int which_cp, string fg )
{
    fg = "FG_Relation_"+fg;
    if (getSaveDataLength( which_cp, fg ) == 0)
        return 0.;
    return getSaveData( which_cp, fg, 0 );
}
void adjustRelationModifierInt( int which_cp, int faction, float delta )
{
    if (delta > 1) delta = 1;
    string saveVar = "Relation_to_"+FactionUtil::GetFactionName( faction );
    if (getSaveDataLength( which_cp, saveVar ) == 0)
        pushSaveData( which_cp, saveVar, delta );
    float  val     = getSaveData( which_cp, saveVar, 0 )+delta;
    if (val > 1) val = 1;
    return putSaveData( which_cp, saveVar, 0, val );
}
void adjustRelationModifier( int which_cp, string faction, float delta )
{
    adjustRelationModifierInt( which_cp, FactionUtil::GetFactionIndex( faction ), delta );
}
void adjustFGRelationModifier( int which_cp, string fg, float delta )
{
    fg = "FG_Relation_"+fg;
    if (getSaveDataLength( which_cp, fg ) == 0) {
        pushSaveData( which_cp, fg, delta );
        return;
    }
    putSaveData( which_cp, fg, 0, getSaveData( which_cp, fg, 0 )+delta );
}

void setMissionOwner( int whichplayer )
{
    mission->player_num = whichplayer;
}
int getMissionOwner()
{
    return mission->player_num;
}
UniverseUtil::PythonUnitIter getUnitList()
{
    return activeSys->getUnitList().createIterator();
}
Unit * getUnit( int index )
{
    un_iter iter = activeSys->getUnitList().createIterator();
    Unit   *un   = NULL;
    for (int i = -1; (un = *iter) && i < index; ++iter) {
        if (un->GetHull() > 0)
            ++i;
        if (i == index)
            break;
    }
    return un;
}
Unit * getUnitByPtr( void *ptr, Unit *finder, bool allowslowness )
{
    if (finder) {
        UnitPtrLocator unitLocator( ptr );
        findObjects( activeSys->collide_map[Unit::UNIT_ONLY], finder->location[Unit::UNIT_ONLY], &unitLocator );
        if (unitLocator.retval)
            return reinterpret_cast< Unit* > (ptr);

        else if ( !finder->isSubUnit() )
            return 0;
    }
    if (!allowslowness)
        return 0;
    return ( (activeSys->getUnitList().contains((Unit*)ptr)) ? reinterpret_cast< Unit* > (ptr) : NULL);
}
Unit * getUnitByName( std::string name )
{
    un_iter iter = activeSys->getUnitList().createIterator();
    while (!iter.isDone() && UnitUtil::getName( *iter ) != name)
        ++iter;
    return ((!iter.isDone()) ? (*iter) : NULL);
}
int getNumUnits()
{
#ifdef USE_STL_COLLECTION
    return activeSys->getUnitList().size();

#else
    //Implentation-safe getNumUnits().
    int     count = 0;
    un_iter iter  = activeSys->getUnitList().createIterator();
    while ( iter.current() ) {
        iter.advance();
        count++;
    }
    return count;
#endif
}
//NOTEXPORTEDYET
/*
 *  float GetGameTime () {
 *       return mission->gametime;
 *  }
 *  void SetTimeCompression () {
 *       setTimeCompression(1.0);
 *  }
 */
string GetAdjacentSystem( string str, int which )
{
    return _Universe->getAdjacentStarSystems( str )[which];
}
string GetGalaxyProperty( string sys, string prop )
{
    return _Universe->getGalaxyProperty( sys, prop );
}
string GetGalaxyPropertyDefault( string sys, string prop, string def )
{
    return _Universe->getGalaxyPropertyDefault( sys, prop, def );
}
#define DEFAULT_FACTION_SAVENAME "FactionTookOver_"

string GetGalaxyFaction( string sys )
{
    string fac = _Universe->getGalaxyProperty( sys, "faction" );
    vector< std::string > *ans =
        &( _Universe->AccessCockpit( 0 )->savegame->getMissionStringData( string( DEFAULT_FACTION_SAVENAME )+sys ) );
    if ( ans->size() )
        fac = (*ans)[0];
    return fac;
}
void SetGalaxyFaction( string sys, string fac )
{
    vector< std::string > *ans =
        &( _Universe->AccessCockpit( 0 )->savegame->getMissionStringData( string( DEFAULT_FACTION_SAVENAME )+sys ) );
    if ( ans->size() )
        (*ans)[0] = fac;
    else
        ans->push_back( std::string( fac ) );
}
int GetNumAdjacentSystems( string sysname )
{
    return _Universe->getAdjacentStarSystems( sysname ).size();
}
float GetDifficulty()
{
    return g_game.difficulty;
}
void SetDifficulty( float diff )
{
    g_game.difficulty = diff;
}
extern void playVictoryTune();
void terminateMission( bool Win )
{
    if (Win)
        playVictoryTune();
    mission->terminateMission();
}
static string dontBlankOut( string objective )
{
    while (1) {
        std::string::size_type where = objective.find( ".blank" );
        if (where != string::npos)
            objective = objective.substr( 0, where )+objective.substr( where+strlen( ".blank" ) );
        else return objective;
    }
    return objective;
}
int addObjective( string objective )
{
    float status = 0;

    mission->objectives.push_back( Mission::Objective( status, dontBlankOut( objective ) ) );
    return mission->objectives.size()-1;
}
void setObjective( int which, string newobjective )
{
    if (which < (int) mission->objectives.size() && which >= 0) {
        mission->objectives[which].objective = dontBlankOut( newobjective );
    }
}
void setCompleteness( int which, float completeNess )
{
    if (which < (int) mission->objectives.size() && which >= 0) {
        mission->objectives[which].completeness = completeNess;
    }
}
float getCompleteness( int which )
{
    if (which < (int) mission->objectives.size() && which >= 0)
        return mission->objectives[which].completeness;
    else
        return 0;
}
void setTargetLabel( std::string label )
{
    _Universe->AccessCockpit()->setTargetLabel( label );
}
std::string getTargetLabel()
{
    return _Universe->AccessCockpit()->getTargetLabel();
}
void eraseObjective( int which )
{
    if (which < (int) mission->objectives.size() && which >= 0) {
        mission->objectives.erase( mission->objectives.begin()+which );
    }
}
void clearObjectives()
{
    if ( mission->objectives.size() ) {
        mission->objectives.clear();
    }
}
void setOwnerII( int which, Unit *owner )
{
    if ( which < (int) mission->objectives.size() )
        mission->objectives[which].setOwner( owner );
}
Unit * getOwner( int which )
{
    if ( which < (int) mission->objectives.size() )
        return mission->objectives[which].getOwner();
    else
        return 0;
}
int numActiveMissions()
{
    int num = 0;
    unsigned int cp = _Universe->CurrentCockpit();
    for (unsigned int i = 0; i < active_missions.size(); ++i)
        if (active_missions[i]->player_num == cp)
            num++;
    return num+::num_delayed_missions();
}
void IOmessage( int delay, string from, string to, string message )
{
    if ( to == "news" && (!game_options.news_from_cargolist) )
        for (unsigned int i = 0; i < _Universe->numPlayers(); i++)
            pushSaveString( i, "news", string( "#" )+message );
    else
        mission->msgcenter->add( from, to, message, delay );
}
Unit * GetContrabandList( string faction )
{
    return FactionUtil::GetContraband( FactionUtil::GetFactionIndex( faction ) );
}
void LoadMission( string missionname )
{
    ::LoadMission( missionname.c_str(), "", false );
}

void LoadNamedMissionScript( string title, string missionscript )
{
    ::LoadMission( ("#"+title).c_str(), missionscript, false );
}

void LoadMissionScript( string missionscript )
{
    ::LoadMission( "nothing.mission", missionscript, false );
}

void SetAutoStatus( int global_auto, int player_auto )
{
    if (global_auto == 1)
        mission->global_autopilot = Mission::AUTO_ON;
    else if (global_auto == -1)
        mission->global_autopilot = Mission::AUTO_OFF;
    else
        mission->global_autopilot = Mission::AUTO_NORMAL;
    if (player_auto == 1)
        mission->player_autopilot = Mission::AUTO_ON;
    else if (player_auto == -1)
        mission->player_autopilot = Mission::AUTO_OFF;
    else
        mission->player_autopilot = Mission::AUTO_NORMAL;
}
QVector SafeStarSystemEntrancePoint( StarSystem *sts, QVector pos, float radial_size )
{
    if (radial_size < 0)
        radial_size = game_options.respawn_unit_size;
    for (unsigned int k = 0; k < 10; ++k) {
        Unit *un;
        bool  collision = false;
        {
            //fixme, make me faster, use collide map
            for (un_iter i = sts->getUnitList().createIterator(); (un = *i) != NULL; ++i) {
                if (UnitUtil::isAsteroid( un ) || un->isUnit() == _UnitType::nebula)
                    continue;
                double dist = ( pos-un->LocalPosition() ).Magnitude()-un->rSize()-/*def_un_size-*/ radial_size;
                if (dist < 0) {
                    QVector delta = pos-un->LocalPosition();
                    double  mag   = delta.Magnitude();
                    if (mag > .01)
                        delta = delta/mag;
                    else
                        delta.Set( 0, 0, 1 );
                    delta = delta.Scale( dist+un->rSize()+radial_size );
                    if (k < 5) {
                        pos = pos+delta;
                        collision = true;
                    } else {
                        QVector r( .5, .5, .5 );
                        pos += ( radial_size+un->rSize() )*r;
                        collision = true;
                    }
                }
            }
            if (collision == false)
                break;
        }
    }
    return pos;
}
QVector SafeEntrancePoint( QVector pos, float radial_size )
{
    return SafeStarSystemEntrancePoint( _Universe->activeStarSystem(), pos, radial_size );
}
Unit * launch( string name_string,
               string type_string,
               string faction_string,
               string unittype,
               string ai_string,
               int nr_of_ships,
               int nr_of_waves,
               QVector pos,
               string sqadlogo )
{
    return launchJumppoint( name_string,
                            faction_string,
                            type_string,
                            unittype,
                            ai_string,
                            nr_of_ships,
                            nr_of_waves,
                            pos,
                            sqadlogo,
                            "" );
}

string LookupUnitStat( const string &unitname, const string &faction, const string &statname )
{
    CSVRow tmp( LookupUnitRow( unitname, faction ) );
    if ( tmp.success() )
        return tmp[statname];

    else
        return string();
}

static std::vector< Unit* >cachedUnits;
void precacheUnit( string type_string, string faction_string )
{
    cachedUnits.push_back( new GameUnit( type_string.c_str(), true, FactionUtil::GetFactionIndex( faction_string ) ) );
}
Unit * getPlayer()
{
    return _Universe->AccessCockpit()->GetParent();
}
bool networked()
{
    return false;
}
bool isserver()
{
    return false;
}
void securepythonstr( string &message )
{
    std::replace( message.begin(), message.end(), '\'', '\"' );
    std::replace( message.begin(), message.end(), '\\', '/' );
    std::replace( message.begin(), message.end(), '\n', ' ' );
    std::replace( message.begin(), message.end(), '\r', ' ' );
}
void receivedCustom( int cp, bool trusted, string cmd, string args, string id )
{
    int cp_orig = _Universe->CurrentCockpit();
    _Universe->SetActiveCockpit( cp );
    _Universe->pushActiveStarSystem( _Universe->AccessCockpit()->activeStarSystem );
    securepythonstr( cmd );
    securepythonstr( args );
    securepythonstr( id );
    string pythonCode = game_options.custompython+"("+(trusted ? "True" : "False")
                        +", r\'"+cmd+"\', r\'"+args+"\', r\'"+id+"\')\n";
    BOOST_LOG_TRIVIAL(info) << "Executing python command: ";
    BOOST_LOG_TRIVIAL(info) << "    " << pythonCode;
    const char *cpycode = pythonCode.c_str();
    ::Python::reseterrors();
    PyRun_SimpleString( const_cast< char* > (cpycode) );
    ::Python::reseterrors();
    _Universe->popActiveStarSystem();
    _Universe->SetActiveCockpit( cp_orig );
}
int getNumPlayers()
{
    return _Universe->numPlayers();
}
Unit * getPlayerX( int which )
{
    if ( which >= getNumPlayers() )
        return NULL;
    return _Universe->AccessCockpit( which )->GetParent();
}
float getPlanetRadiusPercent()
{
    return game_options.auto_pilot_planet_radius_percent;
}
std::string getVariable( std::string section, std::string name, std::string def )
{
    return vs_config->getVariable( section, name, def );
}
std::string getSubVariable( std::string section, std::string subsection, std::string name, std::string def )
{
    return vs_config->getVariable( section, subsection, name, def );
}
double timeofday()
{
    return getNewTime();
}
double sqrt( double x )
{
    return ::sqrt( x );
}
double log( double x )
{
    return ::log( x );
}
double exp( double x )
{
    return ::exp( x );
}
double cos( double x )
{
    return ::cos( x );
}
double sin( double x )
{
    return ::sin( x );
}
double acos( double x )
{
    return ::acos( x );
}
double asin( double x )
{
    return ::asin( x );
}
double atan( double x )
{
    return ::atan( x );
}
double tan( double x )
{
    return ::tan( x );
}
void micro_sleep( int n )
{
    ::micro_sleep( n );
}


string getSaveDir()
{
    return GetSaveDir();
}

static std::string simplePrettySystem( std::string system )
{
    std::string::size_type where = system.find_first_of( '/' );
    std::string::size_type basewhere = system.find_first_of( '@', where );
    return std::string( "Sec:" )+system.substr( 0, where )
           +" Sys:"+( where == string::npos ? std::string("") : system.substr( where+1, (basewhere!=string::npos) ? basewhere-where-1 : string::npos ) )
           +( basewhere == string::npos ? std::string("") : std::string(" ")+system.substr( basewhere+1 ) );
}
static std::string simplePrettyShip( std::string ship )
{
    if (ship.length() > 0)
        ship[0] = toupper( ship[0] );
    std::string::size_type where = ship.find( "." );
    if (where != string::npos) {
        ship = ship.substr( 0, where );
        ship = "Refurbished "+ship;
    }
    return ship;
}

string getSaveInfo( const std::string &filename, bool formatForTextbox )
{
    static SaveGame     savegame( "" );
    static set< string >campaign_score_vars;
    static bool campaign_score_vars_init = false;
    if (!campaign_score_vars_init) {

        string::size_type where = 0, when = game_options.campaigns.find( ' ' );
        while (where != string::npos) {
            campaign_score_vars.insert( game_options.campaigns.substr( where, ( (when == string::npos) ? when : when-where ) ) );
            where = (when == string::npos) ? when : when+1;
            when  = game_options.campaigns.find( ' ', where );
        }
        campaign_score_vars_init = true;
    }
    std::string system;
    std::string lf  = (formatForTextbox ? "#n#" : "\n");
    QVector     pos( 0, 0, 0 );
    bool  updatepos = false;
    float creds;
    vector< std::string >Ships;
    std::string sillytemp = UniverseUtil::setCurrentSaveGame( filename );
    savegame.SetStarSystem( "" );
    savegame.ParseSaveGame( filename, system, "", pos, updatepos, creds, Ships,
                            _Universe->CurrentCockpit(), "", true, false, game_options.quick_savegame_summaries, true, true,
                            campaign_score_vars );
    UniverseUtil::setCurrentSaveGame( sillytemp );
    std::ostringstream ss{};
    ss << "Savegame: " << filename << lf;
    ss << "_________________" << lf;
    try
    {
        ss << "Saved on: ";
        const boost::filesystem::path file_name_path{filename};
        const boost::filesystem::path save_dir_path{getSaveDir()};
        const boost::filesystem::path full_file_path{boost::filesystem::absolute(file_name_path, save_dir_path)};
        std::time_t last_saved_time{boost::filesystem::last_write_time(full_file_path)};
        boost::chrono::system_clock::time_point last_saved_time_point{boost::chrono::system_clock::from_time_t(last_saved_time)};
        ss << boost::chrono::time_fmt(boost::chrono::timezone::local, "%c")
                << last_saved_time_point
                << lf;
    }
    catch (boost::filesystem::filesystem_error& fse)
    {
        BOOST_LOG_TRIVIAL(fatal) << "boost::filesystem::filesystem_error encountered:";
        BOOST_LOG_TRIVIAL(fatal) << fse.what();
        VSExit(-6);
    }
    catch (std::exception& e)
    {
        BOOST_LOG_TRIVIAL(fatal) << "std::exception encountered:";
        BOOST_LOG_TRIVIAL(fatal) << e.what();
        VSExit(-6);
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(fatal) << "unknown exception type encountered!";
        VSExit(-6);
    }
    ss << "Credits: "
            << static_cast<int64_t>(creds)
            << "."
            << (static_cast<int64_t>(creds * 100) % 100)
            << lf;
    ss << simplePrettySystem( system ) << lf;
    if ( Ships.size() ) {
        ss << "Starship: " << simplePrettyShip( Ships[0] ) << lf;
        if (Ships.size() > 2) {
            ss << "Fleet:" << lf;
            for (size_t i = 2; i < Ships.size(); i += 2) {
                ss << " " << simplePrettyShip( Ships[i-1] ) << lf;
                ss << "  Located At:" << lf;
                ss << "  " << simplePrettySystem( Ships[i] ) << lf;
            }
        }
    }
    if (!game_options.quick_savegame_summaries) {
        bool hit = false;
        for (set< string >::const_iterator it = campaign_score_vars.begin(); it != campaign_score_vars.end(); ++it) {
            string var = *it;
            int64_t curscore = savegame.getMissionData( var ).size() + savegame.getMissionStringData( var ).size();
            if (curscore > 0) {
                hit   = true;
                if (var.length() > 0) {
                    var[0] = toupper( var[0] );
                }
                ss << var.substr( 0, var.find( "_" ) ) + " Campaign Score: " << curscore << lf;
            }
        }
        if (!hit) {
            ss << "Campaign Score: 0" << lf;
        }
    }
    return ss.str();
}

string getCurrentSaveGame()
{
    return GetCurrentSaveGame();
}

string setCurrentSaveGame( const string &newsave )
{
    return SetCurrentSaveGame( newsave );
}

string getNewGameSaveName()
{
    static string ngsn( "New_Game" );
    return ngsn;
}

vector< string > GetJumpPath( string from, string to )
{
    vector< string > path;
    _Universe->getJumpPath(from, to, path);
    return path;
}

}

#undef activeSys
