#include "cmd/script/mission.h"
#include "universe_util_generic.h"
#include "universe_generic.h"
#include "cmd/unit.h"
//#include "cmd/unit_interface.h"
#include "cmd/unit_factory_generic.h" //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system_generic.h"
#include <string>
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );

using std::string;

#define activeSys _Universe->activeStarSystem() //less to write

namespace UniverseUtil {
	int musicAddList(string str) {
		return 0;
	}
	void musicPlaySong(string str) {
	}
	void musicPlayList(int which) {
	}
        void musicLoopList (int numloops) {
        }
	void playSound(string soundName, QVector loc, Vector speed) {
	}
        void cacheAnimation(string aniName) {
        }
	void playAnimation(string aniName, QVector loc, float size) {
	}
	void playAnimationGrow(string aniName, QVector loc, float size, float growpercent) {
	}
        unsigned int getCurrentPlayer() {
		return 0;
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
		return NULL;
	}
}
#undef activeSys
