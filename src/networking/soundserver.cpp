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
#include "inet.h"
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
	 paths.push_back (mycurpath);
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
std::string curmus;
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
	if (Mix_FadeOutMusic(fadeout)) {
	}
	while(Mix_PlayingMusic()) {
		SDL_Delay(100);
	}
	if (oldmusic) {
		Mix_FreeMusic(oldmusic);
		oldmusic=NULL;
	}
	sende=true;
	if(Mix_FadeInMusic(music, 1, fadein)==-1) {
	  printf("Mix_FadeInMusic: %s\n", Mix_GetError());
	  return NULL;
	}
	
	// well, there's no music, but most games don't break without music...
	int newvolume=SDL_MIX_MAXVOLUME*volume;
	Mix_VolumeMusic(newvolume);
	return music;
#else
	return NULL;
#endif
}
int mysocket = -1;

void music_finished () {
	if (sende) {
		char data='e';
		INET_Write(mysocket,sizeof(char),&data);	
		printf("\ne\n[SONG DONE]\n");
	}
}
int main(int argc, char **argv)
{
	Mix_Music *music=NULL;
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
#endif
	INET_startup();
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
	for (int i=0;i<10&&mysocket==-1;i++) {
		mysocket = INET_AcceptFrom(4364,"localhost");
	}
	if (mysocket==-1)
		return 1;
	printf("\n[CONNECTED]\n");
	char ministr[2]={'\0','\0'};
	while (!done) {
//		if ((Mix_PlayingMusic() || Mix_PausedMusic())&&(!done)) {
		char arg;
		std::string str;
		arg=INET_fgetc(mysocket);
		printf("%c",arg);
		switch(arg) {
		case 'p':
		case 'P':
			{
				arg=INET_fgetc(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=INET_fgetc(mysocket);
				}
				printf("%s",str.c_str());
				if ((str!=curmus)
#ifdef HAVE_SDL
				    ||(!Mix_PlayingMusic())
#endif
				    ) {
					music=PlayMusic(str.c_str(),music);
					if (music) {
						printf("\n[PLAYING %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),fadein,fadeout);
						curmus=str;
					} else {
						printf("\n[UNABLE TO PLAY %s WITH %d FADEIN AND %d FADEOUT]\n",str.c_str(),fadein,fadeout);
					}
				} else {
					printf("\n[%s WITH %d FADEIN AND %d FADEOUT IS ALREADY PLAYING]\n",str.c_str(),fadein,fadeout);
				}
			}
			break;
		case 'i':
		case 'I':
			{
				arg=INET_fgetc(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=INET_fgetc(mysocket);
				}
				printf("%s",str.c_str());
				fadein=atoi(str.c_str());
				printf("\n[SETTING FADEIN TO %d]\n",fadein);
			}
			break;
		case 'o':
		case 'O':
			{
				arg=INET_fgetc(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=INET_fgetc(mysocket);
				}
				printf("%s",str.c_str());
				fadeout=atoi(str.c_str());
				printf("\n[SETTING FADEOUT TO %d]\n",fadeout);
			}
			break;
		case 'v':
		case 'V':
			{
				arg=INET_fgetc(mysocket);
				while (arg!='\0'&&arg!='\n') {
					if (arg!='\r') {
						ministr[0]=arg;
						str+=ministr;
					}
					arg=INET_fgetc(mysocket);
				}
				printf("%s",str.c_str());
				volume=atof(str.c_str());
				printf("\n[SETTING VOLUME TO %f]\n",volume);
#ifdef HAVE_SDL
				int newvolume=SDL_MIX_MAXVOLUME*volume;
				Mix_VolumeMusic(newvolume);
#endif
			}
			break;
		case 't':
		case 'T':
		case '\0':
			INET_close (mysocket);
			done=true;
			printf("\n[TERMINATING MUSIC SERVER]\n");
			break;
		}
	}
	// free & close
	INET_cleanup();
#ifdef HAVE_SDL
	Mix_CloseAudio();
	SDL_Quit();
#endif

	return(0);
}
