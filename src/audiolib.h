#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
bool AUDInit();
void AUDDestroy();
void AUDListener (const Vector &pos, const Vector &vel);
void AUDListenerOrientation (const Vector & i, const Vector &j, const Vector &k);
void AUDListenerGain (const float gain);
int AUDCreateSound (const char *, const bool LOOP);
void AUDAdjustSound (const Vector &pos, const Vector &vel);
void AUDSoundGain (const int sound, const float gain);
bool AUDIsPlaying (const int sound);
void AUDStopPlaying (const int sound);
void AUDStartPlaying (const int sound);
void AUDPausePlaying (const int sound);
#endif
