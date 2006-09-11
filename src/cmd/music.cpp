

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

#include <set>
#include <algorithm>

#define MAX_RECENT_HISTORY 5

Music * muzak=NULL;
int muzak_count=0;
int muzak_cross_index=0;

bool soundServerPipes() {
    static bool ret=  XMLSupport::parse_bool(vs_config->getVariable("audio","pierce_firewall","true"));
    return ret;
}
Music::Music (Unit *parent):random(false), p(parent),song(-1),thread_initialized(false) {
  loopsleft=0;
  socketw=socketr=-1;
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

#if !defined( _WIN32)
  if (g_game.music_enabled&&!soundServerPipes()) {
    int pid=fork();
    if (!pid) {
	  string soundserver_path = VSFileSystem::datadir+"/bin/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),NULL);
      soundserver_path = VSFileSystem::datadir+"/soundserver";
      pid=execlp(soundserver_path.c_str() , soundserver_path.c_str(),NULL);
      g_game.music_enabled=false;
      VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server\n");
      exit (0);
    } else {
      if (pid==-1) {
        g_game.music_enabled=false;
      }
    }
  }
#endif
#if defined( _WIN32) && !defined( __CYGWIN__)
  if (g_game.music_enabled&&!soundServerPipes()) {
      string ss_path = VSFileSystem::datadir+"/soundserver.exe";
      int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),NULL);
      if (pid==-1) {
		ss_path = VSFileSystem::datadir+"/bin/soundserver.exe";
		chdir("bin");
		int pid=spawnl(P_NOWAIT,ss_path.c_str(),ss_path.c_str(),NULL);
		if (pid==-1) {
			g_game.music_enabled=false;
			VSFileSystem::vs_fprintf(stderr,"Unable to spawn music player server Error (%d)\n",pid);
		}
      }
  }
#endif

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
    soft_vol_up_latency  = XMLSupport::parse_float(vs_config->getVariable("audio","music_volume_up_latency","15"));
    soft_vol_down_latency= XMLSupport::parse_float(vs_config->getVariable("audio","music_volume_down_latency","2"));
    //Hardware volume = 1
    _SetVolume(1,true);
    //Software volume = from config
    _SetVolume(XMLSupport::parse_float(vs_config->getVariable("audio","music_volume",".5")),false);
  }

}

void Music::ChangeVolume(float inc,int layer) 
{
    if (muzak) {
        if (layer<0) {
            for (int i=0; i<muzak_count; i++)
                muzak[i]._SetVolume(muzak[i].soft_vol + inc,false,0.1);
        } else if ((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._SetVolume(muzak[layer].soft_vol + inc,false,0.1);
        }
    }
}
static float tmpmin(float a, float b) {return a<b?a:b;}
static float tmpmax(float a, float b) {return a<b?b:a;}

void Music::_SetVolume (float vol,bool hardware,float latency_override) {
    vol = tmpmax(0.0f,tmpmin((hardware?1.0f:2.0f),vol));
	char tempbuf [100];
    if (  (hardware&&(this->vol==vol))
        ||(!hardware&&(this->soft_vol==vol))  )
        return;
    if (hardware)
	    sprintf(tempbuf,"vh%f\n",vol); else
        sprintf(tempbuf,"vs%f\n%f\n",vol,((latency_override<0)?((vol>soft_vol)?soft_vol_up_latency:soft_vol_down_latency):latency_override));
    if (hardware)
        this->vol = vol; else
        this->soft_vol = vol;
    if (soundServerPipes()) 
        fNET_Write(socketw,strlen(tempbuf),tempbuf); else
        INET_Write(socketw,strlen(tempbuf),tempbuf);
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

int Music::SelectTracks(int layer) {
  static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
  static std::string dj_script = vs_config->getVariable("sound","dj_script","modules/dj.py");
  if ((BaseInterface::CurrentBase||loopsleft>0)&&lastlist < (int)playlist.size()&&lastlist>=0) {
    if (loopsleft>0) {
      loopsleft--;
    }
    if (!playlist[lastlist].empty()) {
      int whichsong=(random?rand():playlist[lastlist].counter++)%playlist[lastlist].size();
	  int spincount=10;
	  std::list<std::string> &recent = muzak[(layer>=0)?layer:0].recent_songs;
	  while (  random && (--spincount > 0) &&  (std::find(recent.begin(), recent.end(), playlist[lastlist][whichsong])!=recent.end())  )
		  whichsong=(random?rand():playlist[lastlist].counter++)%playlist[lastlist].size();
	  if (spincount<=0)
		  recent.clear();
	  recent.push_back(playlist[lastlist][whichsong]);
	  while (recent.size()>MAX_RECENT_HISTORY)
		  recent.pop_front();
      GotoSong(lastlist,whichsong,true,layer);
      return whichsong;
    }
  }
  CompileRunPython (dj_script);
  
  return 0;
  /*
  int whichlist=0;
  static float hostile_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","hostile_auto_radius","1000"));
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
  */
}

namespace Muzak {

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
    Music *me = (Music*)input;
	int socketr = me->socketr;
    me->threadalive=1;
	while(!me->killthread) {
          printf ("Reading from socket %d\n",socketr);
          char data=fNET_fgetc(socketr);
          printf ("Got data froms ocket %c\n",data);
          if (data=='e')
            me->moredata=1;
          micro_sleep(100000);
	}
	return NULL;
}

}

void Music::Listen() {
	if (g_game.music_enabled) {
            if (soundServerPipes()) {
                killthread=0;
                threadalive=0;
                if (!thread_initialized) {
#ifdef _WIN32
                    a_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Muzak::readerThread, (PVOID)this, 0, NULL);
#else
                    int res = pthread_create(&a_thread, NULL, Muzak::readerThread, (void*)this);
#endif
                    thread_initialized = 1;
                }
		//int bytes=fNET_BytesToRead(socketr);
		if (moredata) {
			moredata=0;
            cur_song_file = "";
            _Skip();
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
                cur_song_file = "";
				_Skip();
			} else {
				g_game.music_enabled=false;
			}
		}                
            }	
	}
}

void Music::GotoSong (std::string mus,int layer)
{
    static bool cross = XMLSupport::parse_bool( vs_config->getVariable("audio","cross_fade_music","true") );
    if (cross && (muzak_count>=2)) {
        if (layer<0) {
            if (mus==muzak[muzak_cross_index].cur_song_file) return;
            muzak[muzak_cross_index]._Stop();
            muzak_cross_index = (muzak_cross_index ^ 1);
            muzak[muzak_cross_index]._GotoSong(mus);
        } else if ((layer>=0)&&(layer<muzak_count)) {
            if (mus==muzak[layer].cur_song_file) return;
            muzak[layer]._GotoSong(mus);
        }
    } else {
        muzak->_GotoSong(mus);
    }
}

void Music::_GotoSong (std::string mus) {
	if (g_game.music_enabled) {
        if (mus==cur_song_file) return;
        cur_song_file = mus;

		string data=string("p")+mus+string("\n");
        if (soundServerPipes())
            fNET_Write(socketw,data.size(),data.c_str()); else
            INET_Write(socketw,data.size(),data.c_str());
	}
}

void Music::GotoSong (int whichlist,int whichsong,bool skip,int layer) {
	if (g_game.music_enabled) {
		if (whichsong!=NOLIST&&whichlist!=NOLIST&&whichlist<(int)playlist.size()&&whichsong<(int)playlist[whichlist].size()) {
            if ((layer<0)&&(muzak_count>=2))
                muzak[0].lastlist=muzak[1].lastlist=whichlist; else
                lastlist=whichlist;
			GotoSong(playlist[whichlist][whichsong],layer);
		} else {
			_SkipRandList(layer);
		}
	}
}

void Music::SkipRandSong(int whichlist, int layer)
{
    if (muzak) {
        if (layer<0) {
            if (muzak_count>=2)
                muzak[muzak_cross_index]._SkipRandSong(whichlist); else
                muzak->_SkipRandSong(whichlist);
        } else if ((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._SkipRandSong(whichlist,layer);
        }
    }
}

void Music::_SkipRandSong(int whichlist, int layer) {
	if (this!=NULL) {
	  if (whichlist!=NOLIST&&whichlist>=0&&whichlist<(int)playlist.size()){
	    lastlist = whichlist;
        static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
        if (playlist[whichlist].size()) {
          GotoSong(whichlist,random?randInt(playlist[whichlist].size()):playlist[whichlist].counter++%playlist[whichlist].size(),true,layer);
        }else {
          fprintf (stderr,"Error no songs in playlist %d\n",whichlist);
        }
	    return;
	  }
	}
	_SkipRandList(layer);
}

void Music::SkipRandList(int layer) 
{
    if (muzak) {
        if (layer<0) {
            if (muzak_count>=2)
                muzak[muzak_cross_index]._SkipRandList(); else
                muzak->_SkipRandList();
        } else if ((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._SkipRandList(layer);
        }
    }
}

void Music::_SkipRandList(int layer) {
	for (unsigned int i=0;i<playlist.size();i++) {
          static bool random=XMLSupport::parse_bool(vs_config->getVariable("audio","shuffle_songs","true"));
          if (!playlist[i].empty())
            GotoSong(i,random?randInt(playlist[i].size()):playlist[i].counter++%playlist[i].size(),false,layer);
	}
}

int Music::Addlist(std::string listfile)
{
    int res=-1;
    if (muzak) res=muzak->_Addlist(listfile);
    if (muzak) for (int i=1; i<muzak_count; i++) muzak[i]._Addlist(listfile);
    return res;
}

int Music::_Addlist (std::string listfile) {
	bool retval=LoadMusic(listfile.c_str());
	if (retval) {
	  return playlist.size()-1;
	}else {
	  return -1;
	}
}

void Music::Skip(int layer) 
{
    if (muzak) {
        if (layer<0) {
            if (muzak_count>=2)
                muzak[muzak_cross_index]._Skip(); else
                muzak->_Skip();
        } else if ((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._Skip(layer);
        }
    }
}

void Music::_Skip(int layer) 
{
	if (g_game.music_enabled)
        SelectTracks(layer);
}

Music::~Music() 
{
	char send[2]={'t','\n'};
	if (socketw != -1) {
		if (soundServerPipes()) {
			if (threadalive&&thread_initialized) {
				killthread=1;
				int spindown = 50; // Thread has 5 seconds to close down.
				while (threadalive&&(spindown-- > 0)) micro_sleep(100000);
			}

			fNET_Write(socketw,2,send);
			fNET_close(socketw);
			//fNET_close(socketr);
			fNET_cleanup();
		}else {
			INET_Write(socketw,2,send);
			INET_close(socketw);
			INET_cleanup();
		}
		socketw=-1;
	}
}
void incmusicvol (const KBData&, KBSTATE a) 
{
	if (a==PRESS) Music::ChangeVolume (.0625);
}
void decmusicvol (const KBData&, KBSTATE a) 
{
	if (a==PRESS) Music::ChangeVolume (-.0625);
}

void Music::SetParent(Unit * parent)
{
    p=parent;
}

void Music::InitMuzak() 
{
    muzak_count = XMLSupport::parse_int( vs_config->getVariable ("audio","music_layers","2") );
    muzak = new Music[muzak_count];
}

void Music::CleanupMuzak()
{
    if (muzak) {
        delete[] muzak;
        muzak=NULL;
        muzak_count=0;
    }
}

void Music::MuzakCycle()
{
    if (muzak) for (int i=0; i<muzak_count; i++) 
        muzak[i].Listen();
}

void Music::Stop(int layer)
{
    if (muzak) {
        if (layer<0) {
            if (muzak_count>=2)
                muzak[muzak_cross_index]._Stop(); else
                muzak->_Stop();
        } else if((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._Stop();
        }
    }
}

void Music::_Stop() 
{
    if (g_game.music_enabled) {
        cur_song_file="";
	    char send[1]={'s'};
        if (soundServerPipes())
            fNET_Write(socketw,1,send); else
            INET_Write(socketw,1,send);
	}
}

void Music::SetVolume(float vol, int layer,bool hardware,float latency_override)
{
    if (muzak) {
        if (layer<0) {
            for (int i=0; i<muzak_count; i++)
                muzak[i]._SetVolume(vol,hardware,latency_override);
        } else if((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._SetVolume(vol,hardware,latency_override);
        }
    }
}

void Music::Mute(bool mute, int layer)
{
    static vector<float> saved_vol;
    saved_vol.resize(muzak_count,-1);

    if (muzak) {
        static float muting_fadeout = XMLSupport::parse_float( vs_config->getVariable ("audio","music_muting_fadeout","0.2") );
        static float muting_fadein  = XMLSupport::parse_float( vs_config->getVariable ("audio","music_muting_fadeout","0.5") );
        if (layer<0) {
            for (int i=0; i<muzak_count; i++) {
                if (mute) {
                    if (muzak[i].soft_vol != 0) {
                        saved_vol[i]=muzak[i].soft_vol;
                        muzak[i]._SetVolume(0,false,muting_fadeout);
                    }
                } else {
                    if (saved_vol[i]>=0)
                        muzak[i]._SetVolume(saved_vol[i],false,muting_fadein);
                }
            }
        } else if((layer>=0)&&(layer<muzak_count)) {
            if (mute) {
                if (muzak[layer].soft_vol != 0) {
                    saved_vol[layer]=muzak[layer].soft_vol;
                    muzak[layer]._SetVolume(0,false,muting_fadeout);
                }
            } else {
                if (saved_vol[layer]>=0)
                    muzak[layer]._SetVolume(saved_vol[layer],false,muting_fadein);
            }
        }
    }
}

void Music::SetLoops(int numloops, int layer)
{
    if (muzak) {
        if (layer<0) {
            //This only will apply to the crossfading channel (layers 0 && 1)
            SetLoops(numloops,0);
            SetLoops(numloops,1);
        } else if ((layer>=0)&&(layer<muzak_count)) {
            //Specific channel
            muzak[layer].loopsleft = numloops;
        }
    }
}
