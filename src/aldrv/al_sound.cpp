#include "audiolib.h"
#include "hashtable.h"
#include "vsfilesystem.h"
#include <string>
#include "al_globals.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmd/unit_generic.h"
#include "gfx/cockpit_generic.h"
#ifdef HAVE_AL
#ifdef __APPLE__
#include <al.h>
#include <alc.h>
#include <alut.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//their LoadWav is b0rken seriously!!!!!!

bool MacFixedLoadWAVFile(VSFileSystem::VSFile & f,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq){
	long length = f.Size();
    char * buf = (char *) malloc (length);
    f.Read(buf,length);
    alutLoadWAVMemory(buf,format,data,size,freq);
    free(buf);
    return true;
}

#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#endif
//#include <AL/alext.h>
#endif
#include <vector>
#include "vs_globals.h"
#include <algorithm>
#ifdef HAVE_AL
std::vector <unsigned int> dirtysounds;
std::vector <OurSound> sounds;
std::vector <ALuint> buffers;

static int LoadSound (ALuint buffer, bool looping) {
  unsigned int i;
  if (!dirtysounds.empty()) {
    i = dirtysounds.back();
    dirtysounds.pop_back();
    //    assert (sounds[i].buffer==(ALuint)0);
    if (sounds[i].buffer!=(ALuint)0) {
      VSFileSystem::vs_fprintf (stderr,"using claimed buffer %d",sounds[i].buffer);
    }
    sounds[i].buffer= buffer;
  } else {
    i=sounds.size();
    sounds.push_back (OurSound (0,buffer));
  }
  sounds[i].source = (ALuint)0;
  sounds[i].looping = looping?AL_TRUE:AL_FALSE;
  //limited number of sources
  //  alGenSources( 1, &sounds[i].source);
  //alSourcei(sounds[i].source, AL_BUFFER, buffer );
  //alSourcei(sounds[i].source, AL_LOOPING, looping ?AL_TRUE:AL_FALSE);
  return i;

}
#endif

using namespace VSFileSystem;

int AUDCreateSoundWAV (const std::string &s, const bool music, const bool LOOP){
#ifdef HAVE_AL
  if ((g_game.sound_enabled&&!music)||(g_game.music_enabled&&music)) {
	VSFile f;
	VSError error = f.OpenReadOnly( s.c_str(), SoundFile);
    bool shared=(error==Shared);

	if( error <= Ok)
	{
	    ALuint * wavbuf =NULL;
	    std::string hashname;
	    if (!music)
		{
	      hashname = shared?VSFileSystem::GetSharedSoundHashName(s):VSFileSystem::GetHashName (s);
	      wavbuf = soundHash.Get(hashname);
	    }
	    if (wavbuf==NULL)
		{
	      wavbuf = (ALuint *) malloc (sizeof (ALuint));
	      alGenBuffers (1,wavbuf);
	      ALsizei size;	
	      ALsizei freq;
	      void *wave;
		  ALboolean looping;
	      ALboolean err=AL_TRUE;
#ifndef WIN32
#ifdef __APPLE__
		  ALint format;
		  // MAC OS X
		  err = false;
		  if( error<=Ok)
			err=MacFixedLoadWAVFile( f, &format, &wave, &size, &freq);
#else
		  // LINUX
		  ALsizei format;
	  	  char * dat = new char[f.Size()];
	  	  f.Read( dat, f.Size());
      	  alutLoadWAVMemory((ALbyte *)dat, &format, &wave, &size, &freq, &looping);
	  	  delete []dat;
#endif
#else
		  ALint format;
	  	  // WIN32
	  	  char * dat = new char[f.Size()];
	  	  f.Read( dat, f.Size());
      	  alutLoadWAVMemory(dat, (int*)&format, &wave, &size, &freq, &looping);
	  	  delete []dat;
#endif
      	  if(err == AL_FALSE)
		  {
			alDeleteBuffers (1,wavbuf);
			free (wavbuf);
			return -1;
      	  }
      	  alBufferData( *wavbuf, format, wave, size, freq );
      	  free(wave);
      	  if (!music)
		  {
			soundHash.Put (hashname,wavbuf);
			buffers.push_back (*wavbuf);
      	  }
		}
		f.Close();
    	return LoadSound (*wavbuf,LOOP);  
    }
	else
	{
		// File not found
		return -1;
	}
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
	VSFile f;
	VSError error = f.OpenReadOnly( s.c_str(), SoundFile);
    bool shared=(error==Shared);
    std::string nam (s);
    ALuint * mp3buf=NULL;
    std::string hashname;
    if (!music) {
      hashname = shared?VSFileSystem::GetSharedSoundHashName(s):VSFileSystem::GetHashName (s);
      mp3buf = soundHash.Get (hashname);
    }
	if( error>Ok)
		return -1;
#ifdef _WIN32
	return -1;
#endif
    if (mp3buf==NULL) {
	  char * data = new char[f.Size()];
	  f.Read( data, f.Size());
      mp3buf = (ALuint *) malloc (sizeof (ALuint));
      alGenBuffers (1,mp3buf);
      if ((*alutLoadMP3p)(*mp3buf,data,f.Size())!=AL_TRUE) {
	    delete []data;
		return -1;
      }
	  delete []data;
      if (!music) {
		soundHash.Put (hashname,mp3buf);
		buffers.push_back (*mp3buf);
      }
    }
	else
		f.Close();
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
    if (sounds[sound].source){
      unusedsrcs.push_back (sounds[sound].source);
      sounds[sound].source=(ALuint)0;
    }
#ifdef SOUND_DEBUG
    if (std::find (dirtysounds.begin(),dirtysounds.end(),sound)==dirtysounds.end()) {
#endif
      dirtysounds.push_back (sound);
#ifdef SOUND_DEBUG
    }else {
      VSFileSystem::vs_fprintf (stderr,"double delete of sound");
      return;
    }
#endif
    //FIXME??
    //    alDeleteSources(1,&sounds[sound].source);
    if (music) {
      alDeleteBuffers (1,&sounds[sound].buffer);
    }

    sounds[sound].buffer=(ALuint)0;
  }
#endif
}
void AUDAdjustSound (const int sound, const QVector &pos, const Vector &vel){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
    float p []= {scalepos*pos.i,scalepos*pos.j,scalepos*pos.k};
    float v []= {scalevel*vel.i,scalevel*vel.j,scalevel*vel.k};
    sounds[sound].pos = pos.Cast();
	sounds[sound].vel=vel;
	if (usepositional&&sounds[sound].source)
	    alSourcefv(sounds[sound].source,AL_POSITION,p);
  if (usedoppler&&sounds[sound].source)
    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
  }
#endif
}
void AUDSoundGain (const int sound, const float gain) {
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()&&sounds[sound].source) {
    alSourcef(sounds[sound].source,AL_GAIN,gain);
    //    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
  }
#endif
}
bool starSystemOK( ) {
	Unit * playa = _Universe->AccessCockpit(0)->GetParent();
	if (!playa)
		return false;
	return playa->getStarSystem()==_Universe->activeStarSystem();

}
void AUDStopAllSounds () {
#ifdef HAVE_AL
	unsigned int s = ::sounds.size();
	for (unsigned int i=0;i < s;++i) {
		if (AUDIsPlaying(i))
			AUDStopPlaying(i);
	}
#endif
}
bool AUDIsPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
	if (!sounds[sound].source) 
		return false;
    ALint state;
#if defined (_WIN32) || defined (__APPLE__)
    alGetSourcei(sounds[sound].source,AL_SOURCE_STATE, &state);  //Obtiene el estado de la fuente para windows
#else
    alGetSourceiv(sounds[sound].source, AL_SOURCE_STATE, &state);
#endif

    return (state==AL_PLAYING);
  }
#endif
  return false;
}
void AUDStopPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
	if (sounds[sound].source!=0) {
	  alSourceStop(sounds[sound].source);
      unusedsrcs.push_back (sounds[sound].source);
	}
    sounds[sound].source=(ALuint)0;
  }
#endif
}
static bool AUDReclaimSource (const int sound) {
#ifdef HAVE_AL
  if (sounds[sound].source==(ALuint)0) {
    if (unusedsrcs.empty()||(!sounds[sound].buffer))
      return false;
    sounds[sound].source = unusedsrcs.back();
    unusedsrcs.pop_back();
    alSourcei(sounds[sound].source, AL_BUFFER, sounds[sound].buffer );
    alSourcei(sounds[sound].source, AL_LOOPING, sounds[sound].looping);    
  }
  return true;
#endif		
  return false;//silly
}
void AUDStartPlaying (const int sound){
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
	  if (starSystemOK())
    if (AUDReclaimSource (sound)) {
	  AUDAdjustSound (sound, sounds[sound].pos, sounds[sound].vel);

      alSourcePlay( sounds[sound].source );
    }
  }
#endif
}

void AUDPlay (const int sound, const QVector &pos, const Vector & vel, const float gain) {
#ifdef HAVE_AL
  char tmp;
  if (sound<0)
    return;
  if (sounds[sound].buffer==0) {
	return;
  }
  if (!starSystemOK())
	  return;
  if ((tmp=AUDQueryAudability (sound,pos.Cast(),vel,gain))!=0) {
    if (AUDReclaimSource (sound)) {
      //ALfloat p [3] = {pos.i,pos.j,pos.k};
      AUDAdjustSound (sound,pos,vel);
      alSourcef(sounds[sound].source,AL_GAIN,gain);    
      if (tmp!=2){
		AUDAddWatchedPlayed (sound,pos.Cast());
		alSourcePlay( sounds[sound].source );
		//AUDAdjustSound (sound,pos,vel);
		//alSourcef(sounds[sound].source,AL_GAIN,gain);    

      }

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
