#ifndef _UNIVERSE_UTIL_H__
#define _UNIVERSE_UTIL_H__
#include "cmd/collection.h"
#include <string>

using std::string;

class Cargo;
class Unit;

namespace GameUniverseUtil {
	void cacheAnimation (string anim);
///this function launches a wormhole or ajump point.
	Unit *launchJumppoint(string name_string,
			string faction_string,
			string type_string,
			string unittype_string,
			string ai_string,
			int nr_of_ships,
			int nr_of_waves, 
			QVector pos, 
			string squadlogo, 
			string destinations);
///this function launches a normal fighter  the name is the flightgroup name, the type is the ship type, the faction is who it belongs to, the unittype is usually "unit" unless you want to make asteroids or nebulae or jump points or planets.  the aistring is either a python filename or "default"  the nr of ships is the number of ships to be launched with this group, the number of waves is num reinforcements... the position is a tuple (x,y,z) where they appear and the squadlogo is a squadron image...you can leave this the empty string '' for the default squadron logo. 
	Unit* launch (string name_string,string type_string,string faction_string,string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string sqadlogo);
///this gets a random cargo type (useful for cargo missions) from either any category if category is '' or else from a specific category  'Contraband'  comes to mind!
	Cargo getRandCargo(int quantity, string category);
///this gets the current time in seconds
	float GetGameTime ();
///this sets the time compresison value to zero
	void SetTimeCompression ();
///this adds a playlist to the music and may be triggered with an int
	int musicAddList(string str);
///this plays a specific song
	void musicPlaySong(string str);
///this plays msuci from a given list (where the int is what was returned by musicAddList)
	void musicPlayList(int which);
///this plays msuci from a given list (where the int is what was returned by musicAddList)
	void musicLoopList(int numloops);
///this plays a sound at a location...if the sound has dual channels it will play in the center
	void playSound(string soundName, QVector loc, Vector speed);
///this plays an image (explosion or warp animation) at a location
	void playAnimation(string aniName, QVector loc, float size);
	void playAnimationGrow(string aniName, QVector loc, float size,float growpercent);
///this gets the player belonging to this mission
	Unit *getPlayer();
///this gets a player number (if in splitscreen mode)
	Unit *getPlayerX(int which);
	unsigned int getCurrentPlayer();
///this gets the number of active players
	int getNumPlayers ();
///this gets a unit with 1 of each cargo type in it
	Unit *GetMasterPartList ();
}

#undef activeSys

#endif
