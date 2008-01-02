#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_generic.h"
//#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h"	 //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "networking/netserver.h"
#include "star_system_generic.h"
#include <string>
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"
extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );
void SetStarSystemLoading (bool value) {}

using std::string;

								 //less to write
#define activeSys _Universe->activeStarSystem()
void ClientServerSetLightContext(int lc) {}

namespace UniverseUtil
{

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
		return _Universe->CurrentCockpit();
	}
	void musicLayerSkip(int layer) {}
	void musicLayerStop(int layer) {}
	void StopAllSounds(void) {}
	void loadGame(const string &savename) {
		int num=-1;
		sscanf(savename.c_str(),"%d",&num);
		if (num>=0 && num<_Universe->numPlayers()) {
			Unit *un=_Universe->AccessCockpit(num)->GetParent();
			if (un) {
				un->hull=0;
				un->Destroy();
			}
		}
	}
	void saveGame(const string &savename) {
		int num=-1;
		sscanf(savename.c_str(),"%d",&num);
		if (num>=0 && num<_Universe->numPlayers()) {
			if (SERVER) VSServer->saveAccount(num);
		}
	}

	void showSplashScreen(const string &filename) {}
	void showSplashMessage(const string &text) {}
	void showSplashProgress(float progress) {}
	void hideSplashScreen() {}
	bool isSplashScreenShowing() { return false; }
	void startMenuInterface(bool firstTime, string error) {
		// Critical game error... enough to bring you back to the game menu!
		printf("GAME ERROR: %s\n",error.c_str());
	}

	void sendCustom(int cp, string cmd, string args, string id) {
		if (cp<0 || cp>=_Universe->numPlayers()) {
			fprintf(stderr, "sendCustom %s with invalid player %d\n", cmd, cp);
			return;
		}
		VSServer->sendCustom(cp, cmd, args, id);
	}
}


#undef activeSys
