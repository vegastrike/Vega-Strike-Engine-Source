#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_
#include <AL/al.h>
#include <string>

template <class a,class b,class c> class Hashtable;
extern Hashtable<std::string, ALuint,char [127]> soundHash;
typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
extern mp3Loader *alutLoadMP3p;


#endif
