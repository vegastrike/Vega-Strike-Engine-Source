/*
 * al_init.cpp
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Benjamen R. Meyer, and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifdef HAVE_AL
#if defined(__APPLE__) && defined(__MACH__)
#include <al.h>
#include <alc.h>
#define KeyMap AppleKeyMap
#undef KeyMap
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <stdlib.h>
#include <stdio.h>

#include <vector>

#include "audiolib.h"
#include "config_xml.h"
#include "xml_support.h"
#include "vs_globals.h"
#include "al_globals.h"
#include "options.h"
#include "vs_logging.h"

#if 1
#define AL_GET_PROC(name) ( alGetProcAddress( const_cast< ALubyte* > ( reinterpret_cast< const ALubyte* > (name) ) ) )
#else
#define AL_GET_PROC( name ) ( alGetProcAddress( name ) )
#endif

static void fixup_function_pointers(void) {
    /* UNCOMMENT AS NEEDED
     *     talcGetAudioChannel = (ALfloat (*)(ALuint channel))
     *                             GP("alcGetAudioChannel_LOKI");
     *     talcSetAudioChannel = (void (*)(ALuint channel, ALfloat volume))
     *                             GP("alcSetAudioChannel_LOKI");
     *
     *     talMute   = (void (*)(ALvoid)) GP("alMute_LOKI");
     *     talUnMute = (void (*)(ALvoid)) GP("alUnMute_LOKI");
     *
     *     talReverbScale = (void (*)(ALuint sid, ALfloat param))
     *             GP("alReverbScale_LOKI");
     *     talReverbDelay = (void (*)(ALuint sid, ALfloat param))
     *             GP("alReverbDelay_LOKI");
     *
     *     talBombOnError = (void (*)(void))
     *             GP("alBombOnError_LOKI");
     *
     *     if(talBombOnError == NULL) {
     *             VS_LOG_AND_FLUSH(fatal, "Could not GetProcAddress alBombOnError_LOKI");
     *             VSExit(1);
     *     }
     *
     *     talBufferi = (void (*)(ALuint, ALenum, ALint ))
     *             GP("alBufferi_LOKI");
     *
     *     if(talBufferi == NULL) {
     *             VS_LOG_AND_FLUSH(fatal, "Could not GetProcAddress alBufferi_LOKI");
     *             VSExit(1);
     *     }
     *
     *     alCaptureInit    = (ALboolean (*)( ALenum, ALuint, ALsizei )) GP("alCaptureInit_EXT");
     *     alCaptureDestroy = (ALboolean (*)( ALvoid )) GP("alCaptureDestroy_EXT");
     *     alCaptureStart   = (ALboolean (*)( ALvoid )) GP("alCaptureStart_EXT");
     *     alCaptureStop    = (ALboolean (*)( ALvoid )) GP("alCaptureStop_EXT");
     *     alCaptureGetData = (ALsizei (*)( ALvoid*, ALsizei, ALenum, ALuint )) GP("alCaptureGetData_EXT");
     *
     *     talBufferAppendData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint)) GP("alBufferAppendData_LOKI");
     *     talBufferAppendWriteData = (ALuint (*)(ALuint, ALenum, ALvoid *, ALint, ALint, ALenum)) GP("alBufferAppendWriteData_LOKI");
     *
     *     talGenStreamingBuffers = (void (*)(ALsizei n, ALuint *bids )) GP("alGenStreamingBuffers_LOKI");
     *     if( talGenStreamingBuffers == NULL ) {
     *             VS_LOG_AND_FLUSH(fatal, "Could not GP alGenStreamingBuffers_LOKI");
     *             VSExit(1);
     *     }
     *
     *
     *
     */
}

#endif

void AUDChangeVolume(float volume) {
#ifdef HAVE_AL
    if (volume == 0) {
        usepositional = false;
        return;
    } else {
        usepositional = true;
    }
    scalepos = 1. / volume;
#endif
}

float AUDGetVolume() {
#ifdef HAVE_AL
    return 1. / scalepos;

#else
    return 1;
#endif
}

void AUDChangeDoppler(float doppler) {
#ifdef HAVE_AL
    if (doppler <= 0) {
        usedoppler = false;
    } else {
        usedoppler = true;
    }
    scalevel = doppler;
#endif
}

float AUDGetDoppler() {
#ifdef HAVE_AL
    return scalevel;

#else
    return 1;
#endif
}

#ifdef HAVE_AL
///I don't think we'll need to switch contexts or devices in vegastrike
static ALCdevice *dev = NULL;

#ifndef _WIN32
static ALvoid *context_id = NULL;
#else
static ALCcontext *context_id = NULL;
#endif
#endif

bool AUDInit() {
#ifdef HAVE_AL
    // g_game.sound_enabled =
    usedoppler = game_options()->Doppler;
    usepositional = game_options()->Positional;
    double linuxadjust = 1;
#ifndef _WIN32
#ifndef __APPLE__
    linuxadjust = 1. / 3.;
#endif
#endif
    scalepos = 1.0f / (game_options()->Volume * linuxadjust);
    scalevel = game_options()->DopplerScale;
    g_game.audio_frequency_mode = game_options()->frequency;
    maxallowedsingle = game_options()->MaxSingleSounds;
    g_game.max_sound_sources =
            maxallowedtotal = game_options()->MaxTotalSounds;
    if (!game_options()->Sound && !game_options()->Music) {
        return false;
    }
    int attrlist[] = {ALC_FREQUENCY, g_game.audio_frequency_mode, 0};
#ifdef _WIN32
    dev = alcOpenDevice(  (ALCchar*)"DirectSound3D" );
#elif __APPLE__
    dev = alcOpenDevice( (ALCchar*)"sdl" );
#endif
    if (dev == NULL) {
        // use default device
        dev = alcOpenDevice(NULL);
    }
    if (dev == NULL) {
        return false;
    }
    context_id = alcCreateContext(dev, attrlist);
    if (context_id == NULL) {
        alcCloseDevice(dev);
        return false;
    }
    alcMakeContextCurrent((ALCcontext *) context_id);

    fixup_function_pointers();
    ALenum alGetEr = 0;
    ALuint cursrc;
    alGetError();
    alGenSources(1, &cursrc);
    alGetEr = alGetError();
    while (alGetEr == 0) {
        unusedsrcs.push_back(cursrc);
        if (unusedsrcs.size() >= maxallowedtotal) {
            break;
        }
        alGenSources(1, &cursrc);
        alGetEr = alGetError();
    }

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    g_game.sound_enabled = game_options()->Sound;

    return true;
#endif
    return false;
}

void AUDDestroy() {
#ifdef HAVE_AL
    //Go through and delete all loaded wavs
    unsigned int i;
    for (i = 0; i < sounds.size(); i++) {
        if (sounds[i].buffer != 0) {
            AUDStopPlaying(i);
        }
        AUDDeleteSound(i);
    }
    for (i = 0; i < unusedsrcs.size(); i++) {
        alDeleteSources(1, &unusedsrcs[i]);
    }
    for (i = 0; i < buffers.size(); i++) {
        alDeleteBuffers(1, &buffers[i]);
    }
    buffers.clear();
    if (context_id) {
        alcDestroyContext((ALCcontext *) context_id);
    }
    if (dev) {
        alcCloseDevice(dev);
    }
#endif
}

