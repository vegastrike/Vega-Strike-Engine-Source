#ifndef _UNIVERSE_UTIL_H__
#define _UNIVERSE_UTIL_H__
#include "cmd/collection.h"
#include <string>

using std::string;

class Unit;

namespace UniverseUtil {
	void pushSystem (string name);
	void popSystem ();
	string getSystemFile();
	string getSystemName();
	un_iter getUnitList();
	Unit *getUnit(int index);
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
	Unit* launch (string name_string,string type_string,string faction_string,string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string sqadlogo);
	Cargo getRandCargo(int quantity, string category);
	string GetFactionName(int index);
	int GetFactionIndex(string name);
	float GetRelation(string myfaction,string theirfaction);
	void AdjustRelation(string myfaction,string theirfaction, float factor, float rank);
	int GetNumFactions ();
	float GetGameTime ();
	void SetTimeCompression ();
	string GetAdjacentSystem (string str, int which);
	string GetGalaxyProperty (string sys, string prop);
	int GetNumAdjacentSystems (string sysname);
	int musicAddList(string str);
	void musicPlaySong(string str);
	void musicPlayList(int which);
	float GetDifficulty ();
	void SetDifficulty (float diff);
	void playSound(string soundName, QVector loc, Vector speed);
	void playAnimation(string aniName, QVector loc, float size);
	void terminateMission();
	Unit *getPlayer();
	Unit *getPlayerX(int which);
	int getNumPlayers ();
	int addObjective(string objective);
	void setObjective(int which, string newobjective);
	void setCompleteness(int which, float completeNess);
	float getCompleteness(int which);
	void setOwner(int which,Unit *owner);
	Unit* getOwner(int which);
    void IOmessage(int delay,string from,string to,string message);
}

#undef activeSys

#endif
