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
#include "gfx/particle.h"




extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );

using std::string;

#define activeSys _Universe->activeStarSystem() //less to write
  void ClientServerSetLightContext (int lightcontext) {
    GFXSetLightContext(lightcontext);
  }

namespace UniverseUtil {

	void playVictoryTune () {
	  static string newssong=vs_config->getVariable("audio","missionvictorysong","../music/victory.ogg");
	  muzak->GotoSong(newssong);
	}
	int musicAddList(string str) {
		return muzak->Addlist(str.c_str());
	}
	void musicSkip() {
		 muzak->Skip();
	}
	void musicPlaySong(string str) {
		muzak->GotoSong(str);
	}
	void musicPlayList(int which) {
		if (which!=-1)
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
	void StopAllSounds(void) {
          AUDStopAllSounds();
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
	int maxMissions () {
	static const int max_missions = XMLSupport::parse_int (vs_config->getVariable ("physics","max_missions","4"));
		return max_missions;
	}
       	void addParticle (QVector loc, Vector velocity, Vector color, float size)
	{
	  ParticlePoint p;
	  p.loc = loc;
	  p.col = color;
	  particleTrail.AddParticle (p,velocity,size);
	}

}
#undef activeSys
