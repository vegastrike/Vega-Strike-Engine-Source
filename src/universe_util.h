#ifndef _UNIVERSE_UTIL_H__
#define _UNIVERSE_UTIL_H__
#include "universe.h"
#include "cmd/collection.h"
#include "star_system.h"
#include <string>
#include "cmd/music.h"
#include "audiolib.h"
extern class Music *muzak;
extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name );

using std::string;

#define activeSys _Universe->activeStarSystem() //less to write

namespace UniverseUtil {
	void pushSystem (string name) {
		StarSystem * ss = _Universe->GenerateStarSystem (name.c_str(),"",Vector(0,0,0));
		_Universe->pushActiveStarSystem(ss);
	}
	void popSystem () {
		_Universe->popActiveStarSystem();
	}
	string getFileName() {
		return activeSys->getFileName();
	}
	string getName() {
		return activeSys->getName();
	}
	un_iter getUnitList() {
		return activeSys->getUnitList().createIterator();
	}
	Unit *launchJumppoint(string name_string,
			string faction_string,
			string type_string,
			string unittype_string,
			string ai_string,
			int nr_of_ships,
			int nr_of_waves, 
			QVector pos, 
			string squadlogo, 
			string destinations){
		int clstype=UNITPTR;
		if (unittype_string=="planet") {
			clstype =PLANETPTR;			
		}else if (unittype_string=="asteroid") {
			clstype = ASTEROIDPTR;
		}else if (unittype_string=="nebula") {
			clstype = NEBULAPTR;
		}
		CreateFlightgroup cf;
		cf.fg = Flightgroup::newFlightgroup (name_string,type_string,faction_string,ai_string,nr_of_ships,nr_of_waves,squadlogo,"",mission);
		cf.unittype=CreateFlightgroup::UNIT;
		cf.terrain_nr=-1;
		cf.waves=nr_of_waves;
		cf.nr_ships=nr_of_ships;
		cf.fg->pos=pos;
		for(int i=0;i<3;i++){
			cf.rot[i]=0.0;
		}
		Unit *tmp= mission->call_unit_launch(&cf,clstype,destinations);
		mission->number_of_ships+=nr_of_ships;
		return tmp;
	}
	Unit* launch (string name_string,string type_string,string faction_string,string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string sqadlogo){
		return launchJumppoint(name_string,faction_string,type_string,type_string,ai_string,nr_of_ships,nr_of_waves,pos,sqadlogo,"");
	}
	Cargo getRandCargo(int quantity, string category) {
	  Cargo *ret=NULL;
	  Unit *mpl = &GetUnitMasterPartList();
	  unsigned int max=mpl->numCargo();
	  if (!category.empty()) {
	    vector <Cargo> cat;
	    mpl->GetCargoCat (category,cat);
	    if (!cat.empty()) {
	      unsigned int i;
	      ret = mpl->GetCargo(cat[rand()%cat.size()].content,i);
	    }
	  }else {
	    if (mpl->numCargo()) {
	      for (unsigned int i=0;i<500;i++) {
		ret = &mpl->GetCargo(rand()%max);  
		if (ret->content.find("mission")==string::npos) {
		  break;
		}
	      }
	    }		  
	  }
	  if (ret) {
	    return *ret;//uses copy
	  }else {
	    Cargo newret;
	    newret.quantity=0;
	    return newret;
	  }
	}
	string GetFactionName(int index) {
		return _Universe->GetFaction(index);
	}
	int GetFactionIndex(string name) {
		return _Universe->GetFaction(name.c_str());
	}
	float GetRelation(string myfaction,string theirfaction) {
		return _Universe->GetRelation(GetFactionIndex(myfaction),GetFactionIndex(theirfaction));
	}
	void AdjustRelation(string myfaction,string theirfaction, float factor, float rank) {
		_Universe->AdjustRelation(GetFactionIndex(myfaction),GetFactionIndex(theirfaction), factor, rank);
	}
	int GetNumFactions () {
		return _Universe->GetNumFactions();
	}
	//NOTEXPORTEDYET
	float GetGameTime () {
		return mission->gametime;
	}
	void SetTimeCompression () {
		setTimeCompression(1.0);
	}
	string GetAdjacentSystem (string str, int which) {
		return _Universe->getAdjacentStarSystems(str)[which];
	}
	string GetGalaxyProperty (string sys, string prop) {
		return _Universe->getGalaxyProperty(sys,prop);
	}
	int GetNumAdjacentSystems (string sysname) {
		return _Universe->getAdjacentStarSystems(sysname).size();
	}
	int musicAddList(string str) {
		return muzak->Addlist(str.c_str());
	}
	void musicPlaySong(string str) {
		muzak->GotoSong(str);
	}
	void musicPlayList(int which) {
		muzak->SkipRandSong(which);
	}
	float GetDifficulty () {
		return g_game.difficulty;
	}
	void SetDifficulty (float diff) {
		g_game.difficulty=diff;
	}
	void playSound(string soundName, QVector loc, Vector speed) {
		int sound = AUDCreateSoundWAV (soundName,false);
		AUDAdjustSound (sound,loc,speed);
		AUDStartPlaying (sound);
		AUDDeleteSound(sound);
	}
	void playAnimation(string aniName, QVector loc, float size) {
		AddAnimation(loc,size,true,aniName);
	}
	void terminateMission(){
		mission->terminateMission();
	}
	Unit *getPlayer(){
		return _Universe->AccessCockpit()->GetParent();;
	}
	Unit *getPlayerX(int which){
		int j=0;
		for (unsigned int i=0;i<_Universe->numPlayers();i++) {
			Unit * un;
			if ((un=_Universe->AccessCockpit(i)->GetParent())) {
				if (j==which) {
					return un;
				}
				j++;
			}
		}
	}
}

#undef activeSys

#endif
