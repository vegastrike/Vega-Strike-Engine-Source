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
#endif
#include <stdarg.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include "inet.h"
int fadeout=0, fadein=0;
Sint16 stream[2][4096];
int len=4096, done=0, bits=0, which=0;
Uint32 flips=0;

/******************************************************************************/
/* some simple exit and error routines                                        */

void errorv(char *str, va_list ap)
{
	vfprintf(stderr,str,ap);
	fprintf(stderr,": %s.\n", SDL_GetError());
}

void cleanExit(char *str,...)
{
	va_list ap;
	va_start(ap, str);
	errorv(str,ap);
	va_end(ap);
	Mix_CloseAudio();
	SDL_Quit();
	exit(1);
}

/******************************************************************************/
/* The main function                                                          */
void changehome (bool to) {
	static std::vector <std::string> paths;
  if (to) {
	 char mycurpath[8192];
	 getcwd(mycurpath,8191);
	 mycurpath[8191]='\0';
	 paths.push_back (mycurpath);
#ifndef _WIN32
	struct passwd *pwent;
	pwent = getpwuid (getuid());
	chdir (pwent->pw_dir);
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
	Mix_Music *music;
	bool home=false;
	if((music=Mix_LoadMUS(file))==NULL){
		changehome (true);
		if((music=Mix_LoadMUS(file))==NULL){
			changehome(false);
			return oldmusic;
		}
		changehome(false);
	}

	sende=false;
	while(!Mix_FadeOutMusic(fadeout) /*&& Mix_PlayingMusic()*/) {
	// wait for any fades to complete
		if (!Mix_PlayingMusic()) {
			break;
		}		
			SDL_Delay(100);

	}
	if (oldmusic) {
		Mix_FreeMusic(oldmusic);
		oldmusic=NULL;
	}
	sende=true;
	if (Mix_GetMusicType(NULL)==MUS_MID) {
		if(Mix_PlayMusic(music, 1)==-1) {
			printf("Mix_PlayMusic: %s\n", Mix_GetError());
			return NULL;
		}
	} else {
		if(Mix_FadeInMusic(music, 1, fadein)==-1) {
			printf("Mix_FadeInMusic: %s\n", Mix_GetError());
			return NULL;
		}
	}
	// well, there's no music, but most games don't break without music...
//	int volume=SDL_MIX_MAXVOLUME;
//	Mix_VolumeMusic(volume);
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
	Mix_Music *music=NULL,*tempmusic=NULL;
	int audio_rate,audio_channels,
		// set this to any of 512,1024,2048,4096
		// the higher it is, the more FPS shown and CPU needed
		audio_buffers=4096;
	Uint16 audio_format;
	int volume=SDL_MIX_MAXVOLUME;

	// initialize SDL for audio and video
	if(SDL_Init(SDL_INIT_AUDIO)<0)
		cleanExit("SDL_Init\n");

	Mix_HookMusicFinished(&music_finished); 
	INET_startup();
	// initialize sdl mixer, open up the audio device
	if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,audio_buffers)<0)
		cleanExit("Mix_OpenAudio\n");

	// print out some info on the audio device and stream
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
	bits=audio_format&0xFF;
	printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
			bits, audio_channels>1?"stereo":"mono", audio_buffers );

	// load the song
	while (mysocket==-1) {
		mysocket = INET_AcceptFrom(4364,"localhost");
	}
	printf("\n[CONNECTED]\n");
	unsigned long bytes=0;
	char ministr[2]={'\0','\0'};
	while (!done) {
//		if ((Mix_PlayingMusic() || Mix_PausedMusic())&&(!done)) {
		char arg;
		std::string str;
		arg=INET_fgetc(mysocket);
		printf("%c",arg);
		switch(arg) {
		case 'p':
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
				if ((str!=curmus)||(!Mix_PlayingMusic())) {
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
		case 't':
		case '\0':
			INET_close (mysocket);
			done=true;
			printf("\n[TERMINATING MUSIC SERVER]\n");
			break;
		}
	}
	// free & close
	Mix_CloseAudio();
	INET_cleanup();
	SDL_Quit();
	return(0);
}
