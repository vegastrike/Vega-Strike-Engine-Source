#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_
#include <AL/al.h>



typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
extern mp3Loader *alutLoadMP3p;


#endif
