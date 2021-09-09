/**
* audiolib.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
#include <string>
bool AUDInit();
void AUDDestroy();
///Sets the size in which all sounds are going to be played
void AUDListenerSize( const float size );
void AUDListener( const QVector &pos, const Vector &vel );
QVector AUDListenerLocation();
///Checks if sounds are still playing
void AUDRefreshSounds();
///Will the sound be played
void AUDListenerOrientation( const Vector &i, const Vector &j, const Vector &k );
void AUDListenerGain( const float gain );
float AUDGetListenerGain();
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateSoundWAV( const std::string&, const bool LOOP = false );
///creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int AUDCreateSoundMP3( const std::string&, const bool LOOP = false );
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateMusicWAV( const std::string&, const bool LOOP = false );
///creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int AUDCreateMusicMP3( const std::string&, const bool LOOP = false );
///copies other sound loaded through AUDCreateSound
int AUDCreateSound( int sound, const bool LOOP = false );
///guesses the type of sound by extension
int AUDCreateSound( const std::string&, const bool LOOP = false );
///guesses the type of sound by extension
int AUDCreateMusic( const std::string&, const bool LOOP = false );
void AUDStopAllSounds( int except_this_one = -1 );
int AUDHighestSoundPlaying();
///deletes a given sound
void AUDDeleteSound( int sound, bool music = false );
///Changes the velocity and/or position of a given sound
void AUDAdjustSound( const int sound, const QVector &pos, const Vector &vel );
///Setup the sound as a streaming source (this means right now only that it doesn't do 3D positional stuff)
void AUDStreamingSound( const int sound );
///Changes the gain of a loaded sound
void AUDSoundGain( int sound, float gain, bool music = false );
///Is a loaded sound still playing
bool AUDIsPlaying( const int sound );
///Stops a loaded sound
void AUDStopPlaying( const int sound );
///Plays a loaded sound
void AUDStartPlaying( const int sound );
///Queries if the sound should be culled. If not, plays
void AUDPlay( const int sound, const QVector &pos, const Vector &vel, const float gain );
///Pauses a loaded sound
void AUDPausePlaying( const int sound );
///Changes the volume (generally 0 or between 1 and 1000)
void AUDChangeVolume( float volume );
float AUDGetVolume();
///changes the scale used for doppler...generally between 0 for off or .01 and 10
void AUDChangeDoppler( float doppler );
///Gets the current playback position in seconds (precision is system&driver-dependant)
float AUDGetCurrentPosition( const int sound );
///Gets current doppler val
float AUDGetDoppler();
//#define PERFRAMESOUND
#endif

