

#include "vegastrike.h"
#include "vs_globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "audiolib.h"
#include "universe.h"
#include "star_system.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "lin_time.h"
#include "collection.h"
#include "unit.h"
#include "vs_path.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "music.h"
Music::Music (Unit *parent):random(false), p(parent),song(-1) {
  if (parent) {
    maxhull = parent->GetHull();
  }else {
    maxhull=1;
  }
  lastsonglength=0;
  curtime=0;
  changehome();
  LoadMusic(0,vs_config->getVariable ("audio","battleplaylist","battle.m3u").c_str());
  LoadMusic(1,vs_config->getVariable ("audio","peaceplaylist","peace.m3u").c_str());
  LoadMusic(2,vs_config->getVariable ("audio","panicplaylist","panic.m3u").c_str());
  returnfromhome();
}

void Music::LoadMusic (int which, const char *file) {
  FILE *fp = fopen (file,"r");
  char songname[1024];
  if (fp) {
    while (!feof (fp)) {
      songname[0]='\0';
      fgets (songname,1022,fp);
      int size = strlen(songname);
      if (size>=1) 
	if (songname[size-1]=='\n') {
	  songname[size-1]='\0';
	}
      if (size>1)
	if (songname[size-2]=='\r'||songname[size-2]=='\n') {
	  songname[size-2]='\0';
	}
      if (songname[0]=='\0'||songname[0]=='#')
	continue;
      this->playlist[which].push_back (std::string(songname));
    }
    fclose (fp);
  }
}
int Music::SelectTracks(int &whichlist) {
  Unit * un;
  if ((un =p.GetUnit())==NULL) {
    whichlist=1;
    if (playlist[1].empty())
      return 0;
    return rand()%playlist[1].size();
  }
  bool perfect=true;
  float goodness=0;
  un_iter iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit *target;
  while ((target = iter.current())!=NULL) {
    float ftmp;
    ftmp = 2*_Universe->GetRelation (un->faction,target->faction);
    if (ftmp<0)
      perfect=false;
    if (FINITE (ftmp))
      goodness += ftmp;
    iter.advance();
  }
  if (perfect||playlist[0].empty()) {
    whichlist=1;
    if (playlist[1].empty())
      return 0;
    return rand()%playlist[1].size();
  }
  float ftmp =(un->FShieldData()+2*un->GetHull()/maxhull+un->RShieldData()-2.7)*10;
  if (ftmp<-.5) {
    whichlist=2;
    if (playlist[2].empty())
      return 0;
    return (rand()%playlist[2].size());    
  }
  whichlist=0;
  if (FINITE (ftmp)) 
    goodness += ftmp;


  goodness -= playlist[0].size()/8;
  goodness += playlist[0].size()/2;
  //  fprintf (stderr,"Choosing Song %f",goodness);
  int tmp=0;
  if (!playlist[0].empty()) {
    tmp=(rand()%((playlist[0].size()+3)/4));
  }
  goodness+=tmp;
  //fprintf (stderr,"Choosing Song With Randomness %f\n",goodness);
  if (goodness<0)
    goodness=0;
  if (goodness>=playlist[0].size()) {
    goodness=playlist[0].size()-1;
  }
  return (int)goodness;
}

float HowLong (std::string &song) {
	unsigned long sze=0;
	FILE * fp = fopen (song.c_str(),"rb");
	if (fp) {
		fseek (fp,0,SEEK_END);
		sze = ftell (fp);
		fclose (fp);
	}
	return ((float)sze)/((128.*1024/8.));
}
void Music::Listen() {
	static bool use_external = XMLSupport::parse_bool(vs_config->getVariable("audio","use_external_player","true"));
	p.GetUnit();//so we look at it every now and then;
	
  if (g_game.music_enabled ) {
#ifdef _WIN32
	  if (use_external) {
		static int firstseconds=0;
		//lastsonglength=0;
		curtime+=SIMULATION_ATOM/getTimeCompression();
		if (curtime>lastsonglength) {
			curtime=0;
			int whichlist;
			int song = SelectTracks (whichlist);
			if (!playlist[whichlist].empty()) {
				lastsonglength= HowLong (playlist[whichlist][song]);
				if (lastsonglength>0.1) {
					static bool replace=true;
#ifdef _WIN32
//					static std::string plyr=vs_config->getVariable("audio","external_player","C:\\Program Files\\Winamp\\Winamp.exe");
//					static std::string play=vs_config->getVariable("audio","external_play_option","/PLAY");
//					static std::string enq=vs_config->getVariable("audio","external_enqueue_option","/ADD");
#if 0
					if (replace) {
						if (play.empty()) {
							spawnl(P_NOWAIT,plyr.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						} else {
							spawnl(P_NOWAIT,plyr.c_str(),play.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						}
					}else {
						if (enq.empty()) {
							spawnl(P_NOWAIT,plyr.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						} else {
							spawnl(P_NOWAIT,plyr.c_str(),enq.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						}
					}
#endif
					int worked=(int)ShellExecute(NULL,"open",(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),"","",2);
					if (worked>=0||worked<=32) {
						
					}
#else	
					static std::string plyr=vs_config->getVariable("audio","external_player","/usr/bin/xmms");
					static std::string play=vs_config->getVariable("audio","external_play_option","");
					static std::string enq=vs_config->getVariable("audio","external_enqueue_option","-e");
					if (replace) {
						if (play.empty()) {
							execlp(plyr.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						} else {
							execlp(plyr.c_str(),play.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						}
					}else {
						if (enq.empty()) {
							execlp(plyr.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						} else {
							execlp(plyr.c_str(),enq.c_str(),(string ("\"")+playlist[whichlist][song]+string ("\"")).c_str(),NULL);
						}
					}
#endif
					replace=false;
				} else {
					lastsonglength=0;
				}
//				execlp(plyr.c_str(),plyr.c_str(),"-e",(playlist[whichlist][song]).c_str(),NULL);
			} 
		}
	  }else 
#endif
	    {
	    if ((!AUDIsPlaying (song))) {
	     AUDDeleteSound (song,true);//delete buffer too;
	     int whichlist;
	     int songnum = SelectTracks(whichlist);
	     if (!playlist[whichlist].empty ()) {
	       song = AUDCreateMusic (playlist[whichlist][songnum],false);
	       AUDStartPlaying (song); 
	   	   AUDAdjustSound (song,_Universe->AccessCamera()->GetPosition(),_Universe->AccessCamera()->GetVelocity());

		 }    
		} else {
		 AUDAdjustSound (song,_Universe->AccessCamera()->GetPosition(),_Universe->AccessCamera()->GetVelocity());
		}
	  }
	}
}

void Music::Skip() {
  AUDStopPlaying (song);
  curtime= lastsonglength;
}
Music::~Music() {
  AUDDeleteSound (song,true);//delete buffer too;
}

