int GetMaxVolume();
#include "lin_time.h"
#ifdef __APPLE__
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
extern "C" {
#include "../fmod.h"
}
#include <queue>
#include <list>
using std::list;
using std::queue;
bool e_already_sent=false;
void music_finished();
signed char endcallback (FSOUND_STREAM * stream, void * buf, int len, int param) {
    if (!buf) {
        music_finished();
    }
    return 0;
}
struct Music {
    FSOUND_STREAM * m;
    int channel;
    Music() {
        m=NULL;channel=0;
    }
    void Stop() {
        if (m) FSOUND_Stream_Stop(m);
    }
    void Free () {
        if (m) {
	  const float ram_limit=47;
	  static list <FSOUND_STREAM *> q;
	  q.push_back (m);
	  if (q.size()>ram_limit) {
	    FSOUND_Stream_Close(q.front());
	    q.pop_front();
	    
	  }
	}
        m=NULL;
    }
    bool Load(const char * file) {
        m=FSOUND_Stream_OpenFile (file,FSOUND_NORMAL | FSOUND_MPEGACCURATE,0);
        return true;
    }
    /*
     
    void Play(float fadeout, float fadein, Music &oldmusic){
        if (!m) return;
        FSOUND_Stream_SetEndCallback(m,endcallback,0);
        channel = FSOUND_Stream_PlayEx(FSOUND_FREE, m, NULL, 1);
        SetVolume(0);
        FSOUND_SetPaused(channel, 0);
        if (fadeout*100>1) {
            for (unsigned int i=0;i<fadeout*100;i++) {
                SetVolume(i/(float)fadeout);
                oldmusic.SetVolume(((float)fadeout-i)/fadeout);
                micro_sleep (10000);
            }
        }
        SetVolume(1);
        oldmusic.Stop();
    }
    void SetVolume(float vol) {
        if (m) {
            F_API FSOUND_SetVolume(this->channel,(int)(vol*GetMaxVolume()));
	    }
    }
    void SetMasterVolume(float vol) {
    FSOUND_SetSFXMasterVolume((int) (vol*GetMaxVolume()));
    }
     */
    void Play(float fadeout, float fadein, Music &oldmusic){
        if (!m) return;
        if (fadeout) {
        FSOUND_Stream_SetEndCallback(m,endcallback,0);
        FSOUND_Stream_SetSynchCallback(m, endcallback, 0);
        channel = FSOUND_Stream_Play(FSOUND_FREE, m);
        if (!e_already_sent) {
            SetVolume(0);
            if (fadeout*10>1) {
                for (unsigned int i=0;i<fadeout*10;i++) {
                    float ratio = ((float)i)/(fadeout*10.);
                    SetVolume(ratio);
                    if (!e_already_sent)
                        oldmusic.SetVolume(1-ratio);
                    micro_sleep (10000);
                }
            }
            if (!e_already_sent)//race condition I know  I know--not much we can do bout it..besides this ole girl crashes when she wishes.
                oldmusic.Stop();
        }
        }else {
            if (!e_already_sent)
                oldmusic.Stop();
        }
        oldmusic.Free();
        if (!fadeout) {
	  FSOUND_Stream_SetEndCallback(m,endcallback,0);
	  channel = FSOUND_Stream_Play(FSOUND_FREE, m);
        }	
  
        SetVolume(1);
        e_already_sent=false;
    }
  void SetMasterVolume(float vol) {
    FSOUND_SetSFXMasterVolume((int) (vol*GetMaxVolume()));
  }
    void SetVolume(float vol) {
        if (m) {
            printf ("Setting %d to %d\n", this->channel, (int)(vol*GetMaxVolume()));
            F_API FSOUND_SetVolume(this->channel,(int)(vol*GetMaxVolume()));
        }
    }    
};
#else
#define HAVE_SDL
#ifdef HAVE_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mixer.h>
#else
typedef int Mix_Music;
#endif
struct Music {
    Mix_Music * m;
    Music () {
        m=NULL;
    }
    bool Load(const char * file) {
        m = Mix_LoadMUS (file);
        return m!=NULL;
    }
    bool Play(float fadeout, float fadein, Music &oldmusic){
        if (m) {
        int fadesteps = fadeout*100;
        for (int i=fadesteps;i>=0;i--) {
            oldmusic.SetVolume(i/(float)fadesteps);
            micro_sleep (10000);
        }        
        Mix_FadeInMusic(m,1,fadein)!=-1;
        micro_sleep (fadein*1000000);
        
        return true;
        }else {
            return false;
        }
    }
    void Stop() {
        if (m) Mix_StopMusic(m);
    }
    void Free () {
        if (m) Mix_FreeMusic(m);
    }
    void SetVolume(float vol) {
        if (m) {
            Mix_VolumeMusic(vol*GetMaxVolume());
        }
    }
  void SetMasterVolume(float vol) {
    SetVolume(vol);
  }
}
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define sleep(sec) Sleep(sec*1000);
#else
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#endif
#endif
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>



#include "inet_file.h"
int fadeout=0, fadein=0;
float volume=0;
int bits=0,done=0;

/******************************************************************************/
/* some simple exit and error routines                                        */
void errorv(char *str, va_list ap)
{
#ifdef HAVE_SDL
	vfprintf(stderr,str,ap);

	fprintf(stderr,": %s.\n", SDL_GetError());
#endif
}
int GetMaxVolume () {
#ifdef __APPLE__
  static int maxVol = FSOUND_GetSFXMasterVolume();
  return maxVol;
#else
#ifdef HAVE_SDL
  return   SDL_MIX_MAXVOLUME;
#else
  return 100
#endif
#endif
}
void cleanExit(char *str,...)
{
#ifdef HAVE_SDL
	va_list ap;
	va_start(ap, str);
	errorv(str,ap);
	va_end(ap);
	Mix_CloseAudio();
	SDL_Quit();
#endif
	exit(1);
}

/******************************************************************************/
/* The main function                                                          */
void changehome (bool to, bool linuxhome=true) {
	static std::vector <std::string> paths;
  if (to) {
	 char mycurpath[8192];
	 getcwd(mycurpath,8191);
	 mycurpath[8191]='\0';
	 paths.push_back (std::string(mycurpath));
#ifndef _WIN32
	 if (linuxhome) {
	   struct passwd *pwent;
	   pwent = getpwuid (getuid());
	   chdir (pwent->pw_dir);
	 }
#endif
	chdir (".vegastrike");
  }else {
	  if (!paths.empty()) {
		chdir (paths.back().c_str());
		paths.pop_back();
	  }
  }
}
#ifdef _WIN32
#undef main
#endif
bool sende=true;
bool invalid_string=true;
std::string curmus;


Music PlayMusic (const char * file, Music &oldmusic) {
    Music music;
    music.Load(file);

    if(music.m==NULL){
        changehome (true,false);
        music.Load(file);
        changehome (false);
        if(music.m==NULL){
            changehome (true,true);
            music.Load(file);
            changehome(false);
            if (music.m==NULL) {
                return oldmusic;
            }	
	}
    }
    sende=false;
    music.Play (fadeout,fadein,oldmusic);
    
    sende=true;
    curmus=file;
    invalid_string=false;
    return music;
}
int mysocket_read = -1;
int mysocket_write=-1;
void music_finished () {
	if (sende) {
		char data='e';
                e_already_sent=true;
		fNET_Write(mysocket_write,sizeof(char),&data);	
		printf("\ne\n[SONG DONE]\n");
                invalid_string=true;
	}
}
int main(int argc, char **argv)
{
	Music music;
	int audio_rate,audio_channels,
		// set this to any of 512,1024,2048,4096
		// the higher it is, the more FPS shown and CPU needed
		audio_buffers=4096;
#ifdef HAVE_SDL
	Uint16 audio_format;
	// initialize SDL for audio and video
	if(SDL_Init(SDL_INIT_AUDIO)<0)
		cleanExit("SDL_Init\n");

	Mix_HookMusicFinished(&music_finished); 
#else
#ifdef __APPLE__
        if (!FSOUND_Init(44100, 64, FSOUND_INIT_GLOBALFOCUS))
	  {
	    printf("SOUND Error %d\n", FSOUND_GetError());
	    exit(1);
	  }
#endif
#endif
	fNET_startup();
	GetMaxVolume();
	// initialize sdl mixer, open up the audio device
#ifdef HAVE_SDL
	if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,audio_buffers)<0)
		cleanExit("Mix_OpenAudio\n");

	// print out some info on the audio device and stream
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
	bits=audio_format&0xFF;
#endif
	printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
			bits, audio_channels>1?"stereo":"mono", audio_buffers );

	// load the song
	printf ("argv %d\n\n\n", argc);
        if (argc!=3) {
	for (int i=0;i<10&&mysocket_write==-1;i++) {
            mysocket_write = fNET_AcceptFrom(4364,"localhost");
            mysocket_read = mysocket_write;
	}
	if (mysocket_write==-1)
		return 1;
        }else {
            mysocket_write = open (argv[2],O_WRONLY|O_SHLOCK|O_CREAT,0xffffffff);
            mysocket_read = open (argv[1],O_RDONLY|O_SHLOCK|O_CREAT,0xffffffff);
        }
	printf("\n[CONNECTED]\n");
	char ministr[2]={'\0','\0'};
	while (!done) {
//		if ((Mix_PlayingMusic() || Mix_PausedMusic())&&(!done)) {
		char arg;
		std::string str;
		arg=fNET_fgetc(mysocket_read);
		printf("%c",arg);
		switch(arg) {
		case 'p':
		case 'P':
			{
				arg=fNET_fgetc(mysocket_read);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=fNET_fgetc(mysocket_read);
				}
				printf("%s",str.c_str());
				if ((str!=curmus||invalid_string)
#ifdef HAVE_SDL
				    ||(!Mix_PlayingMusic())
#endif
				    ) {
					music=PlayMusic(str.c_str(),music);
					if (music.m) {
                                            printf("\n[PLAYING %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),fadein,fadeout);
                                            curmus=str;
                                            invalid_string=false;
					} else {
                                            printf("\n[UNABLE TO PLAY %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),fadein,fadeout);
                                            music_finished();
					}
				} else {
					printf("\n[%s WITH %d FADEIN AND %d FADEOUT IS ALREADY PLAYING]\n",str.c_str(),fadein,fadeout);
				}
			}
			break;
		case 'i':
		case 'I':
			{
				arg=fNET_fgetc(mysocket_read);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=fNET_fgetc(mysocket_read);
				}
				printf("%s",str.c_str());
				fadein=atoi(str.c_str());
				printf("\n[SETTING FADEIN TO %d]\n",fadein);
			}
			break;
		case 'o':
		case 'O':
			{
				arg=fNET_fgetc(mysocket_read);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=fNET_fgetc(mysocket_read);
				}
				printf("%s",str.c_str());
				fadeout=atoi(str.c_str());
				printf("\n[SETTING FADEOUT TO %d]\n",fadeout);
			}
			break;
		case 'v':
		case 'V':
			{
				arg=fNET_fgetc(mysocket_read);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=fNET_fgetc(mysocket_read);
				}
				printf("%s",str.c_str());
				volume=atof(str.c_str());
				printf("\n[SETTING VOLUME TO %f]\n",volume);
				music.SetMasterVolume(volume);
			}
			break;
		case 't':
		case 'T':
		case '\0':
			fNET_close (mysocket_read);
                    if (mysocket_read!=mysocket_write) {
                        fNET_close (mysocket_write);
                    }
			done=true;
			printf("\n[TERMINATING MUSIC SERVER]\n");
			break;
		}
	}
	// free & close
	fNET_cleanup();
#ifdef HAVE_SDL
	Mix_CloseAudio();
	SDL_Quit();
#endif

	return(0);
}
