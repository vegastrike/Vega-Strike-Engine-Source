//#define HAVE_SDL
#if !defined (SDL_MIX_MAXVOLUME)
#define SDL_MIX_MAXVOLUME 128
#endif
#ifdef HAVE_SDL
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mixer.h>
#else
typedef int Mix_Music;
#endif

#include <signal.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define sleep(sec) Sleep(sec*1000);
#else
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#endif
#include <stdarg.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include "inet_file.h"
#include "inet.h"
#ifndef _WIN32
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#endif
int fadeout=0, fadein=0;
float volume=0;
int bits=0,done=0;
static bool fnet;
int my_getchar(int socket){
  if (fnet) {
    return fNET_fgetc(socket);
  }else {
    return INET_fgetc(socket);
  }
}
#if defined(_WIN32)&&defined(_WINDOWS)
FILE *mystdout=stdout;
#define STD_ERR mystdout
#define STD_OUT mystdout
#else
#define STD_ERR stderr
#define STD_OUT stdout
#endif
#ifdef __APPLE
#undef main
#endif
#include "SDL_mutex.h"
/******************************************************************************/
/* some simple exit and error routines                                        */
char * songNames[5]={0,0,0,0,0};
unsigned int counter=0;
SDL_mutex * RestartSong=NULL;
void errorv(char *str, va_list ap)
{
#ifdef HAVE_SDL
	vfprintf(STD_ERR,str,ap);

	fprintf(STD_ERR,": %s.\n", SDL_GetError());
#endif
}
vector<string> split(string tmpstr,string splitter) {
  string::size_type where;
  vector<string> ret;
  while ((where=tmpstr.find(splitter))!=string::npos) {
    ret.push_back(tmpstr.substr(0,where));
    tmpstr= tmpstr.substr(where+1);
  }
  if (tmpstr.length())
    ret.push_back(tmpstr);
  return ret;
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
std::string HOMESUBDIR=".vegastrike";
/******************************************************************************/
/* The main function                                                          */
void changehome (bool to, bool linuxhome=true) {
	static std::vector <std::string> paths;
  if (to) {
	 char mycurpath[8192];
	 getcwd(mycurpath,8191);
	 mycurpath[8191]='\0';
	 paths.push_back (mycurpath);
#ifndef _WIN32
	 if (linuxhome) {
	   struct passwd *pwent;
	   pwent = getpwuid (getuid());
	   chdir (pwent->pw_dir);
	 }
#endif
	chdir (HOMESUBDIR.c_str());
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
volatile bool sende=true;
std::string curmus;
static int numloops (std::string file) {
  int value=1;
  std::string::size_type where;
  if ((where=file.find_last_of("."))!=std::string::npos) {
    file =file.substr(0,where);
    if ((where=file.find_last_of("-_"))!=std::string::npos) {
      file=file.substr(where+1);
      if (file.length()) {
        if (file[0]=='i') {
          return -1;
        }else {
          return atoi(file.c_str());
        }
      }
    }
  }
  return value;
}
Mix_Music * PlayMusic (const char * file, Mix_Music *oldmusic) {
#ifdef HAVE_SDL
	Mix_Music *music=Mix_LoadMUS(file);
	if(music==NULL){
	  changehome (true,false);
	  music=Mix_LoadMUS(file);
	  changehome (false);
	  if(music==NULL){
	    changehome (true,true);
	    music=Mix_LoadMUS(file);
	    changehome(false);
	    if (music==NULL) {
	      return oldmusic;
	    }
	  }
	}
	sende=false;
	if (Mix_HaltMusic()) {
	}
	while(Mix_PlayingMusic()) {
		SDL_Delay(100);
	}
	if (oldmusic) {
		Mix_FreeMusic(oldmusic);
		oldmusic=NULL;
	}
	sende=true;
        int loops=numloops(file);
	if(Mix_PlayMusic(music, loops)==-1) {
	  fprintf(STD_OUT, "Mix_FadeInMusic: %s %d\n", Mix_GetError());
	  return NULL;
	}else {
          fprintf (STD_OUT,"Playing %s with %d loops\n",file,loops);
        }
	
	// well, there's no music, but most games don't break without music...
	int newvolume=(int)(SDL_MIX_MAXVOLUME*volume);
	Mix_VolumeMusic(newvolume);
	return music;
#else
	return NULL;
#endif
}
int mysocket = -1;
int mysocket_write=-1;
Mix_Music *music=NULL;
void music_finished () {
	if (sende) {
		char data='e';
                SDL_mutexP(RestartSong);
                int tmp = counter;
                char * newname=NULL;
                if (tmp<5) {
                  newname=songNames[tmp];
                  counter++;
                }
                if (newname) {
                  music=PlayMusic(newname,music);
                  SDL_mutexV(RestartSong);
                }else {
                  SDL_mutexV(RestartSong);
                  if (fnet) {
                    fNET_Write(mysocket_write,sizeof(char),&data);	
                  }else {
                    INET_Write(mysocket_write,sizeof(char),&data);	
                  }
                }
		fprintf(STD_OUT, "\ne\n[SONG DONE]\n");
	}
}
#if defined(_WIN32)&&defined(_WINDOWS)
typedef char FileNameCharType [65535];
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd) {
	FileNameCharType argvc;
	FileNameCharType *argv= &argvc;
	GetModuleFileName(NULL, argvc, 65534);
	mystdout=fopen("soundserver_stdout.txt", "w");
	if (mystdout) {
		setbuf(mystdout, NULL); /* No buffering */
	} else {
		mystdout=stdout;
	}
#else
int main(int argc, char **argv) {
#endif
	{
	char origpath[65535];
	getcwd (origpath,65534);
	origpath[65534]=0;
        fprintf(STD_OUT,"Current Path %s\n",origpath);
#ifdef _WIN32
        int i;
	for (i=strlen(argv[0]);argv[0][i]!='\\'&&argv[0][i]!='/'&&i>=0;i--) {
	}
	argv[0][i+1]='\0';
	chdir(argv[0]);
#endif	
	struct stat st;
	if (stat("vegastrike.config",&st)!=0) {
		//vegastrike.config not found.  Let's check ../
		chdir (".."); //gotta check outside bin dir
	}
        getcwd(origpath,65534);
        fprintf(STD_OUT,"Final Path %s\n",origpath);
	}


		FILE *version=fopen("Version.txt","r");
		
		if (version) {
			std::string hsd="";
			int c;
			while ((c=fgetc(version))!=EOF) {
				if (((c)==' ')||((c)=='\t')||((c)=='\n')||((c)=='\r')||((c)=='\0'))
					break;
				hsd+=(char)c;
			}
			fclose(version);
			if (hsd.length()) {
				HOMESUBDIR=hsd;
				fprintf (STD_OUT,"Using %s as the home directory\n",hsd.c_str());
			}			
		}
		



	music=NULL;
	int audio_rate,audio_channels,
 		// set this to any of 512,1024,2048,4096
		// the higher it is, the more FPS shown and CPU needed
		audio_buffers=4096;
#ifdef HAVE_SDL
	Uint16 audio_format;
	// initialize SDL for audio and video

	if(SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER)<0)
		cleanExit("SDL_Init\n");
        
        signal( SIGSEGV, SIG_DFL );
        RestartSong = SDL_CreateMutex();
	Mix_HookMusicFinished(&music_finished); 
#endif
        fnet=(argc==3);
        if (fnet) {
          fNET_startup();
        }else {
          INET_startup();
        }
	// initialize sdl mixer, open up the audio device
#ifdef HAVE_SDL
	if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,audio_buffers)<0)
		cleanExit("Mix_OpenAudio\n");

	// print out some info on the audio device and stream
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
	bits=audio_format&0xFF;
#endif
	fprintf(STD_OUT, "Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
			bits, audio_channels>1?"stereo":"mono", audio_buffers );

	// load the song
        if (fnet) {
          mysocket_write = open (argv[2],O_WRONLY|O_CREAT,0xffffffff);
		  flock(mysocket_write,LOCK_SH);
          mysocket = open (argv[1],O_RDONLY|O_CREAT,0xffffffff);
		  flock(mysocket,LOCK_SH);
        }else {
          for (int i=0;i<10&&mysocket==-1;i++) {
            int port = argc==2?atoi(argv[1]):4364;
            if (port==0) port=4364;
            mysocket =mysocket_write= INET_AcceptFrom(port,"localhost");
          }
        }
	if (mysocket==-1)
		return 1;
	fprintf(STD_OUT, "\n[CONNECTED]\n");
	fflush(STD_OUT);
	char ministr[2]={'\0','\0'};
	while (!done) {
//		if ((Mix_PlayingMusic() || Mix_PausedMusic())&&(!done)) {
		char arg;
		std::string str;
		arg=my_getchar(mysocket);
		fprintf(STD_OUT, "%c",arg);
		fflush(STD_OUT);
		switch(arg) {
		case 'p':
		case 'P':
			{
				arg=my_getchar(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=my_getchar(mysocket);
				}
				fprintf(STD_OUT, "%s",str.c_str());
				fflush(STD_OUT);
				if ((str!=curmus)
#ifdef HAVE_SDL
				    ||(!Mix_PlayingMusic())
#endif
				    ) {
                                  vector<string> names = split(str,"|");                                 
                                  char * tmpstrings[5]={NULL,NULL,NULL,NULL,NULL};
                                  for(unsigned int t=0;t<5&&t+1<names.size();++t) {
                                    tmpstrings[t]=strdup(names[t+1].c_str());
                                  }
                                  SDL_mutexP(RestartSong);
                                  memcpy(songNames,tmpstrings,sizeof(char*)*5);
                                  if (names.size()>0) str=names[0];
                                  counter=0;
                                  music=PlayMusic(str.c_str(),music);
                                  SDL_mutexV(RestartSong);
					if (music) {
						fprintf(STD_OUT, "\n[PLAYING %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),fadein,fadeout);
						curmus=str;
					} else {
                                          char mycurpath[8192];
                                          getcwd(mycurpath,8191);
                                          mycurpath[8191]='\0';
                                          fprintf(STD_OUT, "\n[UNABLE TO PLAY %s IN %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),mycurpath,fadein,fadeout);
					}
				} else { 
					fprintf(STD_OUT, "\n[%s WITH %d FADEIN AND %d FADEOUT IS ALREADY PLAYING]\n",str.c_str(),fadein,fadeout);
				}
				fflush(STD_OUT);
			}
			break;
		case 'i':
		case 'I':
			{
				arg=my_getchar(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=my_getchar(mysocket);
				}
				fprintf(STD_OUT, "%s",str.c_str());
				fadein=atoi(str.c_str());
				fprintf(STD_OUT, "\n[SETTING FADEIN TO %d]\n",fadein);
				fflush(STD_OUT);
			}
			break;
		case 'o':
		case 'O':
			{
				arg=my_getchar(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=INET_fgetc(mysocket);
				}
				fprintf(STD_OUT, "%s",str.c_str());
				fadeout=atoi(str.c_str());
				fprintf(STD_OUT, "\n[SETTING FADEOUT TO %d]\n",fadeout);
				fflush(STD_OUT);
			}
			break;
		case 'v':
		case 'V':
			{
				arg=my_getchar(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=my_getchar(mysocket);
				}
				fprintf(STD_OUT, "%s",str.c_str());
				volume=atof(str.c_str());
				fprintf(STD_OUT, "\n[SETTING VOLUME TO %f]\n",volume);
				fflush(STD_OUT);
#ifdef HAVE_SDL
				int newvolume=(int)(SDL_MIX_MAXVOLUME*volume);
				Mix_VolumeMusic(newvolume);
#endif
			}
			break;
		case 't':
		case 'T':
		case '\0':
                  if (fnet) {
                    fNET_close (mysocket);
                    fNET_close (mysocket_write);
                  }else {
                    INET_close (mysocket);
                  }
			done=true;
			fprintf(STD_OUT, "\n[TERMINATING MUSIC SERVER]\n");
			fflush(STD_OUT);
			break;
		}
	}
	// free & close
        if (fnet) {
          fNET_cleanup();
          unlink(argv[1]);
          unlink(argv[2]);

        }else {
          INET_cleanup();
        }
#ifdef HAVE_SDL
	Mix_CloseAudio();
        SDL_DestroyMutex(RestartSong);
	SDL_Quit();
#endif

	return(0);
}
