/**
* universe_util.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

/// Headers for python modules
///
#ifndef _UNIVERSE_UTILGENERIC_H__
#define _UNIVERSE_UTILGENERIC_H__
#include "cmd/collection.h"
#include "gfx/vec.h"
#include "cmd/unit_util.h"

#include <string>
#include <vector>

class Unit;
class StarSystem;
class Cargo;

namespace UniverseUtil
{
class PythonUnitIter : public un_iter
{
public: PythonUnitIter() : UnitIterator() {}
//PythonUnitIterator (UnitListNode * start):UnitIterator(start){}
    PythonUnitIter( const UnitCollection::UnitIterator &t ) : UnitIterator( t ) {}
    ~PythonUnitIter() {}
    inline Unit * current()
    {
        return(**this);
    }
    inline void advance()
    {
        UnitIterator::advance();
    }
    void advanceSignificant();
    void advanceInsignificant();
    void advancePlanet();
    void advanceJumppoint();
    void advanceN( int );
    void advanceNSignificant( int n );
    void advanceNInsignificant( int n );
    void advanceNPlanet( int n );
    void advanceNJumppoint( int n );
    inline void preinsert( Unit *unit )
    {
        UnitIterator::preinsert( unit );
    }
    inline bool isDone()
    {
        return UnitIterator::isDone();
    }
    inline void remove()
    {
        UnitIterator::remove();
    }
    inline class Unit * next()
    {
        advance();
        return current();
    }
};

std::string LookupUnitStat( const std::string &unitname, const std::string &faction, const std::string &statname );


///this gets a unit with 1 of each cargo type in it
Unit * GetMasterPartList();

///this function sets the "current" system to be "name"  where name may be something like "Sol/Sol" or "Crucible/Cephid_17"  this function may take some time if the system has not been loaded before

void pushSystem( std::string name );
bool systemInMemory( std::string name );

///this function restores the active system.... there must be an equal number of pushSystems ans popSystems or else Vega Strike may behave unpredictably
void popSystem();

///This function gets the current system's official name
std::string getSystemFile();

///this function gets the current system's nickname (not useful)
std::string getSystemName();

///this function gets an iterator into the units in the current system... do NOT keep an iterator across a frame--it may get deleted!
PythonUnitIter getUnitList();


///This function gets a unit given a number (how many iterations to go down in the iterator)
Unit * getUnit( int index );

///This function gets a unit given a name
Unit * getUnitByName( std::string name );

///This function gets a unit given an unreferenceable pointer to it - much faster if finder is provided
Unit * getUnitByPtr( void *ptr, Unit *finder = 0, bool allowslowness = true );
Unit * getScratchUnit();
void setScratchUnit( Unit* );

void precacheUnit( std::string name, std::string faction );
QVector getScratchVector();
void setScratchVector( QVector );
int getNumUnits();
void cacheAnimation( std::string anim );

///this function launches a wormhole or a jump point.
Unit * launchJumppoint( std::string name_string,
                        std::string faction_string,
                        std::string type_string,
                        std::string unittype_string,
                        std::string ai_string,
                        int nr_of_ships,
                        int nr_of_waves,
                        QVector pos,
                        std::string squadlogo,
                        std::string destinations );
std::string vsConfig( std::string category, std::string option, std::string def );

///this function launches a normal fighter  the name is the flightgroup name, the type is the ship type, the faction is who it belongs to, the unittype is usually "unit" unless you want to make asteroids or nebulae or jump points or planets.  the aistring is either a python filename or "default"  the nr of ships is the number of ships to be launched with this group, the number of waves is num reinforcements... the position is a tuple (x,y,z) where they appear and the squadlogo is a squadron image...you can leave this the empty string '' for the default squadron logo.
Unit * launch( std::string name_string,
               std::string type_string,
               std::string faction_string,
               std::string unittype,
               std::string ai_string,
               int nr_of_ships,
               int nr_of_waves,
               QVector pos,
               std::string sqadlogo );

///this gets a random cargo type (useful for cargo missions) from either any category if category is '' or else from a specific category  'Contraband'  comes to mind!
Cargo getRandCargo( int quantity, std::string category );

///this gets a string which has in it a space delimited list of neighmoring systems
std::string GetAdjacentSystem( std::string str, int which );

///this gets a specific property of this system as found in universe/milky_way.xml
std::string GetGalaxyProperty( std::string sys, std::string prop );

///get the shortest path between systems as found in universe/milky_way.xml
std::vector< std::string > GetJumpPath( std::string from, std::string to );

///this gets a specific property of this system as found in universe/milky_way.xml and returns a default value if not found
std::string GetGalaxyPropertyDefault( std::string sys, std::string prop, std::string def );

std::string GetGalaxyFaction( std::string sys );
void SetGalaxyFaction( std::string sys, std::string fac );

///this gets the number of systems adjacent to the sysname
int GetNumAdjacentSystems( std::string sysname );

///this gets the current game time since last start in seconds
float GetGameTime();

///this gets the current absolute startime in seconds
float getStarTime();

///this gets the current stardate string
string getStarDate();

///this sets the time compresison value to zero
void SetTimeCompression();

///this adds a playlist to the music and may be triggered with an int
int musicAddList( std::string str );

///sets the software volume, with smooth transitions (latency_override==-1 uses default transition time)
void musicSetSoftVolume( float vol, float latency_override );

///sets the hardware volume, does not support transitions of any kind.
void musicSetHardVolume( float vol );

///this plays a specific song, at a specific layer
void musicLayerPlaySong( std::string str, int layer );

///this plays msuci from a given list, at a specific layer (where the int is what was returned by musicAddList)
void musicLayerPlayList( int which, int layer );

///this plays msuci from a given list, at a specific layer (where the int is what was returned by musicAddList)
void musicLayerLoopList( int numloops, int layer );

///this skips the current music track, at a specific layer (and goes to the next in the currently playing list)
void musicLayerSkip( int layer );

///this stops the music currently playing at a specific layer - with a nice fadeout
void musicLayerStop( int layer );

///sets the software volume, with smooth transitions (latency_override==-1 uses default transition time)
void musicLayerSetSoftVolume( float vol, float latency_override, int layer );

///sets the hardware volume, does not support transitions of any kind.
void musicLayerSetHardVolume( float vol, int layer );

///this mutes sound - or unmutes it
void musicMute( bool stopSound );

///this plays a specific song, through the crossfader construct
inline void musicPlaySong( std::string str )
{
    musicLayerPlaySong( str, -1 );
}

///this plays msuci from a given list (where the int is what was returned by musicAddList)
inline void musicPlayList( int which )
{
    musicLayerPlayList( which, -1 );
}

///this plays msuci from a given list (where the int is what was returned by musicAddList)
inline void musicLoopList( int numloops )
{
    musicLayerLoopList( numloops, -1 );
}

///this skips the current music track (and goes to the next in the currently playing list)
inline void musicSkip()
{
    musicLayerSkip( -1 );
}

///this stops the music currently playing - with a nice fadeout
inline void musicStop()
{
    musicLayerStop( -1 );
}

///this gets the difficutly of the game... ranges between 0 and 1... many missions depend on it never going past .99 unless it's always at one.
float GetDifficulty();

///this sets the difficulty
void SetDifficulty( float diff );

///this plays a sound at a location...if the sound has dual channels it will play in the center
void playSound( std::string soundName, QVector loc, Vector speed );

///this plays a sound at full volume in the cockpit
void playSoundCockpit( std::string soundName );

///this plays an image (explosion or warp animation) at a location
void playAnimation( std::string aniName, QVector loc, float size );
void playAnimationGrow( std::string aniName, QVector loc, float size, float growpercent );

///tells the respective flightgroups in this system to start shooting at each other
void TargetEachOther( std::string fgname, std::string faction, std::string enfgname, std::string enfaction );

///tells the respective flightgroups in this system to stop killing each other urgently...they may still attack--just not warping and stuff
void StopTargettingEachOther( std::string fgname, std::string faction, std::string enfgname, std::string enfaction );

///this ends the mission with either success or failure
void terminateMission( bool term );

///this gets the player belonging to this mission
Unit * getPlayer();

///this gets a player number (if in splitscreen mode)
Unit * getPlayerX( int which );
unsigned int getCurrentPlayer();

///this gets the number of active players
int getNumPlayers();

//Clears all objectives (used for server-side when it's easy to get inconsistent.
void setTargetLabel( std::string label );
std::string getTargetLabel();

float getRelationModifierInt( int which_cp, int faction );
float getRelationModifier( int which_cp, string faction );
float getFGRelationModifier( int which_cp, string fg );
void adjustRelationModifierInt( int which_cp, int faction, float delta );
void adjustRelationModifier( int which_cp, string faction, float delta );
void adjustFGRelationModifier( int which_cp, string fg, float delta );

void AdjustRelation( std::string myfaction, std::string theirfaction, float factor, float rank );
float GetRelation( std::string myfaction, std::string theirfaction );

void clearObjectives();
//Erases an objective.
void eraseObjective( int which );

///this adds an objective for the cockpit to view ("go here and do this)
int addObjective( std::string objective );

///this sets the objective's completeness (the int was returned by add objective)
void setObjective( int which, std::string newobjective );

///this sets the completeness of a particular objective... chanigng the color onscreen
void setCompleteness( int which, float completeNess );

///this gets that completeness
float getCompleteness( int which );

///this sets the owner of a completeness
void setOwnerII( int which, Unit *owner );

///this gets an owner of a completeness (NULL means all players can see this objective)
Unit * getOwner( int which );
//gets the owner of this mission
int getMissionOwner();
//sets the owner of this mission to be a particular cockpit
void setMissionOwner( int );

///returns number missions running to tweak difficulty
int numActiveMissions();

///this sends an IO message... I'm not sure if delay currently works, but from, to and message do :-) ... if you want to send to the bar do "bar" as the to string... if you want to make news for the news room specify "news"
void IOmessage( int delay, std::string from, std::string to, std::string message );

///this gets a unit with 1 of each cargo type in it
Unit * GetMasterPartList();

///this gets a unit with a faction's contraband list... may be null (check with isNull)
Unit * GetContrabandList( std::string faction );

///this sets whether or not a player may autopilot.  Normally they are both 0 and the autopiloting is allowed based on if enemies are near... if you pass in 1 then autopilot will be allowed no matter who is near... if you set -1 then autopilot is never allowed.  global affects all players... player just affects the player who accepted the mission.
void SetAutoStatus( int global_auto, int player_auto );
void LoadMission( std::string missionname );
void LoadMissionScript( std::string scriptcontents );
void LoadNamedMissionScript( std::string missiontitle, std::string scriptcontents );
QVector SafeEntrancePoint( QVector, float radial_size = -1 );
QVector SafeStarSystemEntrancePoint( StarSystem*sts, QVector, float radial_size = -1 );
float getPlanetRadiusPercent();

void cacheAnimation( std::string anim );

///this function launches a wormhole or ajump point.
Unit * launchJumppoint( std::string name_string,
                        std::string faction_string,
                        std::string type_string,
                        std::string unittype_string,
                        std::string ai_string,
                        int nr_of_ships,
                        int nr_of_waves,
                        QVector pos,
                        std::string squadlogo,
                        std::string destinations );

///this function launches a normal fighter  the name is the flightgroup name, the type is the ship type, the faction is who it belongs to, the unittype is usually "unit" unless you want to make asteroids or nebulae or jump points or planets.  the aistring is either a python filename or "default"  the nr of ships is the number of ships to be launched with this group, the number of waves is num reinforcements... the position is a tuple (x,y,z) where they appear and the squadlogo is a squadron image...you can leave this the empty string '' for the default squadron logo.
Unit * launch( std::string name_string,
               std::string type_string,
               std::string faction_string,
               std::string unittype,
               std::string ai_string,
               int nr_of_ships,
               int nr_of_waves,
               QVector pos,
               std::string sqadlogo );

///this gets a random cargo type (useful for cargo missions) from either any category if category is '' or else from a specific category  'Contraband'  comes to mind!
Cargo getRandCargo( int quantity, std::string category );

///this gets the current game time since last start in seconds
//float GetGameTime();

///this gets the current absolute startime in seconds
//float getStarTime();

///this sets the time compresison value to zero
void SetTimeCompression();

///this adds a playlist to the music and may be triggered with an int
int musicAddList( std::string str );

///this plays a specific song
void musicPlaySong( std::string str );

///this plays msuci from a given list (where the int is what was returned by musicAddList)
void musicPlayList( int which );

///this plays msuci from a given list (where the int is what was returned by musicAddList)
void musicLoopList( int numloops );

///this plays a sound at a location...if the sound has dual channels it will play in the center
void playSound( std::string soundName, QVector loc, Vector speed );

///this plays an image (explosion or warp animation) at a location
void playAnimation( std::string aniName, QVector loc, float size );
void playAnimationGrow( std::string aniName, QVector loc, float size, float growpercent );

///this gets the player belonging to this mission
Unit * getPlayer();

///this gets a player number (if in splitscreen mode)
Unit * getPlayerX( int which );
void StopAllSounds( void );
unsigned int getCurrentPlayer();

///this gets the number of active players
int getNumPlayers();
unsigned int maxMissions();
bool networked();
bool isserver();
//Forwards this request onto the server if this is a client, or send back to client.
void sendCustom( int cp, string cmd, string args, string id );
//Executes a python script
void receivedCustom( int cp, bool trusted, string cmd, string args, string id );

std::string getVariable( std::string section, std::string name, std::string def );
std::string getSubVariable( std::string section, std::string subsection, std::string name, std::string def );
double timeofday();
double sqrt( double );
double log( double );
double exp( double );
double cos( double );
double sin( double );
double acos( double );
double asin( double );
double atan( double );
double tan( double );
void micro_sleep( int n );
void addParticle( QVector loc, Vector velocity, Vector color, float size );

std::string getSaveDir();
std::string getSaveInfo( const std::string &filename, bool formatForTextbox );
std::string getCurrentSaveGame();
std::string setCurrentSaveGame( const std::string &newsave );
std::string getNewGameSaveName();
void loadGame( const std::string &savename );
void saveGame( const std::string &savename );

//Splash stuff
void showSplashScreen( const std::string &filename );
void showSplashMessage( const std::string &text );
void showSplashProgress( float progress );
void hideSplashScreen();
bool isSplashScreenShowing();

//Defined in gamemenu.cpp for vegastrike, universe_util_server.cpp for vegaserver.
void startMenuInterface( bool firsttime, string alert = string() );         //If game fails, bring it back to a common starting point.
}

#undef activeSys
#endif

