
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

bool MacFixedLoadWAVFile(char * buf, ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq){
    ALboolean bleh=false;
    alutLoadWAVMemory((ALbyte*)buf,format,data,size,freq,&bleh);
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
#include <stdio.h>
#ifdef HAVE_AL
#ifdef HAVE_OGG

#include <vorbis/vorbisfile.h>
#endif
std::vector <unsigned int> dirtysounds;
std::vector <OurSound> sounds;
std::vector <ALuint> buffers;

static void convertToLittle(unsigned int tmp, char * data){
  data[0]=(char)(tmp%256);
  data[1]=(char)((tmp/256)%256);
  data[2]=(char)((tmp/65536)%256);
  data[3]=(char)((tmp/65536)/256);  
}
#ifdef HAVE_OGG
struct fake_file{
  char * data;
  size_t size;
  size_t loc;
};
size_t mem_read(void * ptr, size_t size, size_t nmemb, void * datasource) {
  fake_file * fp = (fake_file*)datasource;
  if (fp->loc+size>fp->size) {
    size_t tmp=fp->size-fp->loc;
    if (tmp)
      memcpy (ptr,fp->data+fp->loc,tmp);
    fp->loc=fp->size;
    return tmp;
  }else {
    memcpy(ptr,fp->data+fp->loc,size);
    fp->loc+=size;
    return size;
  }
}
int mem_close(void*) {
  return 0;
}

long   mem_tell  (void *datasource){
 fake_file * fp = (fake_file*)datasource; 
 return (long)fp->loc;
}
int cant_seek (void * datasource, ogg_int64_t offset, int whence) {
  return -1;
}
int mem_seek (void * datasource, ogg_int64_t offset, int whence) {
 fake_file * fp = (fake_file*)datasource; 
 if (whence==SEEK_END) {
   if (offset<0) {
     if (fp->size<(size_t)-offset) {
       return -1;
     }else {
       fp->loc = fp->size+offset;
       return 0;
     }
   }else if (offset==0) {
     fp->loc=fp->size;
   }else return -1;
 }else if (whence==SEEK_CUR) {
   if (offset<0) {
     if (fp->loc<(size_t)-offset)
       return -1;
     else {
       fp->loc+=offset;
       return 0;
     }
   }else {
     if (fp->loc+offset>fp->size) {
       return -1;
     }else {
       fp->loc+=offset;
       return 0;
     }
   }
 }else if (whence==SEEK_SET) {
   if (offset>fp->size) 
     return -1;
   fp->loc = offset;
   return 0;
 }
 return -1;
}
#endif
static void ConvertFormat (vector<char>& ogg ) {
  vector<char> converted;

  if (ogg.size()>4) {
    if (ogg[0]=='O'&&ogg[1]=='g'&&ogg[2]=='g'&&ogg[3]=='S') {
#ifdef HAVE_OGG
      OggVorbis_File vf;
      ov_callbacks callbacks;
      vorbis_info *info;       
      int bitstream = -1;
      long samplesize;
      long samples;
      int read, to_read;
      int must_close = 1;
      int was_error = 1;
      //FILE * tmpf=tmpfile();
      //fwrite(&ogg[0],ogg.size(),1,tmpf);
      //fseek(tmpf,0,SEEK_SET);
      fake_file ff;
      ff.data=&ogg[0];
      ff.loc=0;
      ff.size=ogg.size();
      callbacks.read_func=&mem_read;
      callbacks.seek_func=&mem_seek;
      callbacks.close_func=&mem_close;
      callbacks.tell_func=&mem_tell;
      if (ov_open_callbacks(&ff,&vf,NULL,0,callbacks) ){
        ogg.clear();
      }else {        
        int bigendian=0;
        long bytesread=0;
        vorbis_info *info=ov_info(&vf,-1);
        //ogg_int64_t totalsize=ov_pcm_total(&vf,-1);
        //long num_streams=ov_streams(&vf);
        const int segmentsize=16384;
        const int samples=16;
        converted.push_back('R');
        converted.push_back('I');
        converted.push_back('F');
        converted.push_back('F');
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);//fill in with weight;
        converted.push_back('W');
        converted.push_back('A');
        converted.push_back('V');
        converted.push_back('E');
        converted.push_back('f');
        converted.push_back('m');
        converted.push_back('t');
        converted.push_back(' ');

        converted.push_back(18);//size of header (16 bytes)
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);
        
        converted.push_back(1);//compression code
        converted.push_back(0);

        converted.push_back((char)(info->channels%256));//num channels;
        converted.push_back((char)(info->channels/256));
        
        converted.push_back(0);//sample rate
        converted.push_back(0);//sample rate
        converted.push_back(0);//sample rate
        converted.push_back(0);//sample rate
        convertToLittle(info->rate,&converted[converted.size()-4]);

        long byterate = info->rate*info->channels*samples/8;
        converted.push_back(0);//bytes per second rate
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);
        convertToLittle(byterate,&converted[converted.size()-4]);

        converted.push_back((char)((info->channels*samples/8)%256));//num_channels*16 bits/8
        converted.push_back((char)((info->channels*samples/8)/256));        
        
        converted.push_back(samples);// 16 bit samples
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);


        // PCM header
        converted.push_back('d');
        converted.push_back('a');
        converted.push_back('t');
        converted.push_back('a');


        converted.push_back(0);        
        converted.push_back(0);
        converted.push_back(0);
        converted.push_back(0);
        ogg_int64_t pcmsizestart=converted.size();
        converted.resize(converted.size()+segmentsize);
        int signedvalue=1;
        int bitstream=0;
        while ((bytesread=ov_read(&vf,&converted[converted.size()-segmentsize], segmentsize, 0, samples/8, signedvalue, &bitstream))>0){
          if (bytesread<segmentsize) {
            int numtoerase=segmentsize-bytesread;
            converted.erase(converted.end()-numtoerase,converted.end());
          }
          
          
          converted.resize(converted.size()+segmentsize);
        }
        converted.erase(converted.end()-segmentsize,converted.end());
        convertToLittle(converted.size()-8,&converted[4]);
        convertToLittle(converted.size()-pcmsizestart,&converted[pcmsizestart-4]);
#if 0
		FILE * tmp = fopen("c:/temp/bleh","wb");
        fwrite(&converted[0],converted.size(),1,tmp);
        fclose(tmp);
#endif
        converted.swap(ogg);
      }
      ov_clear(&vf);
#else
      ogg.clear();
#endif
    }
  }
}
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
#ifdef SOUND_DEBUG
  printf (" with buffer %d and looping property %d\n",i,(int)looping);
#endif
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
#ifdef SOUND_DEBUG
	printf ("AUDCreateSoundWAV:: ");
#endif
  if ((g_game.sound_enabled&&!music)||(g_game.music_enabled&&music)) {
	    ALuint * wavbuf =NULL;
	    std::string hashname;
	    if (!music)
		{
	      hashname = VSFileSystem::GetHashName (s);
		  wavbuf = soundHash.Get(hashname);
		  if (!wavbuf) {
		      hashname = VSFileSystem::GetSharedSoundHashName(s);
		      wavbuf = soundHash.Get(hashname);
		  }
	    }
		if (wavbuf) {
#ifdef SOUND_DEBUG
		  printf ("Sound %s restored with alBuffer %d\n",s.c_str(),*wavbuf);
#endif
		}
	    if (wavbuf==NULL)
		{
	  	  VSFile f;
	  	  VSError error = f.OpenReadOnly( s.c_str(), SoundFile);
		  bool shared=(error==Shared);
		  if (shared)
			  hashname = VSFileSystem::GetSharedSoundHashName(s);
		  else
		      hashname = VSFileSystem::GetHashName (s);
		  if (error>Ok)
			  return -1;
	      wavbuf = (ALuint *) malloc (sizeof (ALuint));
	      alGenBuffers (1,wavbuf);
#ifdef SOUND_DEBUG
		  printf ("Sound %s created with and alBuffer %d\n",s.c_str(),*wavbuf);
#endif
	      ALsizei size;	
	      ALsizei freq;
	      void *wave;
		  ALboolean looping;
	      ALboolean err=AL_TRUE;
              vector <char> dat;
              dat.resize(f.Size());
              f.Read( &dat[0], f.Size());
              ConvertFormat(dat);
              if (dat.size()==0)//conversion messed up
                return -1;
#ifndef WIN32
#ifdef __APPLE__
		  ALint format;
		  // MAC OS X
		  err = false;
		  if( error<=Ok)
			err=MacFixedLoadWAVFile( &dat[0], &format, &wave, &size, &freq);
#else
		  // LINUX
		  ALsizei format;
                  alutLoadWAVMemory((ALbyte *)&dat[0], &format, &wave, &size, &freq, &looping);
#endif
#else
		  ALint format;
	  	  // WIN32
                  alutLoadWAVMemory(&dat[0], (int*)&format, &wave, &size, &freq, &looping);
#endif

      	  if(err == AL_FALSE)
		  {
			alDeleteBuffers (1,wavbuf);
			free (wavbuf);
			return -1;
      	  }
      	  alBufferData( *wavbuf, format, wave, size, freq );
          alutUnloadWAV(format,wave,size,freq);
      	  if (!music)
		  {
			soundHash.Put (hashname,wavbuf);
			buffers.push_back (*wavbuf);
      	  }
		  f.Close();
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
  if (AUDIsPlaying (sound))
    AUDStopPlaying (sound);
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
#ifdef SOUND_DEBUG
      printf("AUDDeleteSound: Sound Playing enqueue soundstodelete %d %d\n",sounds[sound].source,sounds[sound].buffer);
#endif
	soundstodelete.push_back(sound);
	return;
      } else
	AUDStopPlaying (sound);
    }
#ifdef SOUND_DEBUG
	printf("AUDDeleteSound: Sound Not Playing push back to unused src %d %d\n",sounds[sound].source,sounds[sound].buffer);
#endif

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
      VSFileSystem::vs_fprintf (stderr,"double delete of sound %d",sound);
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
	if (usepositional&&sounds[sound].source) {
	    alSourcefv(sounds[sound].source,AL_POSITION,p);
            bool relative=(p[0]==0&&p[1]==0&&p[2]==0);
            alSourcei(sounds[sound].source,AL_SOURCE_RELATIVE,relative);
        }
  if (usedoppler&&sounds[sound].source)
    alSourcefv(sounds[sound].source,AL_VELOCITY,v);
  }
#endif
}
void AUDSoundGain (int sound, float gain) {
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
#ifdef SOUND_DEBUG
      printf("AUDStopPlaying sound %d source(releasing): %d buffer:%d\n",sound,sounds[sound].source,sounds[sound].buffer);
#endif	  
	if (sounds[sound].source!=0) {
	  alSourceStop(sounds[sound].source);
      unusedsrcs.push_back (sounds[sound].source);
	}
    sounds[sound].source=(ALuint)0;
  }
#endif
}
static bool AUDReclaimSource (const int sound, bool high_priority=false) {
#ifdef HAVE_AL
  if (sounds[sound].source==(ALuint)0) {
    if (!sounds[sound].buffer)
      return false;
    if (unusedsrcs.empty()) {
      if (high_priority) {
        unsigned int i;
        unsigned int candidate=0;
        bool found=false;
        for (i=0;i<sounds.size();++i) {
          if (sounds[i].source!=0) {
            if (sounds[i].pos.i!=0||sounds[i].pos.j!=0||sounds[i].pos.k!=0) {
              if (found) {
                if (AUDDistanceSquared(candidate)<AUDDistanceSquared(i)) {
                  candidate=i;
                }
              }else {
                candidate=i;
              }
              found=true;             
            }
          }
        }
        if (!found){
          return false;
        } else {
          alSourceStop(sounds[candidate].source);
          sounds[sound].source=sounds[candidate].source;
          sounds[candidate].source=0;
        }
      }else {        
        return false;
      }
    }else {
      sounds[sound].source = unusedsrcs.back();
      unusedsrcs.pop_back();
    }
    alSourcei(sounds[sound].source, AL_BUFFER, sounds[sound].buffer );
    alSourcei(sounds[sound].source, AL_LOOPING, sounds[sound].looping);    
  }
  return true;
#endif		
  return false;//silly
}
void AUDStartPlaying (const int sound){
#ifdef SOUND_DEBUG
	printf ("AUDStartPlaying(%d)",sound);
#endif
	
#ifdef HAVE_AL
  if (sound>=0&&sound<(int)sounds.size()) {
	  if (starSystemOK())
    if (AUDReclaimSource (sound,sounds[sound].pos==QVector(0,0,0))) {
#ifdef SOUND_DEBUG
      printf("AUDStartPlaying sound %d source:%d buffer:%d\n",sound,sounds[sound].source,sounds[sound].buffer);
#endif
      AUDAdjustSound (sound, sounds[sound].pos, sounds[sound].vel);

      alSourcePlay( sounds[sound].source );
    }
  }
#endif
}

void AUDPlay (const int sound, const QVector &pos, const Vector & vel, const float gain) {
#ifdef HAVE_AL
#ifdef SOUND_DEBUG
	printf ("AUDPlay(%d)",sound);
#endif
  char tmp;
  if (sound<0)
    return;
  if (sounds[sound].buffer==0) {
	return;
  }
  if (!starSystemOK())
	  return;
  if ((tmp=AUDQueryAudability (sound,pos.Cast(),vel,gain))!=0) {
    if (AUDReclaimSource (sound,pos==QVector(0,0,0))) {
      //ALfloat p [3] = {pos.i,pos.j,pos.k};
      AUDAdjustSound (sound,pos,vel);
      alSourcef(sounds[sound].source,AL_GAIN,gain);    
      if (tmp!=2){
#ifdef SOUND_DEBUG
        printf("AUDPlay sound %d %d\n",sounds[sound].source,sounds[sound].buffer);
#endif
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
