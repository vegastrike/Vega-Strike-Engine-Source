#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
bool AUDInit();
void AUDDestroy();
void AUDListener (const Vector &pos, const Vector &vel);
void AUDListenerOrientation (const Vector & i, const Vector &j, const Vector &k);
void AUDListenerGain (const float gain);
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateSound (const char *, const bool LOOP=false);
///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP=false);
int AUDDeleteSound (int sound);
void AUDAdjustSound (const Vector &pos, const Vector &vel);
void AUDPlayOnce(int sound, const Vector &pos, const Vector &vel);
void AUDPlayOnce (const char *sound, const Vector &pos, const Vector &vel);
void AUDSoundGain (const int sound, const float gain);
bool AUDIsPlaying (const int sound);
void AUDStopPlaying (const int sound);
void AUDStartPlaying (const int sound);
void AUDPausePlaying (const int sound);
#endif
