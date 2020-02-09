#ifndef AL_GLOBALS_H_
#define AL_GLOBALS_H_

/* #undef SOUND_DEBUG */

#include <string>
#include <vector>

#ifdef HAVE_AL
#ifdef __APPLE__
#include <al.h>
#else
#include <AL/al.h>
#endif
#endif

struct AUDSoundProperties
{
    // Keep here all that is AL-independent
    
    bool success;
    
    //Hashing.
    //Set even if AUDLoadSoundFile fails so that a hash entry can be written.
    bool shared;
    std::string hashname;
    
    void     *wave;
    
    // From here on, AL-dependent stuff
    
#ifdef HAVE_AL
    //OpenAL properties.
    ALenum    format;
    ALsizei   size;
    ALsizei   freq;
    ALboolean looping;     //can be set by caller.
#endif

    AUDSoundProperties()
    {
        shared  = false;
        success = false;
        wave    = NULL;
#ifdef HAVE_AL
        looping = false;
        size    = 0;
        freq    = 22050;
#endif
    }
};

#ifdef HAVE_AL

class Vector;

struct OurSound
{
    ALuint    source;
    ALuint    buffer;
    Vector    pos;
    Vector    vel;
    float     gain;
    ALboolean looping;
    bool music;
    OurSound( ALuint source, ALuint buffername )
    {
            buffer = buffername;
        pos.Set( 0, 0, 0 );
        vel.Set( 0, 0, 0 );
            gain   = 1.0f;
    }
};

extern float scalepos;
extern float scalevel;
template < typename a, typename b, int c >
class Hashtable;

extern std::vector< ALuint >  unusedsrcs;
extern std::vector< ALuint >  buffers;
extern std::vector< OurSound >sounds;
extern Hashtable< std::string, ALuint, 127 >soundHash;
typedef ALboolean (mp3Loader)( ALuint, ALvoid*, ALint );
extern mp3Loader   *alutLoadMP3p;
extern unsigned int maxallowedsingle;
extern unsigned int maxallowedtotal;
extern bool usedoppler;
extern bool usepositional;

#endif

float AUDDistanceSquared( const int sound );
char AUDQueryAudability( const int sound, const Vector &pos, const Vector &vel, const float gain );
void AUDAddWatchedPlayed( const int sound, const Vector &pos );
bool AUDLoadSoundFile( const char *s, struct AUDSoundProperties *info, bool use_fileptr = false );

//It is up to the caller to free(info.wave) after using!!!
int AUDBufferSound( const struct AUDSoundProperties *info, bool music );

#endif

