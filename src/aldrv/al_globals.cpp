#include "al_globals.h"
#include "hashtable.h"
mp3Loader *alutLoadMP3p = 0;
Hashtable<std::string, ALuint,char [127]> soundHash;
