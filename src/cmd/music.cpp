

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

// To allow for loading in another thread, we must handle some AL vars ourselves...
#include "aldrv/al_globals.h"
#include <map>
#include <set>
#include <algorithm>

#define MAX_RECENT_HISTORY "5"

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
  music_load_info = NULL;
  killthread=0;
  threadalive=0;
  freeWav=true;
#ifdef HAVE_AL
  music_load_info = new AUDSoundProperties;
#endif
  
#ifdef _WIN32
  musicinfo_mutex = CreateMutex(NULL, TRUE, NULL);
#else
  pthread_mutex_init(&musicinfo_mutex, NULL);
  // Lock it immediately, since the loader will want to wait for its first data upon creation.
  pthread_mutex_lock(&musicinfo_mutex);
#endif
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

/*
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
*/
    string data=string("i")+vs_config->getVariable("audio","music_fadein","0")+"\n"
		"o"+vs_config->getVariable("audio","music_fadeout","0")+"\n";
/*
    if (soundServerPipes()) {
        fNET_Write(socketw,data.size(),data.c_str());
    }else {
        INET_Write(socketw,data.size(),data.c_str());
    }
*/
    soft_vol_up_latency  = XMLSupport::parse_float(vs_config->getVariable("audio","music_volume_up_latency","15"));
    soft_vol_down_latency= XMLSupport::parse_float(vs_config->getVariable("audio","music_volume_down_latency","2"));
    //Hardware volume = 1
    _SetVolume(1,true);
    //Software volume = from config
    _SetVolume(XMLSupport::parse_float(vs_config->getVariable("audio","music_volume",".5")),false);

}

void Music::ChangeVolume(float inc,int layer) 
{
    if (muzak) {
        if (layer<0) {
          for (int i=0; i<muzak_count; i++){
                muzak[i]._SetVolume(muzak[i].soft_vol + inc,false,0.1);
          }
        } else if ((layer>=0)&&(layer<muzak_count)) {
            muzak[layer]._SetVolume(muzak[layer].soft_vol + inc,false,0.1);
        }
    }
}
static float tmpmin(float a, float b) {return a<b?a:b;}
static float tmpmax(float a, float b) {return a<b?b:a;}

void Music::_SetVolume (float vol,bool hardware,float latency_override) {
/*
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
*/
  if (vol<0)vol=0;
  this->vol=vol;
  this->soft_vol=vol;//for now fixme for fading
  for (std::list<int>::const_iterator iter = playingSource.begin() ; iter != playingSource.end(); iter++ ) {
    AUDSoundGain(*iter, soft_vol,true);
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
      if (songname[0]=='\0')
	continue;
      if (songname[0]=='#') {
        if (strncmp(songname,"#pragma ",8)==0) {
          char *sep = strchr(songname+8,' ');
          if (sep) {
            *sep = 0;
            this->playlist.back().pragmas[songname+8] = sep+1;
          } else if(songname[8]) {
            this->playlist.back().pragmas[songname+8] = "1";
          }
        }
        continue;
      }
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
  static int maxrecent=XMLSupport::parse_int(vs_config->getVariable("audio","shuffle_songs.history_depth",MAX_RECENT_HISTORY));
  static std::string dj_script = vs_config->getVariable("sound","dj_script","modules/dj.py");
  if ((BaseInterface::CurrentBase||loopsleft>0)&&lastlist < (int)playlist.size()&&lastlist>=0) {
    if (loopsleft>0) {
      loopsleft--;
    }
    if (!playlist[lastlist].empty() && !playlist[lastlist].haspragma("norepeat")) {
      int whichsong=(random?rand():playlist[lastlist].counter++)%playlist[lastlist].size();
	  int spincount=10;
	  std::list<std::string> &recent = muzak[(layer>=0)?layer:0].recent_songs;
	  while (  random && (--spincount > 0) &&  (std::find(recent.begin(), recent.end(), playlist[lastlist][whichsong])!=recent.end())  )
		  whichsong=(random?rand():playlist[lastlist].counter++)%playlist[lastlist].size();
	  if (spincount<=0)
		  recent.clear();
	  recent.push_back(playlist[lastlist][whichsong]);
	  while (recent.size()>maxrecent)
		  recent.pop_front();
      GotoSong(lastlist,whichsong,true,layer);
      return whichsong;
    }
  }
  if (_Universe&&_Universe->numPlayers()){
    CompileRunPython (dj_script);
  }else {
    static std::string loading_tune=vs_config->getVariable("audio","loading_sound","../music/loading.ogg");
    GotoSong(loading_tune,layer);
  }
  return 0;
}

namespace Muzak {
  std::map<std::string, AUDSoundProperties> cachedSongs;
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
#ifdef _WIN32
		WaitForSingleObject(me->musicinfo_mutex, INFINITE);
#else
		pthread_mutex_lock(&me->musicinfo_mutex);
#endif
		if (me->killthread) break;
		me->music_loaded = false;
		me->music_load_info->success=false;
                size_t len=me->music_load_info->hashname.length();
                char *songname = (char*)malloc(len+1);
                songname[len]='\0';
                memcpy(songname,me->music_load_info->hashname.data(),len);
                std::map<std::string, AUDSoundProperties>::iterator wherecache=cachedSongs.find(songname);
                bool foundcache=wherecache!=cachedSongs.end();
                static std::string cachable_songs=vs_config->getVariable("audio","cache_songs","../music/land.ogg");
                bool docacheme=cachable_songs.find(songname)!=std::string::npos;
                if (foundcache==false&&docacheme) {
                  me->music_load_info->wave=NULL;
                  cachedSongs[songname]=*me->music_load_info;
                  wherecache=cachedSongs.find(songname);
                }
#ifdef _WIN32
		ReleaseMutex(me->musicinfo_mutex);
#else
		pthread_mutex_unlock(&me->musicinfo_mutex);
#endif
		{
                  me->freeWav=true;
                  if (foundcache) {
                    *me->music_load_info=wherecache->second;
                    me->freeWav=false;
                  }else if (!AUDLoadSoundFile(songname, me->music_load_info)) {
                    fprintf(stderr, "Failed to load song %s\n", songname);
                  }
		}

                if (me->freeWav&&docacheme) {
                  me->freeWav=false;
                  wherecache->second=*me->music_load_info;
                }
                free(songname);
		me->music_loaded = true;
		while (me->music_loaded) {
			micro_sleep(10000); // 10ms of busywait for now... wait until end of frame.
		}
	}
	me->threadalive=0;
	return NULL;
}

}

void Music::_LoadLastSongAsync() {
#ifdef HAVE_AL
	if (!music_load_info) return;
	std::string song = music_load_list.back();

        std::map<std::string, AUDSoundProperties>::iterator where=Muzak::cachedSongs.find(song);
        if (where!=Muzak::cachedSongs.end()) {
          if (where->second.wave!=NULL) {
            int source = AUDBufferSound(&where->second, true);
            AUDAdjustSound(source,QVector(0,0,0),Vector(0,0,0));
            music_load_info->wave=NULL;
            if (source!=-1) {
              playingSource.push_back(source);
            }
            if (playingSource.size()==1) { // Start playing if first in list.
              _StopNow();
              AUDStartPlaying(playingSource.front());
              // FIXME FIXME FIXME Presumed race condition or somesuch -- AUDSoundGain here breaks windows music -- temporary hack, actual fix later
              AUDSoundGain(playingSource.front(),vol,true);
            }
            return;
          }
        }
	music_load_info->hashname = song;
#endif
#ifdef _WIN32
	ReleaseMutex(musicinfo_mutex);
#else
	pthread_mutex_unlock(&musicinfo_mutex);
#endif
}

void Music::Listen() {
	if (g_game.music_enabled) {
          
		/*
            if (soundServerPipes()) {
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
		*/
		if (!music_load_list.empty()) {
			if (music_loaded) {
#ifdef _WIN32
				if (WaitForSingleObject(musicinfo_mutex, 0)==WAIT_TIMEOUT) {
#else
				if (pthread_mutex_trylock(&musicinfo_mutex)==EBUSY) {
#endif
					fprintf(stderr,"Failed to lock music loading mutex despite loaded flag being set...\n");
					return;
				}
				music_loaded = false; // once the loading thread sees this, it will try to grab a lock and wait.
				// The lock will only be achieved once the next song is put in the queue.
				
#ifdef HAVE_AL
				if (music_load_info->success && music_load_info->wave) {
					int source = AUDBufferSound(music_load_info, true);
                                        AUDAdjustSound(source,QVector(0,0,0),Vector(0,0,0));
                                        if (freeWav)
                                          free(music_load_info->wave);
					music_load_info->wave=NULL;
					if (source!=-1) {
						playingSource.push_back(source);
					}
				}
#endif
				if (playingSource.size()==1) { // Start playing if first in list.
                                  _StopNow();
                                  AUDStartPlaying(playingSource.front());
                                  // FIXME FIXME FIXME Presumed race condition or somesuch -- AUDSoundGain here breaks windows music -- temporary hack, actual fix later
                                  AUDSoundGain(playingSource.front(),vol,true);
				}
				music_load_list.pop_back();
				if (!music_load_list.empty()) {
					_LoadLastSongAsync();
				}
				return ; // Returns if finished loading, since the AUDIsPlaying() could fail right now.
			}
		}
		if (!playingSource.empty()) {
			if (!AUDIsPlaying(playingSource.front())) {
				AUDDeleteSound(playingSource.front(),true);
				playingSource.pop_front();
				if (!playingSource.empty()) {
                                  _StopNow();
                                  AUDStartPlaying(playingSource.front());
                                  AUDSoundGain(playingSource.front(),vol,true);
				}
			}
		}
		if (playingSource.empty() && muzak[muzak_cross_index].playingSource.empty()
			&& music_load_list.empty() && muzak[muzak_cross_index].music_load_list.empty()) {
			cur_song_file = "";
                        _Skip();
			
		}
	}
}

void Music::GotoSong (std::string mus,int layer)
{
    static bool cross = XMLSupport::parse_bool( vs_config->getVariable("audio","cross_fade_music","true") );
    if (cross && (muzak_count>=2)) {
        if (layer<0) {
            if (mus==muzak[muzak_cross_index].cur_song_file) return;
            muzak[muzak_cross_index]._StopLater();
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

std::vector<std::string> rsplit(std::string tmpstr,std::string splitter) {
  std::string::size_type where;
  std::vector<std::string> ret;
  while ((where=tmpstr.rfind(splitter))!=std::string::npos) {
    ret.push_back(tmpstr.substr(where+1));
    tmpstr= tmpstr.substr(0,where);
  }
  if (tmpstr.length())
    ret.push_back(tmpstr);
  return ret;
}

void Music::_GotoSong (std::string mus) {
	if (g_game.music_enabled) {
        if (mus==cur_song_file||mus.length()==0) return;
        cur_song_file = mus;

		_StopLater(); // Kill all our currently playing songs.
		
		music_load_list = rsplit(mus,"|"); // reverse order.
		if (!thread_initialized) {
#ifdef _WIN32
			a_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Muzak::readerThread, (PVOID)this, 0, NULL);
			if (a_thread) {
				thread_initialized = true;
			} else {
				fprintf(stderr, "Error creating music load thread: %d\n", GetLastError());
			}
#else
			if (pthread_create(&a_thread, NULL, Muzak::readerThread, this)==0) {
				thread_initialized = true;
			} else {
				perror("Error creating music load thread");
			}
#endif
		}
		_LoadLastSongAsync();
	/*
        if (soundServerPipes())
            fNET_Write(socketw,data.size(),data.c_str());
		else
            INET_Write(socketw,data.size(),data.c_str());
	*/
	}
}

void Music::GotoSong (int whichlist,int whichsong,bool skip,int layer) {
	if (g_game.music_enabled) {
		if (whichsong!=NOLIST&&whichlist!=NOLIST&&whichlist<(int)playlist.size()&&whichsong<(int)playlist[whichlist].size()) {
			if (muzak[(layer>=0)?layer:0].lastlist!=whichlist) {
				static bool clear = XMLSupport::parse_bool( vs_config->getVariable("audio","shuffle_songs.clear_history_on_list_change","true") );
				if (clear) {
					std::list<std::string> &recent = muzak[(layer>=0)?layer:0].recent_songs;
					recent.clear();
				}
			}
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
	if (threadalive&&thread_initialized) {
		killthread=1;
#ifdef _WIN32
		ReleaseMutex(musicinfo_mutex);
#else
		pthread_mutex_unlock(&musicinfo_mutex);
#endif
		int spindown = 50; // Thread has 5 seconds to close down.
		while (threadalive&&(spindown-- > 0)) micro_sleep(100000);
		if (threadalive) {
			/*
			// The thread should be dead to make exiting easier...
#ifdef _WIN32
			TerminateThread(a_thread, 1);
#else
			// Taking its time to load a song...
			pthread_kill(a_thread, SIGKILL);
#endif
			*/
			threadalive=false;
		}
	}
	
	// Kill the thread.
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
    muzak_count = XMLSupport::parse_int( vs_config->getVariable ("audio","music_layers","1") );
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
void Music::_StopNow() {
    if (g_game.music_enabled) {
		/*
        cur_song_file="";
	    char send[1]={'s'};
        if (soundServerPipes())
            fNET_Write(socketw,1,send); else
            INET_Write(socketw,1,send);
		*/
		for (std::vector<int>::const_iterator iter = sounds_to_stop.begin(); iter!=sounds_to_stop.end(); iter++) {
			int sound = *iter;
			AUDStopPlaying(sound);
			AUDDeleteSound(sound,true);
		}
                sounds_to_stop.clear();
	}

}
void Music::_StopLater() 
{
    if (g_game.music_enabled) {
		/*
        cur_song_file="";
	    char send[1]={'s'};
        if (soundServerPipes())
            fNET_Write(socketw,1,send); else
            INET_Write(socketw,1,send);
		*/
		for (std::list<int>::const_iterator iter = playingSource.begin(); iter!=playingSource.end(); iter++) {
			int sound = *iter;
                        sounds_to_stop.push_back(sound);
		}
		playingSource.clear();
	}
}

void Music::_Stop() 
{
    if (g_game.music_enabled) {
		/*
        cur_song_file="";
	    char send[1]={'s'};
        if (soundServerPipes())
            fNET_Write(socketw,1,send); else
            INET_Write(socketw,1,send);
		*/
		for (std::list<int>::const_iterator iter = playingSource.begin(); iter!=playingSource.end(); iter++) {
			int sound = *iter;
			AUDStopPlaying(sound);
			AUDDeleteSound(sound,true);
		}
		playingSource.clear();
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
