#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_
#include <AL/al.h>
#include <string>
#include <vector>
struct OurSound{
  ALuint source;
  ALint buffer;
  OurSound(ALuint source, ALuint buffername) {buffer=buffername;};
};

template <class a,class b,class c> class Hashtable;
extern std::vector<ALuint> buffers;
extern std::vector <OurSound> sounds;
extern Hashtable<std::string, ALuint,char [127]> soundHash;
typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
extern mp3Loader *alutLoadMP3p;


#endif
