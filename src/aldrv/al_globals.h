#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_

/* #undef SOUND_DEBUG */

#ifdef HAVE_AL
#ifdef __APPLE__
#include <al.h>
#else
#include <AL/al.h>
#endif
#include <string>
#include <vector>
class Vector;
struct OurSound{
  ALuint source;
  ALuint buffer;
  Vector pos;
  Vector vel;
  ALboolean looping;
  OurSound(ALuint source, ALuint buffername) {buffer=buffername;pos.Set(0,0,0);vel.Set(0,0,0);};
};
extern float scalepos;
extern float scalevel;
template <typename a,typename b,int c> class Hashtable;

extern std::vector <ALuint> unusedsrcs;
extern std::vector<ALuint> buffers;
extern std::vector <OurSound> sounds;
extern Hashtable<std::string, ALuint,127> soundHash;
float AUDDistanceSquared(const int sound);
typedef ALboolean (mp3Loader)(ALuint, ALvoid *, ALint);
extern mp3Loader *alutLoadMP3p;
extern unsigned int maxallowedsingle;
extern unsigned int maxallowedtotal;
extern bool usedoppler;
extern bool usepositional;
char AUDQueryAudability (const int sound, const Vector &pos, const Vector & vel, const float gain);
void AUDAddWatchedPlayed (const int sound, const Vector &pos);
#endif
#endif
