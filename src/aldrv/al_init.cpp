#include "audiolib.h"
#ifdef HAVE_AL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <stdlib.h>
#include <stdio.h>
#include "al_globals.h"
#include <vector>
static void fixup_function_pointers(void) {
  alutLoadMP3p = (mp3Loader *) alGetProcAddress((ALubyte *)"alutLoadMP3_LOKI");
  if(alutLoadMP3p == NULL) {
    fprintf(stderr, "Could not GetProc %s\n","alutLoadMP3_LOKI");
    exit(-4);
  }


  /* UNCOMMENT AS NEEDED
	talcGetAudioChannel = (ALfloat (*)(ALuint channel))
				GP("alcGetAudioChannel_LOKI");
	talcSetAudioChannel = (void (*)(ALuint channel, ALfloat volume))
				GP("alcSetAudioChannel_LOKI");

	talMute   = (void (*)(ALvoid)) GP("alMute_LOKI");
	talUnMute = (void (*)(ALvoid)) GP("alUnMute_LOKI");

	talReverbScale = (void (*)(ALuint sid, ALfloat param))
		GP("alReverbScale_LOKI");
	talReverbDelay = (void (*)(ALuint sid, ALfloat param))
		GP("alReverbDelay_LOKI");

	talBombOnError = (void (*)(void))
		GP("alBombOnError_LOKI");

	if(talBombOnError == NULL) {
		fprintf(stderr,
			"Could not GetProcAddress alBombOnError_LOKI\n");
		exit(1);
	}
	
	talBufferi = (void (*)(ALuint, ALenum, ALint ))
		GP("alBufferi_LOKI");

	if(talBufferi == NULL) {
		fprintf(stderr,
			"Could not GetProcAddress alBufferi_LOKI\n");
		exit(1);
	}

	alCaptureInit    = (ALboolean (*)( ALenum, ALuint, ALsizei )) GP("alCaptureInit_EXT");
	alCaptureDestroy = (ALboolean (*)( ALvoid )) GP("alCaptureDestroy_EXT");
	alCaptureStart   = (ALboolean (*)( ALvoid )) GP("alCaptureStart_EXT");
	alCaptureStop    = (ALboolean (*)( ALvoid )) GP("alCaptureStop_EXT");
	alCaptureGetData = (ALsizei (*)( ALvoid*, ALsizei, ALenum, ALuint )) GP("alCaptureGetData_EXT");

	talBufferAppendData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint)) GP("alBufferAppendData_LOKI");
	talBufferAppendWriteData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint, ALenum)) GP("alBufferAppendWriteData_LOKI");

	talGenStreamingBuffers = (void (*)(ALsizei n, ALuint *bids )) GP("alGenStreamingBuffers_LOKI");
	if( talGenStreamingBuffers == NULL ) {
		fprintf( stderr, "Could not GP alGenStreamingBuffers_LOKI\n");
		exit(1);
	}
	
	talutLoadRAW_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadRAW_ADPCMData_LOKI");
	if( talutLoadRAW_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadRAW_ADPCMData_LOKI\n");
		exit(1);
	}

	talutLoadIMA_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadIMA_ADPCMData_LOKI");
	if( talutLoadIMA_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadIMA_ADPCMData_LOKI\n");
		exit(1);
	}

	talutLoadMS_ADPCMData = (ALboolean (*)(ALuint bid,ALvoid *data, ALuint size, ALuint freq,ALenum format)) GP("alutLoadMS_ADPCMData_LOKI");
	if( talutLoadMS_ADPCMData == NULL ) {
		fprintf( stderr, "Could not GP alutLoadMS_ADPCMData_LOKI\n");
		exit(1);
	}

  */
	return;
}
#endif
///I don't think we'll need to switch contexts or devices in vegastrike
static ALCdevice *dev=NULL;
static void *context_id=NULL;

bool AUDInit () {
#ifdef HAVE_AL
	int attrlist[] = { ALC_FREQUENCY, 22050, 0 };
	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return false;
	}

	/* Initialize ALUT. */
	context_id = alcCreateContext( dev, attrlist );
	if(context_id == NULL) {
		alcCloseDevice( dev );
		return false;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();
	return true;
#endif
	return false;
}


void AUDDestroy() {
#ifdef HAVE_AL
  //Go through and delete all loaded wavs
  for (unsigned int i=0;i<buffers.size();i++) {
    alDeleteBuffers (1,&buffers[i]);
  }
  buffers.clear();
  if (context_id)
    alcDestroyContext(context_id);
  if (dev)
    alcCloseDevice( dev );
#endif
}
