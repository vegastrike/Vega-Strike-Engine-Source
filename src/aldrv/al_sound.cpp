#include "audiolib.h"
#include "hashtable.h"
#include "vs_path.h"
#include <string>
#include "al_globals.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_AL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>
#endif
#include <vector>
#include "vs_globals.h"
#ifdef HAVE_AL
std::vector <unsigned int> dirtysounds;
std::vector <OurSound> sounds;
std::vector <ALuint> buffers;

static int LoadSound (ALuint buffer, bool looping) {
  unsigned int i;
  if (!dirtysounds.empty()) {
    i = dirtysounds.back();
    dirtysounds.pop_back();
    assert (sounds[i].buffer==(ALuint)0);
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
#endif
int AUDCreateSoundWAV (const std::string &s, const bool music, const bool LOOP){
#ifdef HAVE_AL
  if ((g_game.sound_enabled&&!music)||(g_game.music_enabled&&music)) {
    FILE * fp = fopen (s.c_str(),"rb");
    bool shared=false;
    std::string nam (s);
    if (fp) {
      fclose (fp);
    }else {
      nam = GetSharedSoundPath (s);
      shared=true;
    }
    ALuint * wavbuf =NULL;
    std::string hashname;
    if (!music) {
      hashname = shared?GetSharedSoundHashName(s):GetHashName (s);
      wavbuf = soundHash.Get(hashname);
    }
    if (wavbuf==NULL) {
      wavbuf = (ALuint *) malloc (sizeof (ALuint));
      alGenBuffers (1,wavbuf);
      ALsizei size;	
      ALsizei bits;	
      ALsizei freq;	
      ALsizei format;
      void *wave;
      ALboolean err = alutLoadWAV(nam.c_str(), &wave, &format, &size, &bits, &freq);
      if(err == AL_FALSE) {
	return -1;
      }
      alBufferData( *wavbuf, format, wave, size, freq );
      free(wave);
      if (!music) {
	soundHash.Put (hashname,wavbuf);
	buffers.push_back (*wavbuf);
      }
    }
    return LoadSound (*wavbuf,LOOP);  
  }
#endif
  return -1;
}
int AUDCreateSoundWAV (const std::string &s, const bool LOOP) {
  return AUDCreateSoundWAV (s,false,LOOP);
}
int AUDCreateMusicWAV (const std::string &s, const bool LOOP) {
  return AUDCreateSoundWAV (s,true,LOOP);
}

int AUDCreateSoundMP3 (const std::string &s, const bool music, const bool LOOP){
#ifdef HAVE_AL
  if ((g_game.sound_enabled&&!music)||(g_game.music_enabled&&music)) {
    FILE * fp = fopen (s.c_str(),"rb");
    bool shared=false;
    std::string nam (s);
    if (fp) {
      fclose (fp);
    }else {
      nam = GetSharedSoundPath (s);
      shared=true;
    }
    ALuint * mp3buf=NULL;
    std::string hashname;
    if (!music) {
      hashname = shared?GetSharedSoundHashName(s):GetHashName (s);
      mp3buf = soundHash.Get (hashname);
    }
    if (mp3buf==NULL) {
      FILE * fp = fopen (nam.c_str(),"rb");
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
      if (!music) {
	soundHash.Put (hashname,mp3buf);
	buffers.push_back (*mp3buf);
      }
    }
    return LoadSound (*mp3buf,LOOP);
  }
#endif
  return -1;
}

int AUDCreateSoundMP3 (const std::string &s, const bool LOOP) {
  return AUDCreateSoundMP3 (s,false,LOOP);
}
int AUDCreateMusicMP3 (const std::string &s, const bool LOOP) {
  return AUDCreateSoundMP3 (s,true,LOOP);
}
int AUDCreateSound (const std::string &s,const bool LOOP) {
  if (s.end()-1>=s.begin()){
    if (*(s.end()-1)=='3') {
      return AUDCreateSoundMP3 (s,LOOP);
    } else {
      return AUDCreateSoundWAV (s,LOOP);
    }
  }
  return -1;
}
int AUDCreateMusic (const std::string &s,const bool LOOP) {
  if (s.end()-1>=s.begin()){
    if (*(s.end()-1)=='v') {
      return AUDCreateMusicWAV (s,LOOP);
    } else {
      return AUDCreateMusicMP3 (s,LOOP);
    }
  }
  return -1;
}

///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP/*=false*/){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size())
    return LoadSound (sounds[sound].buffer,LOOP);
#endif
  return -1;
}
extern std::vector <int> soundstodelete;
void AUDDeleteSound (int sound, bool music){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    if (AUDIsPlaying (sound)) {
      if (!music) {
	soundstodelete.push_back(sound);
	return;
      } else
	AUDStopPlaying (sound);
    }
    dirtysounds.push_back (sound);
    alDeleteSources(1,&sounds[sound].source);
    if (music) {
      alDeleteBuffers (1,&sounds[sound].buffer);
    }
    sounds[sound].buffer=(ALuint)0;
  }
#endif
}
void AUDAdjustSound (const int sound, const Vector &pos, const Vector &vel){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    float p []= {pos.i,pos.j,pos.k};
    float v []= {vel.i,vel.j,vel.k};
    sounds[sound].pos = pos;
    alSourcefv(sounds[sound].source,AL_POSITION,p);
    //    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
  }
#endif
}
void AUDSoundGain (const int sound, const float gain) {
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    alSourcef(sounds[sound].source,AL_GAIN,gain);
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

void AUDPlay (const int sound, const Vector &pos, const Vector & vel, const float gain) {
#ifdef HAVE_AL
  char tmp;
  if (sound<0)
    return;
  if ((tmp=AUDQueryAudability (sound,pos,vel,gain))!=0) {

    ALfloat p [3] = {pos.i,pos.j,pos.k};
    AUDAdjustSound (sound,pos,vel);
    alSourcef(sounds[sound].source,AL_GAIN,gain);    
    if (tmp!=2){
      AUDAddWatchedPlayed (sound,pos);
      alSourcePlay( sounds[sound].source );
    }
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
