

#include "vegastrike.h"
#include "vs_globals.h"

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
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <string.h>
#include <stdio.h>
#include <direct.h>
#include <stdlib.h>
#endif
#include "music.h"
#include "base.h"
#include "networking/inet_file.h"
#include "networking/inet.h"
#include "python/python_compile.h"
bool soundServerPipes() {
    static bool ret=  XMLSupport::parse_bool(vs_config->getVariable("audio","pierce_firewall","false"));
    return ret;
}
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
  socketw=socketr=-1;
  if (soundServerPipes()) {
      fNET_startup();
      int pipesw[2];
      int pipesr[2];
      socketw=socketr=-1;//FIXME
#ifdef _WIN32
#define pipe _pipe
#endif
  if (0==  pipe(pipesw
#ifdef _WIN32
                ,32,O_BINARY
#endif
                )&&
	  0== pipe(pipesr
#ifdef _WIN32
                   ,32,O_BINARY
#endif
                   )) {
		  socketw=pipesw[1];
		  socketr=pipesr[0];
		  char buffer1[32];
		  char buffer2[32];
		sprintf (buffer1,"%d",pipesw[0]);
		sprintf (buffer2,"%d",pipesr[1]);

#if defined( _WIN32) && !defined( __CYGWIN__)
	  string ss_path = VSFileSystem::datadir+"/soundserver.exe";
      int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),buffer1,buffer2,NULL);
      if (pid==-1) {
		ss_path = VSFileSystem::datadir+"/bin/soundserver.exe";
		bool chsuccess= (chdir("bin")==0);
		int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),buffer1,buffer2,NULL);
		if (chsuccess) chdir("..");
		if (pid==-1) {
			g_game.music_enabled=false;
			VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server Error (%d)\n",pid);
		}
      }
#else
  if (g_game.music_enabled) {
    std::string tmp=VSFileSystem::datadir+"/bin/soundserver";
    FILE * fp=fopen (tmp.c_str(),"rb");
    if (!fp) {
      tmp=VSFileSystem::datadir+"/soundserver";
      fp = fopen(tmp.c_str(),"rb");
      if (!fp){
        g_game.music_enabled=false;
        socketw=-1;
        socketr=-1;
      }else fclose(fp);
    }else fclose(fp);
  }
  if (g_game.music_enabled) {
    int pid=fork();
    if (!pid) {
	  string soundserver_path = VSFileSystem::datadir+"/bin/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),buffer1,buffer2,NULL);
      soundserver_path = VSFileSystem::datadir+"/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),buffer1,buffer2,NULL);
      g_game.music_enabled=false;
      VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server\n");
      close(atoi(buffer1));
      close(atoi(buffer2));
      exit (0);
    } else {
      if (pid==-1) {
	g_game.music_enabled=false;
      }
    }
  }

#endif
  }
  }else {
      int socket=-1;
      INET_startup();
      for (i=0;(i<10)&&(socket==-1);i++) {
	  socket=INET_ConnectTo("localhost",4364);
      }
      socketw=socketr=socket;
  }
  if (socketw==-1||socketr==-1) {
	  g_game.music_enabled=false;
  } else {
    string data=string("i")+vs_config->getVariable("audio","music_fadein","0")+"\n"
		"o"+vs_config->getVariable("audio","music_fadeout","0")+"\n";
    if (soundServerPipes()) {
        fNET_Write(socketw,data.size(),data.c_str());
    }else {
        INET_Write(socketw,data.size(),data.c_str());
    }
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
        if (soundServerPipes()) {
            fNET_Write(socketw,strlen(tempbuf),tempbuf);
        }else {
            INET_Write(socketw,strlen(tempbuf),tempbuf);
        }
}

bool Music::LoadMusic (const char *file) {
	using namespace VSFileSystem;
	// Loads a playlist so try to open a file in datadir or homedir
  VSFile f;
  VSError err = f.OpenReadOnly(file, UnknownFile);
  if (err>Ok)
     err = f.OpenReadOnly( VSFileSystem::HOMESUBDIR +"/"+file, UnknownFile);

  char songname[1024];
  this->playlist.push_back(PlayList());
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
      this->playlist.back().songs.push_back (std::string(songname));
    }
    f.Close();
  }else {
    return false;
  }
  return true;
}

static int randInt (int max) {
  int ans= int((((double)rand())/((double)RAND_MAX))*max);
  if (ans==max) {
    return max-1;
  }
  return ans;
}

int Music::SelectTracks(void) {
  static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
  if ((BaseInterface::CurrentBase||loopsleft>0)&&lastlist < (int)playlist.size()&&lastlist>=0) {
    if (loopsleft>0) {
      loopsleft--;
    }
    if (!playlist[lastlist].empty()) {

     
      int whichsong=(random?rand():playlist[lastlist].counter++)%playlist[lastlist].size();
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
volatile int moredata=0;
#ifndef _WIN32
void * 
#else
DWORD WINAPI
#endif
readerThread (
#ifdef _WIN32
PVOID
#else
			  void * 
#endif		  
			  input) {
	int socketr = (int)input;
	while(1) {
          printf ("Reading from socket %d\n",socketr);
          char data=fNET_fgetc(socketr);
          printf ("Got data froms ocket %c\n",data);
          if (data=='e')
            moredata=1;
          micro_sleep(500000);
          
	}
	return NULL;
}

void Music::Listen() {
	if (g_game.music_enabled) {
            if (soundServerPipes()) {
#ifdef _WIN32
		static void * a_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readerThread, (PVOID)socketr, 0, NULL);
#else
                static pthread_t a_thread;
                static int res = pthread_create(&a_thread, NULL, readerThread, (void*)socketr);

#endif
		//int bytes=fNET_BytesToRead(socketr);
		if (moredata) {
			moredata=0;
			Skip();
		}
            }else {
		int bytes=INET_BytesToRead(socketr);
		if (bytes) {
			char data;
			while (bytes) {
				data=INET_fgetc(socketr);
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
}

void Music::GotoSong (std::string mus) {
	if (g_game.music_enabled) {
		string data=string("p")+mus+string("\n");
                if (soundServerPipes()){
                    fNET_Write(socketw,data.size(),data.c_str());
                }else {
                    INET_Write(socketw,data.size(),data.c_str());
                }
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
            static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
            if (playlist[whichlist].size()) {
              GotoSong(whichlist,random?randInt(playlist[whichlist].size()):playlist[whichlist].counter++%playlist[whichlist].size(),true);
            }else {
              fprintf (stderr,"Error no songs in playlist %d\n",whichlist);
            }
	    return;
	  }
	}
	SkipRandList();
}

void Music::SkipRandList() {
	for (unsigned int i=0;i<playlist.size();i++) {
          static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
          if (!playlist[i].empty())
            GotoSong(i,random?randInt(playlist[i].size()):playlist[i].counter++%playlist[i].size(),false);
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
	char send[2]={'t','\n'};
        if (soundServerPipes()) {
            fNET_Write(socketw,2,send);
            fNET_close(socketw);
            //fNET_close(socketr);
            fNET_cleanup();
        }else {
            INET_Write(socketw,2,send);
            INET_close(socketw);
            INET_cleanup();
        }
}
void incmusicvol (const KBData&, KBSTATE a) {
	if (a==PRESS) {
		muzak->ChangeVolume (.0625);
	}
}
void decmusicvol (const KBData&, KBSTATE a) {
	if (a==PRESS) {
		muzak->ChangeVolume (-.0625);
	}
}
