

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
#include "unit_generic.h"
#include "vsfilesystem.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "music.h"
#include "base.h"
#include "networking/inet.h"
#include "python/python_compile.h"

Music::Music (Unit *parent):random(false), p(parent),song(-1) {
  loopsleft=0;
  if (!g_game.music_enabled)
	  return;
  lastlist=PEACELIST;
  if (parent) {
    maxhull = parent->GetHull();
  }else {
    maxhull=1;
  }
  int i;
  const char *listvars [MAXLIST]={"battleplaylist","peaceplaylist","panicplaylist","victoryplaylist","lossplaylist"};
  const char *deflistvars [MAXLIST]={"battle.m3u","peace.m3u","panic.m3u","victory.m3u","loss.m3u"};
  for (i=0;i<MAXLIST;i++) {
    LoadMusic(vs_config->getVariable ("audio",listvars[i],deflistvars[i]).c_str());
  }
  socket=-1;
  INET_startup();
  for (i=0;(i<10)&&(socket==-1);i++)
	  socket=INET_ConnectTo("localhost",4364);
  if (socket==-1) {
	  g_game.music_enabled=false;
  } else {
    string data=string("i")+vs_config->getVariable("audio","music_fadein","0")+"\n"
		"o"+vs_config->getVariable("audio","music_fadeout","0")+"\n";
    INET_Write(socket,data.size(),data.c_str());
    this->vol=XMLSupport::parse_float(vs_config->getVariable("audio","music_volume",".5"));
	ChangeVolume();
  }

}

void Music::ChangeVolume (float inc) {
	this->vol+=inc;
	if (this->vol>1) {
		this->vol=1;
	} else if (this->vol<0) {
		this->vol=0;
	}
	char tempbuf [100];
	sprintf(tempbuf,"v%f\n",this->vol);
    INET_Write(socket,strlen(tempbuf),tempbuf);
}

bool Music::LoadMusic (const char *file) {
	using namespace VSFileSystem;
	// Loads a playlist so try to open a file in datadir or homedir
  VSFile f;
  VSError err = f.OpenReadOnly(file, UnknownFile);
  if (err>Ok)
     err = f.OpenReadOnly( VSFileSystem::HOMESUBDIR +"/"+file, UnknownFile);

  char songname[1024];
  this->playlist.push_back(std::vector <std::string> ());
  if (err<=Ok) {
    while (!f.Eof()) {
      songname[0]='\0';
      f.ReadLine(songname,1022);
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
      this->playlist.back().push_back (std::string(songname));
    }
    f.Close();
  }else {
    return false;
  }
  return true;
}

inline int randInt (int max) {
	int ans= int((((double)rand())/((double)RAND_MAX))*max);
	if (ans==max) {
	  return max-1;
	}
	return ans;
}

int Music::SelectTracks(void) {
  if ((BaseInterface::CurrentBase||loopsleft>0)&&lastlist < (int)playlist.size()&&lastlist>=0) {
    if (loopsleft>0) {
      loopsleft--;
    }
    if (!playlist[lastlist].empty()) {
      int whichsong=rand()%playlist[lastlist].size();
      GotoSong (lastlist,whichsong,true);
      return whichsong;
    }
  }
  CompileRunPython (vs_config->getVariable("sound","dj_script","modules/dj.py"));
  
  return 0;
  int whichlist=0;
  static float hostile_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","hostile_auto_radius","8000"));
  Unit * un=_Universe->AccessCockpit()->GetParent();
  if (un==NULL) {
    whichlist=PEACELIST;
    if (playlist[PEACELIST].empty())
      return NOLIST;
    return randInt((playlist[PEACELIST].size()));
  }
  


  bool perfect=true;
  un_iter iter = _Universe->activeStarSystem()->getUnitList().createIterator();
  Unit *target;
  while ((target = iter.current())!=NULL) {
    float ftmp;
    ftmp = 2*FactionUtil::GetIntRelation (un->faction,target->faction);
    if (ftmp<0&&((un->Position()-target->Position()).Magnitude())<hostile_autodist)
      perfect=false;
    iter.advance();
  }
  if (perfect||playlist[BATTLELIST].empty()) {
    whichlist=PEACELIST;
    if (playlist[PEACELIST].empty())
      return NOLIST;
    return randInt((playlist[PEACELIST].size()));
  }
  float ftmp =(un->FShieldData()+2*un->GetHull()/maxhull+un->RShieldData()-2.7)*10;
  if (ftmp<-.5) {
    whichlist=PANICLIST;
    if (!playlist[PANICLIST].empty())
	    return randInt((playlist[PANICLIST].size()));
  }
  whichlist=BATTLELIST;
  int tmp=NOLIST;
  if (!playlist[BATTLELIST].empty()) {
    tmp=randInt((playlist[BATTLELIST].size()));
  }
  return (int)tmp;
}

void Music::Listen() {
	if (g_game.music_enabled) {
		int bytes=INET_BytesToRead(socket);
		if (bytes) {
			char data;
			while (bytes) {
				data=INET_fgetc(socket);
				bytes--;
			}
			if (data=='e') {
				Skip();
			} else {
				g_game.music_enabled=false;
			}
		}
	}
}

void Music::GotoSong (std::string mus) {
	if (g_game.music_enabled) {
		string data=string("p")+mus+string("\n");
		INET_Write(socket,data.size(),data.c_str());
	}
}

void Music::GotoSong (int whichlist,int whichsong,bool skip) {
	if (g_game.music_enabled) {
		if (whichsong!=NOLIST&&whichlist!=NOLIST&&whichlist<(int)playlist.size()&&whichsong<(int)playlist[whichlist].size()) {
			GotoSong(playlist[whichlist][whichsong]);
		} else {
			SkipRandList();
		}
	}
}

void Music::SkipRandSong(int whichlist) {
	if (this!=NULL) {
	  if (whichlist!=NOLIST&&whichlist>=0&&whichlist<(int)playlist.size()){
	    lastlist = whichlist;
	    GotoSong(whichlist,randInt(playlist[whichlist].size()),true);
	    return;
	  }
	}
	SkipRandList();
}

void Music::SkipRandList() {
	for (unsigned int i=0;i<playlist.size();i++) {
		if (!playlist[i].empty())
			GotoSong(i,randInt(playlist[i].size()),false);
	}
}

int Music::Addlist (std::string listfile) {
	bool retval=LoadMusic(listfile.c_str());
	if (retval) {
	  return playlist.size()-1;
	}else {
	  return -1;
	}
}

void Music::Skip() {
	if (g_game.music_enabled) {
	  SelectTracks();
	}
}
Music::~Music() {
	char send='t';
	INET_Write(socket,1,&send);
	send='\n';
	INET_Write(socket,1,&send);
	INET_close(socket);
	INET_cleanup();
}
void incmusicvol (const std::string&, KBSTATE a) {
	if (a==PRESS) {
		muzak->ChangeVolume (.0625);
	}
}
void decmusicvol (const std::string&, KBSTATE a) {
	if (a==PRESS) {
		muzak->ChangeVolume (-.0625);
	}
}
