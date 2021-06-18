/**
 * universe.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn and Alan Shieh
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


#include <stdio.h>
#include <fcntl.h>
#include "gfxlib.h"
#include "universe.h"
#include "lin_time.h"
#include "in.h"
#include "gfx/aux_texture.h"
#include "profile.h"
#include "gfx/cockpit.h"
#include "galaxy_xml.h"
#include <algorithm>
#include "config_xml.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "audiolib.h"
#include "cmd/script/mission.h"
#include "cmd/unit.h"
#include "in_kb.h"
#include "in_kb_data.h"
#include "in_main.h"
#if defined (__APPLE__)
#import <sys/param.h>
#endif
#include "savegame.h"
#include "gfx/screenshot.h"
#include "universe_util.h"
#include "star_system.h"
#include "save_util.h"
#include "cmd/csv.h"
#include "cmd/role_bitmask.h"
#include "universe_globals.h"
#include "vsfilesystem.h"

#include "weapon_factory.h"

#include <algorithm>
#include <string>
#include <vector>

#include "options.h"

// Using
using namespace VSFileSystem;
using namespace GalaxyXML;

using std::string;
using std::vector;
using std::find;

// externs
extern void CacheJumpStar( bool );
extern void SortStarSystems( vector< StarSystem* > &ss, StarSystem *drawn );
extern void bootstrap_first_loop();
extern bool RefreshGUI();
extern float rand01();
extern int timecount;
extern StarSystem * GetLoadedStarSystem( const char *system );
extern bool screenshotkey;
extern int getmicrosleep();
extern void MakeStarSystem( string file, Galaxy *galaxy, string origin, int forcerandom );
extern string RemoveDotSystem( const char *input );
extern StarSystem * GetLoadedStarSystem( const char *file );


// Local Variables
vector< StarSystem* >deleteQueue;


// Local Functions
void CalculateCoords( unsigned int i, unsigned int size, float &x, float &y, float &w, float &h )
{
    if (size <= 1) {
        x = y = 0;
        w = h = 1;
        return;
    }
    if ( size <= 3 || i < (size/2) ) {
        y = 0;
        h = 1;
        w = 1./( (float) size );
        x = ( (float) i )/size;
        if (size > 3)
            h = .5;
    }
    if (size > 3) {
        if (i > size/2) {
            y = .5;
            h = .5;
            x = ( (float) i-(size/2) )/(size-size/2);
            w = 1/(size-size/2);
        }
    }
}



inline void loadsounds( const string &str, const int max, soundArray &snds, bool loop = false )
{
    char addstr[2] = {'\0'};
    snds.allocate( max );
    for (int i = 0; i < max; ++i) {
        addstr[0] = '1'+i;
        string mynewstr = str;
        while (1) {
            std::string::size_type found = mynewstr.find( '?' );
            if (found != std::string::npos) {
                mynewstr[found] = addstr[0];
            } else {
                break;
            }
        }
        snds.ptr[i].loadsound( mynewstr, loop );
    }
}

static void UpdateTimeCompressionSounds()
{
    static int lasttimecompress = 0;
    if ( (timecount != lasttimecompress) && (game_options.compress_max > 0) ) {
        static bool inittimecompresssounds = false;
        static soundArray loop_snds;
        static soundArray burst_snds;
        static soundArray end_snds;
        if (inittimecompresssounds == false) {
            loadsounds( game_options.compress_loop, game_options.compress_max, loop_snds, true );
            loadsounds( game_options.compress_stop, game_options.compress_max, end_snds );
            loadsounds( game_options.compress_change, game_options.compress_max, burst_snds );
            inittimecompresssounds = true;
        }
        int soundfile     = (timecount-1)/game_options.compress_interval;
        int lastsoundfile = (lasttimecompress-1)/game_options.compress_interval;
        if (timecount > 0 && lasttimecompress >= 0) {
            if ( (soundfile+1) >= game_options.compress_max ) {
                burst_snds.ptr[game_options.compress_max-1].playsound();
            } else {
                if ( lasttimecompress > 0 && loop_snds.ptr[lastsoundfile].sound >= 0
                    && AUDIsPlaying( loop_snds.ptr[lastsoundfile].sound ) )
                    AUDStopPlaying( loop_snds.ptr[lastsoundfile].sound );
                loop_snds.ptr[soundfile].playsound();
                burst_snds.ptr[soundfile].playsound();
            }
        } else if (lasttimecompress > 0 && timecount == 0) {
            for (int i = 0; i < game_options.compress_max; ++i)
                if ( loop_snds.ptr[i].sound >= 0 && AUDIsPlaying( loop_snds.ptr[i].sound ) )
                    AUDStopPlaying( loop_snds.ptr[i].sound );
            if (lastsoundfile >= game_options.compress_max)
                end_snds.ptr[game_options.compress_max-1].playsound();
            else
                end_snds.ptr[lastsoundfile].playsound();
        }
        lasttimecompress = timecount;
    }
}

Unit * DockToSavedBases( int playernum, QVector &safevec )
{
    string str = game_options.startDockedTo;
    Unit  *plr = _Universe->AccessCockpit( playernum )->GetParent();
    if ( !plr || !plr->getStarSystem() ) {
        safevec = QVector( 0, 0, 0 );
        return NULL;
    }
    vector< string >strs = loadStringList( playernum, mission_key );
    if ( strs.size() )
        str = strs[0];
    Unit   *closestUnit  = NULL;
    float   lastdist     = 0;
    float   dist = 0;
    Unit   *un;
    QVector dock_position( plr->curr_physical_state.position );
    for (un_iter iter = plr->getStarSystem()->getUnitList().createIterator(); (un = *iter); ++iter)
        if (un->name == str || un->getFullname() == str) {
            dist = UnitUtil::getSignificantDistance( plr, un );
            if (closestUnit == NULL || dist < lastdist) {
                lastdist    = dist;
                closestUnit = un;
            }
        }
    if (closestUnit) {
        if (UnitUtil::getSignificantDistance( plr, closestUnit ) > 0 && closestUnit->isUnit() != _UnitType::planet)
            dock_position = closestUnit->Position();
        dock_position = UniverseUtil::SafeEntrancePoint( dock_position, plr->rSize() );
        plr->SetPosAndCumPos( dock_position );

        vector< DockingPorts >dprt = closestUnit->pImage->dockingports;
        unsigned int i;
        for (i = 0;; i++) {
            if ( i >= dprt.size() ) {
                safevec = QVector( 0, 0, 0 );
                return NULL;
            }
            if (!dprt[i].IsOccupied())
                break;
        }
        plr->ForceDock( closestUnit, i );
        closestUnit->pImage->clearedunits.push_back( plr );
        closestUnit->RequestPhysics();
        _Universe->AccessCockpit( playernum )->retry_dock = 0;
    } else {
        if (_Universe->AccessCockpit( playernum )->retry_dock == 0)
            _Universe->AccessCockpit( playernum )->retry_dock = 128;
        else _Universe->AccessCockpit( playernum )->retry_dock -= 1;
    }
    safevec = dock_position;
    return ( closestUnit && closestUnit->isDocked( plr ) ) ? closestUnit : NULL;
}

void SortStarSystems( std::vector< StarSystem* > &ss, StarSystem *drawn )
{
    if ( ( *ss.begin() ) == drawn )
        return;
    vector< StarSystem* >::iterator drw = std::find( ss.begin(), ss.end(), drawn );
    if ( drw != ss.end() ) {
        StarSystem *tmp = drawn;
        vector< StarSystem* >::iterator i = ss.begin();
        while (i <= drw) {
            StarSystem *t = *i;
            *i  = tmp;
            tmp = t;
            i++;
        }
    }
}

static void ss_generating( bool enable )
{
    static bool ss_generating_active = false;
    if (enable) {
        if ( !UniverseUtil::isSplashScreenShowing() ) {
            static const std::string empty;
            UniverseUtil::showSplashScreen( empty );
            ss_generating_active = true;
        }
    } else if (ss_generating_active) {
        UniverseUtil::hideSplashScreen();
        ss_generating_active = false;
    }
}

static void AppendUnitTables(const string &csvfiles)
{
    CSVTable *table = loadCSVTableList(csvfiles, VSFileSystem::UnitFile, true);
    if (table != NULL)
        unitTables.push_back(table);
}

void InitUnitTables()
{
    AppendUnitTables(game_options.modUnitCSV);
    AppendUnitTables(game_options.unitCSV);
}

void CleanupUnitTables()
{
    for (std::vector<CSVTable*>::iterator it = unitTables.begin(); it != unitTables.end(); ++it)
    {
        delete *it;
    }
    unitTables.clear();
}



// Constructors and Init
Universe::Universe( int argc, char **argv, const char *galaxy_str)
{
    //Select drivers
#if defined (__APPLE__)
    //get the current working directory so when glut trashes it we can restore.
    char pwd[MAXPATHLEN];
    getcwd( pwd, MAXPATHLEN );
#endif
    GFXInit( argc, argv );
#if defined (__APPLE__)
    //Restore it
    chdir( pwd );
#endif
    StartGFX();
    InitInput();

    hud_camera = Camera();

    //Hasten splash screen loading, to cover up lengthy universe initialization
    bootstrap_first_loop();

    ROLES::getAllRolePriorities();
    //LoadWeapons( VSFileSystem::weapon_list.c_str() );
    WeaponFactory wf = WeaponFactory(VSFileSystem::weapon_list);

    galaxy.reset(new GalaxyXML::Galaxy( galaxy_str ));
    static bool firsttime = false;
    if (!firsttime) {
        LoadFactionXML( "factions.xml" );
        firsttime = true;
    }
    _script_system = NULL;
    _current_cockpit = 0;
    _script_system = nullptr;
}

Universe::Universe()
{
    _current_cockpit = 0;
    _script_system = nullptr;
}

Universe::~Universe()
{
    factions.clear();
    _cockpits.clear();
    DeInitInput();
    GFXShutdown();
}


StarSystem* Universe::Init( string systemfile, const Vector &centr, const string planetname )
{
    static bool js = true;
    if (js) {
        js = false;
        // LoadWeapons( VSFileSystem::weapon_list.c_str() );
        WeaponFactory wf = WeaponFactory(VSFileSystem::weapon_list);

        CacheJumpStar( false );
    }

    string fullname = systemfile+".system";
    return GenerateStarSystem( (char*) fullname.c_str(), "", centr );
}


// Gameplay Methods
void Universe::Loop( void main_loop() )
{
    GFXLoop( main_loop );
}

void Universe::WriteSaveGame( bool auto_save )
{
    for (unsigned int i = 0; i < _cockpits.size(); ++i) {
        if ( AccessCockpit( i ) ) {
            ::WriteSaveGame( AccessCockpit( i ), auto_save );
#if 0
            if ( AccessCockpit( i )->GetParent() )
                if (AccessCockpit( i )->GetParent()->GetHull() > 0) {
                    AccessCockpit( i )->savegame->WriteSaveGame( AccessCockpit(
                                                                    i )->activeStarSystem->getFileName().c_str(),
                                                                AccessCockpit( i )->GetParent()->Position(), AccessCockpit(
                                                                    i )->credits, AccessCockpit( i )->GetUnitFileName() );
                    AccessCockpit( i )->GetParent()->WriteUnit( AccessCockpit( i )->GetUnitModifications().c_str() );
                }

#endif

        }
    }
}


void Universe::StartDraw()
{
#ifndef WIN32
    RESETTIME();
#endif
    GFXBeginScene();
    size_t i;
    StarSystem  *lastStarSystem = NULL;
    for (i = 0; i < _cockpits.size(); ++i) {
        SetActiveCockpit( i );
        float x, y, w, h;
        CalculateCoords( i, _cockpits.size(), x, y, w, h );
        AccessCamera()->SetSubwindow( x, y, w, h );
        if (_cockpits.size() > 1 && AccessCockpit( i )->activeStarSystem != lastStarSystem) {
            _active_star_systems[0]->SwapOut();
            lastStarSystem = AccessCockpit()->activeStarSystem;
            _active_star_systems[0] = lastStarSystem;
            lastStarSystem->SwapIn();
        }
        AccessCockpit()->SelectProperCamera();
        if (_cockpits.size() > 0)
            AccessCamera()->UpdateGFX();
        if ( !RefreshGUI() && !UniverseUtil::isSplashScreenShowing() )
            activeStarSystem()->Draw();
        AccessCamera()->SetSubwindow( 0, 0, 1, 1 );
    }
    UpdateTime();
    UpdateTimeCompressionSounds();
    _Universe->SetActiveCockpit( ( (int) ( rand01()*_cockpits.size() ) )%_cockpits.size() );
    for (i = 0; i < star_system.size() && i < game_options.NumRunningSystems; ++i)
        star_system[i]->Update( (i == 0) ? 1 : game_options.InactiveSystemTime/i, true );
    StarSystem::ProcessPendingJumps();
    for (i = 0; i < _cockpits.size(); ++i) {
        SetActiveCockpit( i );
        pushActiveStarSystem( AccessCockpit( i )->activeStarSystem );
        ProcessInput( i );                       //input neesd to be taken care of;
        popActiveStarSystem();
    }
    if (screenshotkey) {
        KBData b;
        Screenshot( b, PRESS );
        screenshotkey = false;
    }
    GFXEndScene();
    //so we don't starve the audio thread
    micro_sleep( getmicrosleep() );

    //remove systems not recently visited?
    static int sorttime = 0;
    if (game_options.garbagecollectfrequency != 0) {
        //don't want to delete something when there is something pending to jump therexo
        if ( PendingJumpsEmpty() ) {
            if ( (++sorttime)%game_options.garbagecollectfrequency == 1 ) {
                SortStarSystems( star_system, _active_star_systems.back() );
                if (star_system.size() > game_options.numoldsystems && game_options.deleteoldsystems) {
                    if ( std::find( _active_star_systems.begin(), _active_star_systems.end(),
                                   star_system.back() ) == _active_star_systems.end() ) {
                        delete star_system.back();
                        star_system.pop_back();
                    } else {
                        BOOST_LOG_TRIVIAL(error) << "error with active star system list\n";
                    }
                }
            }
        }
    }
}

void Universe::StartGFX()
{
    GFXBeginScene();
    GFXMaterial mat;
    setMaterialAmbient( mat, 1.0, 1.0, 1.0, 1.0 );
    setMaterialDiffuse( mat, 1.0, 1.0, 1.0, 1.0 );
    setMaterialSpecular( mat, 1.0, 1.0, 1.0, 1.0 );
    setMaterialEmissive( mat, 0.0, 0.0, 0.0, 1.0 );
    mat.power = 60.0F;
    unsigned int tmp;
    GFXSetMaterial( tmp, mat );
    GFXSelectMaterial( tmp );
    int ligh;
    GFXCreateLightContext( ligh );
    GFXSetLightContext( ligh );
    GFXLightContextAmbient( GFXColor( 0, 0, 0, 1 ) );
    GFXEndScene();
}

// Missing startGL!

void Universe::Update()
{
    for (unsigned int i = 0; i < star_system.size(); ++i)
        //Calls the update function for server
        star_system[i]->Update( (i == 0) ? 1 : game_options.InactiveSystemTime/i );
}


// Camera
Camera * Universe::AccessCamera( int num )
{
    return AccessCockpit()->AccessCamera( num );
}

Camera * Universe::AccessCamera()
{
    return AccessCockpit()->AccessCamera();
}

Camera * Universe::AccessHudCamera()
{
    return &hud_camera;
}

void Universe::SelectCamera( int cam )
{
    AccessCockpit()->SelectCamera( cam );
}

// Cockpit
// TODO: candidate for deletion
void Universe::SetupCockpits(std::vector< std::string >players)
{
    for (unsigned int i = 0; i < players.size(); ++i) {
        _cockpits.push_back( NULL );
        _cockpits.back() = new GameCockpit( "", NULL, players[i] );
    }
}



Cockpit * Universe::AccessCockpit()
{
    return _cockpits[_current_cockpit];
}

Cockpit * Universe::AccessCockpit( int i )
{
    return _cockpits[i];
}

unsigned int Universe::CurrentCockpit()
{
    return _current_cockpit;
}

void Universe::SetActiveCockpit( int i )
{
#ifdef VS_DEBUG
    if ( i < 0 || i >= cockpit.size() ) {
        BOOST_LOG_TRIVIAL(error) << boost::format("ouch invalid cockpit %1$d") % i;
    }
#endif
    _current_cockpit = i;
}

void Universe::SetActiveCockpit( Cockpit *cp )
{
    for (unsigned int i = 0; i < _cockpits.size(); i++)
        if (_cockpits[i] == cp) {
            SetActiveCockpit( i );
            return;
        }
}

Cockpit* Universe::createCockpit( std::string player )
{
    Cockpit *cp = new Cockpit( "", NULL, player );
    _cockpits.push_back( cp );
    return cp;
}


// Galaxy
GalaxyXML::Galaxy * Universe::getGalaxy()
{
    return galaxy.get();
}


// Light Map
void Universe::activateLightMap( int stage )
{
    getActiveStarSystem( 0 )->activateLightMap( stage );
}

Texture* Universe::getLightMap()
{
    return getActiveStarSystem( 0 )->getLightMap();
}


// Player Ship
Cockpit* Universe::isPlayerStarship( const Unit *doNotDereference )
{
    using std::vector;
    if (!doNotDereference)
        return NULL;
    for (std::vector< Cockpit* >::iterator iter = _cockpits.begin(); iter < _cockpits.end(); iter++)
        if ( doNotDereference == ( *(iter) )->GetParent() )
            return *(iter);
    return NULL;
}

int Universe::whichPlayerStarship( const Unit *doNotDereference )
{
    if (!doNotDereference)
        return -1;
    for (unsigned int i = 0; i < _cockpits.size(); i++)
        if ( doNotDereference == _cockpits[i]->GetParent() )
            return i;
    return -1;
}


// Script System
StarSystem * Universe::scriptStarSystem()
{
    if (_script_system != NULL) return _script_system;

    else return activeStarSystem();
    ;
}
bool Universe::setScriptSystem( string name )
{
    if (name == "-active-") {
        _script_system = NULL;
        return true;
    }
    StarSystem *ss = getStarSystem( name );
    if (_script_system != NULL) {
        _script_system = ss;
        return true;
    }
    return false;
}


// Star System
StarSystem * Universe::activeStarSystem()
{
    return _active_star_systems.empty() ? NULL
           : _active_star_systems.back();
}

// Missing bool StillExists( StarSystem *ss );
void Universe::setActiveStarSystem( StarSystem *ss )
{
    if ( _active_star_systems.empty() )
        pushActiveStarSystem( ss );
    else
        _active_star_systems.back() = ss;
}
void Universe::pushActiveStarSystem( StarSystem *ss )
{
    _active_star_systems.push_back( ss );
}
void Universe::popActiveStarSystem()
{
    if ( !_active_star_systems.empty() )
        _active_star_systems.pop_back();
}

StarSystem* Universe::GenerateStarSystem( const char *file, const char *jumpback, Vector center )
{
    StarSystem *tmpcache;
    if ( ( tmpcache = GetLoadedStarSystem( file ) ) )
        return tmpcache;
    this->Generate1( file, jumpback );
    StarSystem *ss = new StarSystem( file, center );
    this->Generate2( ss );
    return ss;
}

void Universe::LoadStarSystem( StarSystem *s )
{
    BOOST_LOG_TRIVIAL(info) << "Loading a starsystem";
    star_system.push_back( s );
    SortStarSystems( star_system, s );     //dont' want instadie
}

bool Universe::StillExists( StarSystem *s )
{
    return std::find( star_system.begin(), star_system.end(), s ) != star_system.end();
}

void Universe::UnloadStarSystem( StarSystem *s )
{
    //not sure what to do here? serialize?
}

void Universe::Generate1( const char *file, const char *jumpback )
{
    int count = 0;
    if (game_options.while_loading_starsystem)
        ss_generating( true );
    VSFile  f;
    VSError err = f.OpenReadOnly( file, SystemFile );
    //If the file is not found we generate a system
    if (err > Ok)
        MakeStarSystem( file, galaxy.get(), RemoveDotSystem( jumpback ), count );
}

void Universe::Generate2( StarSystem *ss )
{
    static bool firsttime = true;
    LoadStarSystem( ss );
    pushActiveStarSystem( ss );
    for (unsigned int tume = 0; tume <= game_options.num_times_to_simulate_new_star_system*SIM_QUEUE_SIZE+1; ++tume)
        ss->UpdateUnitPhysics( true );
    //notify the director that a new system is loaded (gotta have at least one active star system)
    StarSystem *old_script_system = _script_system;
    _script_system = ss;
    BOOST_LOG_TRIVIAL(info) << boost::format("Loading Star System %1$s\n") % ss->getFileName().c_str();
    const vector< std::string > &adjacent = getAdjacentStarSystems( ss->getFileName() );
    for (unsigned int i = 0; i < adjacent.size(); i++) {
        BOOST_LOG_TRIVIAL(info) << boost::format(" Next To: %1$s\n") % adjacent[i].c_str();
    }
    static bool first = true;
    if (!first)
        mission->DirectorStartStarSystem( ss );
    first = false;
    _script_system = old_script_system;
    popActiveStarSystem();
    if ( _active_star_systems.empty() ) {
        pushActiveStarSystem( ss );
    } else {
        ss->SwapOut();
        activeStarSystem()->SwapIn();
    }
    if (firsttime) {
        firsttime = false;
    } else {}
    ss_generating( false );
}

void Universe::clearAllSystems()
{
    while ( star_system.size() ) {
        star_system.back()->RemoveStarsystemFromUniverse();
        delete star_system.back();
        star_system.pop_back();
    }
    _active_star_systems.clear();
    _script_system = NULL;
}

StarSystem * Universe::getActiveStarSystem( unsigned int size )
{
    return size >= _active_star_systems.size() ? NULL : _active_star_systems[size];
}
unsigned int Universe::getNumActiveStarSystem()
{
    return _active_star_systems.size();
}

StarSystem* Universe::getStarSystem( string name )
{
    vector< StarSystem* >::iterator iter;
    for (iter = star_system.begin(); iter != star_system.end(); iter++) {
        StarSystem *ss = *iter;
        if (ss->getName() == name)
            return ss;
    }
    return NULL;
}

int Universe::StarSystemIndex( StarSystem *ss )
{
    for (unsigned int i = 0; i < star_system.size(); i++)
        if (star_system[i] == ss)
            return i;
    return -1;
}


// Misc. Methods
void Universe::LoadFactionXML( const char *factfile )
{
    Faction::LoadXML( factfile );
}

UnitCollection& Universe::getActiveStarSystemUnitList() {
    return activeStarSystem()->getUnitList();
}

unsigned int Universe::numPlayers()
{
    return _cockpits.size();
}


/////////////////////////////////////////////////////////
// Unsorted























