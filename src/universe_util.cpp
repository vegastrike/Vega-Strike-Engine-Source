#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe.h"
#include "cmd/unit.h"
#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h" //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system.h"
#include <string>
#include "cmd/music.h"
#include "audiolib.h"
#include "gfx/animation.h"
#include "lin_time.h"
#include "load_mission.h"
#include "config_xml.h"
#include "vs_globals.h"
extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );

using std::string;

#define activeSys _Universe.activeStarSystem() //less to write

namespace GameUniverseUtil {
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
		Cargo tempret = *ret;
		tempret.quantity=quantity;
	    return tempret;//uses copy
	  }else {
	    Cargo newret;
	    newret.quantity=0;
	    return newret;
	  }
	}
	//NOTEXPORTEDYET
	float GetGameTime () {
		return mission->gametime;
	}
	void SetTimeCompression () {
		setTimeCompression(1.0);
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
        void musicLoopList (int numloops) {
                muzak->loopsleft=numloops;
        }
	void playSound(string soundName, QVector loc, Vector speed) {
		int sound = AUDCreateSoundWAV (soundName,false);
		AUDAdjustSound (sound,loc,speed);
		AUDStartPlaying (sound);
		AUDDeleteSound(sound);
	}
        void cacheAnimation(string aniName) {
	  static vector <Animation *> anis;
	  anis.push_back (new Animation(aniName.c_str()));
        }
	void playAnimation(string aniName, QVector loc, float size) {
		AddAnimation(loc,size,true,aniName,1);
	}
	void playAnimationGrow(string aniName, QVector loc, float size, float growpercent) {
		AddAnimation(loc,size,true,aniName,growpercent);
	}
        unsigned int getCurrentPlayer() {
	  return _Universe.CurrentCockpit();
        }
	Unit *getPlayer(){
		return _Universe.AccessCockpit()->GetParent();;
	}
	int getNumPlayers () {
		return _Universe.numPlayers();
	}
	Unit *getPlayerX(int which){
		if (which>=getNumPlayers()) {
			return NULL;
		}
		return _Universe.AccessCockpit(which)->GetParent();
	}
	Unit *GetMasterPartList () {
		return UnitFactory::getMasterPartList();
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
		cf.fg = GameFlightgroup::newGameFlightgroup (name_string,type_string,faction_string,ai_string,nr_of_ships,nr_of_waves,squadlogo,"",mission);
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
		return launchJumppoint(name_string,faction_string,type_string,unittype,ai_string,nr_of_ships,nr_of_waves,pos,sqadlogo,"");
	}
}
#undef activeSys
