#include "audiolib.h"
#include "hashtable.h"
#include <string>
#include "al_globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#include <vector>
struct OurSound{
  ALuint source;
  ALint buffer;
  OurSound(ALuint source, ALuint buffername) {buffer=buffername;};
};
std::vector <unsigned int> dirtysounds;
std::vector <OurSound> sounds;
std::vector <ALuint> buffers;
static int LoadSound (ALuint buffer, bool looping) {
  unsigned int i;
  if (!dirtysounds.empty()) {
    i = dirtysounds.back();
    dirtysounds.pop_back();
    assert (sounds[i].buffer==-1);
    sounds[i].buffer= buffer;
  } else {
    i=sounds.size();
    sounds.push_back (OurSound (0,buffer));
  }
  alGenSources( 1, &sounds[i].source);
  alSourcei(sounds[i].source, AL_BUFFER, buffer );
  alSourcei(sounds[i].source, AL_LOOPING, looping ?AL_TRUE:AL_FALSE);
  return i;
}

int AUDCreateSoundWAV (const std::string &s, const bool LOOP=false){
#ifdef HAVE_AL
  ALuint * wavbuf = soundHash.Get(s);
  if (wavbuf==NULL) {
    wavbuf = (ALuint *) malloc (sizeof (ALuint));
    alGenBuffers (1,wavbuf);
    ALsizei size;	
    ALsizei bits;	
    ALsizei freq;	
    ALsizei format;
    void *wave;
    ALboolean err = alutLoadWAV(s.c_str(), &wave, &format, &size, &bits, &freq);
    if(err == AL_FALSE) {
      return -1;
    }
    alBufferData( *wavbuf, format, wave, size, freq );
    free(wave);
    soundHash.Put (s,wavbuf);
    buffers.push_back (*wavbuf);
  }
  return LoadSound (*wavbuf,LOOP);  
#endif
  return -1;
}
int AUDCreateSoundMP3 (const std::string &s, const bool LOOP=false){
#ifdef HAVE_AL
  ALuint * mp3buf = soundHash.Get (s);
  if (mp3buf==NULL) {
    FILE * fp = fopen (s.c_str(),"rb");
    if (!fp)
      return -1;
    fseek (fp,0,SEEK_END);
    long length = ftell (fp);
    rewind (fp);
    char *data = (char *)malloc (length);
    fread (data,1,length,fp);
    fclose (fp);
    mp3buf = (ALuint *) malloc (sizeof (ALuint));
    alGenBuffers (1,mp3buf);
    if ((*alutLoadMP3p)(*mp3buf,data,length)!=AL_TRUE) {
      free (data);
      return -1;
    }
    free (data);
    soundHash.Put (s,mp3buf);
    buffers.push_back (*mp3buf);
  }
  return LoadSound (*mp3buf,LOOP);
#endif
  return -1;
}
///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP=false){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size())
    return LoadSound (sounds[sound].buffer,LOOP);
#endif
  return -1;
}
void AUDDeleteSound (int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    dirtysounds.push_back (sound);
    alDeleteSources(1,&sounds[sound].source);
    sounds[sound].buffer=-1;
    //if refcount 0, delete buffer? I think we save that for deinit
  }
#endif
}
void AUDAdjustSound (const int sound, const Vector &pos, const Vector &vel){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    float p []= {pos.i,pos.j,pos.k};
    float v []= {vel.i,vel.j,vel.k};
    alSourcefv(sounds[sound].source,AL_POSITION,p);
    //    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
  }
#endif
}

bool AUDIsPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    ALint state;
    alGetSourceiv(sounds[sound].source, AL_SOURCE_STATE, &state);
    return (state==AL_PLAYING);
  }
#endif
  return false;
}
void AUDStopPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    alSourceStop(sounds[sound].source);
  }
#endif
}
void AUDStartPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    alSourcePlay( sounds[sound].source );
  }
#endif
}
void AUDPausePlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    //    alSourcePlay( sounds[sound].source() );
  }
#endif
}
