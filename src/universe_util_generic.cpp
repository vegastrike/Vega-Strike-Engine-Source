#include <math.h>
#include "lin_time.h"
#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_generic.h"
//#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h" //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system_generic.h"
#include <string>
//#include "cmd/music.h"
//#include "audiolib.h"
//#include "gfx/animation.h"
#include "gfx/cockpit_generic.h"
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
//extern class Music *muzak;
//extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );
extern Unit&GetUnitMasterPartList();

using std::string;

#define activeSys _Universe->activeStarSystem() //less to write

namespace UniverseUtil {
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
	Unit *GetMasterPartList () {
		return UnitFactory::getMasterPartList();
	}
	void pushSystem (string name) {
		StarSystem * ss = _Universe->GenerateStarSystem (name.c_str(),"",Vector(0,0,0));
		_Universe->pushActiveStarSystem(ss);
	}
	void popSystem () {
		_Universe->popActiveStarSystem();
	}
	string getSystemFile() {
		return activeSys->getFileName();
	}
	string getSystemName() {
		return activeSys->getName();
	}
	void setMissionOwner(int whichplayer) {
		mission->player_num=whichplayer;
	}
	int getMissionOwner() {
		return mission->player_num;
	}
	un_iter getUnitList() {
		return activeSys->getUnitList().createIterator();
	}
	Unit *getUnit(int index) {
		un_iter iter=activeSys->getUnitList().createIterator();
		for(int i=0;iter.current()&&i<index;i++) {
			iter.advance();
		}
		return iter.current();
	}
        int getNumUnits() {
	  int count=0;
	  un_iter iter=activeSys->getUnitList().createIterator();
	  while (iter.current()){
	    iter.advance();
	    count++;
	  }
	  return count;
	}
	/*
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
	*/
	//NOTEXPORTEDYET
	/*
	float GetGameTime () {
		return mission->gametime;
	}
	void SetTimeCompression () {
		setTimeCompression(1.0);
	}
	*/
	string GetAdjacentSystem (string str, int which) {
		return _Universe->getAdjacentStarSystems(str)[which];
	}
	string GetGalaxyProperty (string sys, string prop) {
		return _Universe->getGalaxyProperty(sys,prop);
	}
	int GetNumAdjacentSystems (string sysname) {
		return _Universe->getAdjacentStarSystems(sysname).size();
	}
	/*
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
	*/
	float GetDifficulty () {
		return g_game.difficulty;
	}
	void SetDifficulty (float diff) {
		g_game.difficulty=diff;
	}
	/*
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
	  return _Universe->CurrentCockpit();
        }
	Unit *getPlayer(){
		return _Universe->AccessCockpit()->GetParent();;
	}
	int getNumPlayers () {
		return _Universe->numPlayers();
	}
	Unit *getPlayerX(int which){
		if (which>=getNumPlayers()) {
			return NULL;
		}
		return _Universe->AccessCockpit(which)->GetParent();
	}
	*/
	extern void playVictoryTune();
	void terminateMission(bool Win){
		if (Win)
			playVictoryTune();
		mission->terminateMission();
	}
	int addObjective(string objective) {
		mission->objectives.push_back(Mission::Objective(0,objective));
		return mission->objectives.size()-1;
	}
	void setObjective(int which, string newobjective) {
		if (which<(int)mission->objectives.size()) {
			mission->objectives[which].objective=newobjective;
		}
	}
	void setCompleteness(int which, float completeNess) {
		if (which<(int)mission->objectives.size()) {
			mission->objectives[which].completeness=completeNess;
		}
	}
	float getCompleteness(int which) {
		if (which<(int)mission->objectives.size()) {
			return mission->objectives[which].completeness;
		} else {
			return 0;
		}
	}
	void setOwnerII(int which,Unit *owner) {
		if (which<(int)mission->objectives.size()) {
			mission->objectives[which].owner=owner;
		}
	}
	Unit* getOwner(int which) {
		if (which<(int)mission->objectives.size()) {
			return mission->objectives[which].owner.GetUnit();
		} else {
			return 0;
		}
	}
    int numActiveMissions() {
        return active_missions.size();
    }
    void IOmessage(int delay,string from,string to,string message){
		mission->msgcenter->add(from,to,message,delay);
	}
	Unit *GetContrabandList (string faction) {
		return FactionUtil::GetContraband(FactionUtil::GetFaction(faction.c_str()));
	}
        void LoadMission (string missionname) {
                delayLoadMission (missionname);
        }
        void SetAutoStatus (int global_auto, int player_auto) {
	  if (global_auto==1) {
	    mission->global_autopilot = Mission::AUTO_ON;
	  }else if (global_auto==-1) {
	    mission->global_autopilot = Mission::AUTO_OFF;
	  }else {
	    mission->global_autopilot = Mission::AUTO_NORMAL;
	  }

	  if (player_auto==1) {
	    mission->player_autopilot = Mission::AUTO_ON;	    
	  }else if (player_auto==-1) {
	    mission->player_autopilot = Mission::AUTO_OFF;
	  }else {
	    mission->player_autopilot = Mission::AUTO_NORMAL;
	  }
        }
  QVector SafeEntrancePoint (QVector pos, float radial_size) {
    static double def_un_size = XMLSupport::parse_float (vs_config->getVariable ("physics","respawn_unit_size","400"));
    if (radial_size<0)
      radial_size = def_un_size;
    
    for (unsigned int k=0;k<10;k++) {
      Unit * un;
      bool collision=false;
      for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();(un=*i)!=NULL;++i) {
	if (un->isUnit()==ASTEROIDPTR||un->isUnit()==NEBULAPTR) {
	  continue;
	}
	double dist = (pos-un->LocalPosition()).Magnitude()-un->rSize()-def_un_size-radial_size;
	if (dist<0) {
	  QVector delta  = pos-un->LocalPosition();
	  double mag = delta.Magnitude();
	  if (mag>.01){
	    delta=delta/mag;
	  }else {
	    delta.Set(0,0,1);
	  }
	  delta = delta.Scale ( dist+def_un_size+radial_size);
	  if (k<5) {
	    pos = pos+delta;
	    collision=true;
	  }else {
	    QVector r(.5,.5,.5);
	    pos+=un->rSize()*r;
	    collision=true;
	  }
	  
	}
      }
      if (collision==false)
	break;
    }
    return pos;
  }
	Unit* launch (string name_string,string type_string,string faction_string,string unittype, string ai_string,int nr_of_ships,int nr_of_waves, QVector pos, string sqadlogo){
		return launchJumppoint(name_string,faction_string,type_string,unittype,ai_string,nr_of_ships,nr_of_waves,pos,sqadlogo,"");
	}
	Unit *getPlayer(){
		return _Universe->AccessCockpit()->GetParent();;
	}
	int getNumPlayers () {
		return _Universe->numPlayers();
	}
	Unit *getPlayerX(int which){
		if (which>=getNumPlayers()) {
			return NULL;
		}
		return _Universe->AccessCockpit(which)->GetParent();
	}
	float getPlanetRadiusPercent () {
		static float planet_rad_percent =  XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_radius_percent",".75"));
		return planet_rad_percent;
	}
	std::string getVariable(std::string section,std::string name,std::string def) {
		return vs_config->getVariable(section,name,def);
	}
	std::string getSubVariable(std::string section,std::string subsection,std::string name,std::string def) {
		return vs_config->getVariable(section,subsection,name,def);
	}
  double timeofday () {return getNewTime();}
  double sqrt (double x) {return ::sqrt (x);}
  double log (double x) {return ::log (x);}
  double exp (double x) {return ::exp (x);}
  double cos (double x) {return ::cos (x);}
  double sin (double x) {return ::sin (x);}
  double acos (double x) {return ::acos (x);}
  double asin (double x) {return ::asin (x);}
  double atan (double x) {return ::atan (x);}
  double tan (double x) {return ::tan (x);}
  void micro_sleep(int n) {
    ::micro_sleep(n);
  }

}

#undef activeSys
