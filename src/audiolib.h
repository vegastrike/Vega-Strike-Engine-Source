#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
#include <string>
bool AUDInit();
void AUDDestroy();
void AUDListener (const Vector &pos, const Vector &vel);
void AUDListenerOrientation (const Vector & i, const Vector &j, const Vector &k);
void AUDListenerGain (const float gain);
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateSoundWAV (const std::string &, const bool LOOP=false);
///creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int AUDCreateSoundMP3 (const std::string &, const bool LOOP=false);
///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP=false);
///deletes a given sound
int AUDDeleteSound (int sound);
///Changes the velocity and/or position of a given sound
void AUDAdjustSound (const int sound, const Vector &pos, const Vector &vel);
///Changes the gain of a loaded sound
void AUDSoundGain (const int sound, const float gain);
///Is a loaded sound still playing
bool AUDIsPlaying (const int sound);
///Stops a loaded sound
void AUDStopPlaying (const int sound);
///Plays a loaded sound
void AUDStartPlaying (const int sound);
///Pauses a loaded sound
void AUDPausePlaying (const int sound);
#endif
