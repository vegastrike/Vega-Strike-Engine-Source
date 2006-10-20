#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_generic.h"
//#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h" //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system_generic.h"
#include <string>
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );
void SetStarSystemLoading (bool value) {}

using std::string;

#define activeSys _Universe->activeStarSystem() //less to write
void ClientServerSetLightContext(int lc) {}

namespace UniverseUtil {
	void playVictoryTune(){}
	int musicAddList(string str) {
		return 0;
	}
	void musicLayerPlaySong(string str,int layer) {
	}
       	void addParticle (QVector loc, Vector velocity, Vector color, float size)
	{}
	void musicLayerPlayList(int which, int layer) {
	}
        void musicLayerLoopList (int numloops, int layer) {
        }
    void musicLayerSetSoftVolume(float vol, float latency_override, int layer) {

	}
	void musicLayerSetHardVolume(float vol, int layer) {

	}
    void musicSetSoftVolume(float vol, float latency_override) {

	}
	void musicSetHardVolume(float vol) {

	}

        void musicMute(bool stopSound){}
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
	void musicLayerSkip(int layer) {}
	void musicLayerStop(int layer) {}
	void StopAllSounds(void) {
        }

}
#undef activeSys
