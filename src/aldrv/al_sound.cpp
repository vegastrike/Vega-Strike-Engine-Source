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
  int buffer;//reference into buffers
  ALuint registeredbuffer;//registered buffer ... or -1
  OurSound(ALuint source, ALuint buffername) {buffer=buffername;};
};
std::vector <unsigned int> dirtysounds;
std::vector <OurSound> sounds;

struct ALBuffer {
  void *data;
  ALsizei size;	
  ALsizei bits;	
  ALsizei freq;	
  ALsizei format;
  enum {MP3,WAV} type;
  vector <ALuint> unusedbuffers;
  ALuint AUDGenerateBuffer ();
};

std::vector <ALBuffer> buffers;
using std::vector;
void AUDFreeAllBuffers () {
  for (unsigned int i=0;i<sounds.size();i++) {
    AUDDeleteSound (i);
  }
  sounds.clear();
  dirtysounds.clear();
  vector <ALBuffer>::iterator b;
  for (b=buffers.begin();b!=buffers.end();b++) {
    if (b->data) {
      free (b->data);
    }
    if (!b->unusedbuffers.empty()) {
      alDeleteBuffers (b->unusedbuffers.size(),b->unusedbuffers.begin());
    }
    b->unusedbuffers.clear();
  }
  buffers.clear();
}
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
  //  alSourcei(sounds[i].source, AL_BUFFER, buffer );//buffers cannot be shared among sources
  alSourcei(sounds[i].source, AL_LOOPING, looping ?AL_TRUE:AL_FALSE);
  sounds[i].buffer = buffer;
  sounds[i].registeredbuffer = (ALuint) 0;
  return i;
}
///Given an index to the buffers vector, it generates and loads a buffer from the saved data
int AUDCreateSoundWAV (const std::string &s, const bool LOOP=false){
#ifdef HAVE_AL
  ALuint * wavbuf = soundHash.Get(s);
  if (wavbuf==NULL) {
    wavbuf = (ALuint *) malloc (sizeof (ALuint));
    //alGenBuffers (1,wavbuf);
    ALsizei size;	
    ALsizei bits;	
    ALsizei freq;	
    ALsizei format;
    void *wave;
    ALboolean err = alutLoadWAV(s.c_str(), &wave, &format, &size, &bits, &freq);
    if(err == AL_FALSE) {
      return -1;
    }
    //    alBufferData( *wavbuf, format, wave, size, freq );
    buffers.push_back (ALBuffer());
    buffers.back().size=size;
    buffers.back().bits=bits;
    buffers.back().freq=freq;
    buffers.back().format=format;
    buffers.back().data = wave;
    buffers.back().type = ALBuffer::WAV;

    *wavbuf = buffers.size()-1;
    soundHash.Put (s,wavbuf);
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
    buffers.push_back (ALBuffer());
    buffers.back().size=length;
    buffers.back().data=data;
    buffers.back().type=ALBuffer::MP3;
    *mp3buf = buffers.size()-1;
    soundHash.Put (s,mp3buf);
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
    if (AUDIsPlaying (sound))
      AUDStopPlaying (sound);//redelivers buffer to "unused" pile
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
    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
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
    OurSound * snd = &sounds[sound];
    alSourceStop(snd->source);
    if (snd->registeredbuffer!=0) {
      alSourcei(snd->source, AL_BUFFER, 0 );//buffers cannot be shared among sources
      buffers[snd->buffer].unusedbuffers.push_back (snd->registeredbuffer);
      snd->registeredbuffer=0;
    }
  }
#endif
}
void AUDStartPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    OurSound * snd = &sounds[sound];
    if (snd->registeredbuffer==0) {
      snd->registeredbuffer = buffers[snd->buffer].AUDGenerateBuffer ();
      alSourcei (snd->source,AL_BUFFER,snd->registeredbuffer);
    }
    alSourcePlay( snd->source );
  }
#endif
}

ALuint ALBuffer::AUDGenerateBuffer () {
  ALuint buf;
  if (!unusedbuffers.empty()) {
    buf = unusedbuffers.back();
    unusedbuffers.pop_back();
  } else {
    alGenBuffers (1,&buf);
    switch (type) {
    case MP3:
      if ((*alutLoadMP3p)(buf,data,size)!=AL_TRUE) {alDeleteBuffers (1,&buf);return 0;}
      break;
    case WAV:
      alBufferData( buf, format, data, size, freq );
      break;
    }
  }
  return buf;
}
