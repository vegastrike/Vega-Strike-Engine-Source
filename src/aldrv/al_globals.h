#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_
#include <AL/al.h>
#include <string>
#include <vector>
class Vector;
struct OurSound{
  ALuint source;
  ALuint buffer;
  Vector pos;
  OurSound(ALuint source, ALuint buffername) {buffer=buffername;};
};

template <class a,class b,class c> class Hashtable;
extern std::vector<ALuint> buffers;
extern std::vector <OurSound> sounds;
extern Hashtable<std::string, ALuint,char [127]> soundHash;
typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
extern mp3Loader *alutLoadMP3p;
extern unsigned int maxallowedsingle;
extern unsigned int maxallowedtotal;
char AUDQueryAudability (const int sound, const Vector &pos, const Vector & vel, const float gain);
void AUDAddWatchedPlayed (const int sound, const Vector &pos);
#endif
