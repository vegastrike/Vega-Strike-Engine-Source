/*
 * al_listen.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifdef HAVE_AL
#if defined(__APPLE__) && defined(__MACH__)
#include <al.h>
#else
#include <AL/al.h>
#endif
#endif
#include <stdio.h>
#include <vector>
#include "audiolib.h"
#include "al_globals.h"
#include "vs_globals.h"
#include "vs_logging.h"
#include "config_xml.h"
#include "options.h"

using std::vector;

struct Listener {
    Vector pos;
    Vector vel;
    Vector p, q, r;
    float gain;
    float rsize;

    Listener() : pos( 0, 0, 0 )
            ,  vel( 0, 0, 0 )
            ,  p( 1, 0, 0 )
            ,  q( 0, 1, 0 )
            ,  r( 0, 0, 1 )
            ,  gain( 1 )
            ,  rsize( 1 ) {}
}
        mylistener;

unsigned int totalplaying = 0;
const unsigned int hashsize = 47;

struct ApproxSound {
    int soundname;
};

typedef std::vector<ApproxSound> ApproxSoundVec;
static ApproxSoundVec playingbuffers[hashsize];

unsigned int hash_sound(unsigned int buffer) {
    return buffer % hashsize;
}

float AUDDistanceSquared(const int sound) {
#ifdef HAVE_AL
    return (sounds[sound].pos - mylistener.pos).MagnitudeSquared();

#else
    return 0.0;
#endif
}

QVector AUDListenerLocation() {
    return mylistener.pos.Cast();
}

static float EstimateGain(const Vector &pos, const float gain) {
    // Base priority is source gain
    float final_gain = gain;

    // Account for distance attenuation
    float listener_size = sqrt(mylistener.rsize);
    float distance = (AUDListenerLocation() - pos.Cast()).Magnitude()
            - listener_size
            - vs_options::instance().audio_ref_distance;
    float ref = vs_options::instance().audio_ref_distance;
    float rolloff = 1.0f;
    final_gain *= (distance <= 0) ? 1.f : float(ref / (ref + rolloff * distance));

    return final_gain;
}

char AUDQueryAudability(const int sound, const Vector &pos, const Vector &vel, const float gain) {
#ifdef HAVE_AL
    if (sounds[sound].buffer == (ALuint) 0) {
        return 0;
    }
    sounds[sound].pos = pos;
    sounds[sound].vel = vel;
    Vector t = pos - mylistener.pos;
    float mag = t.Dot(t);
    if (pos == Vector(0, 0, 0)) {
        t = Vector(0, 0, 0);
        mag = 0;
        return 1;
    }
    if (!(mag < vs_options::instance().audio_max_distance * vs_options::instance().audio_max_distance)) {
        return 0;
    }
    unsigned int hashed = hash_sound(sounds[sound].buffer);
    if ((!unusedsrcs.empty()) && playingbuffers[hashed].size() < maxallowedsingle) {
        return 1;
    }
    ///could theoretically "steal" buffer from playing sound at this point
    if (playingbuffers[hashed].empty()) {
        return 1;
    }
    //int target = rand()%playingbuffers[hashed].size();
    float est_gain = EstimateGain(pos, gain);
    float min_gain = est_gain;
    int min_index = -1;
    for (size_t target = 0; target < playingbuffers[hashed].size(); ++target) {
        int target1 = playingbuffers[hashed][target].soundname;
        t = sounds[target1].pos - mylistener.pos;
        if (sounds[target1].pos == Vector(0, 0, 0)) {
            t = Vector(0, 0, 0);
        }
        //steal sound!
        if (sounds[target1].buffer == sounds[sound].buffer) {
            float target_est_gain;
            if (sounds[target1].pos == Vector(0, 0, 0)) {
                // relative sound, constant gain
                target_est_gain = sounds[target1].gain;
            } else {
                // positional sound
                target_est_gain = EstimateGain(sounds[target1].pos, sounds[target1].gain);
            }
            if (target_est_gain <= min_gain) {
                min_index = target;
                min_gain = target_est_gain;
            }
        }
    }
    if (min_index >= 0) {
        int target = min_index;
        int target1 = playingbuffers[hashed][target].soundname;

        ALuint tmpsrc = sounds[target1].source;

        sounds[target1].source = sounds[sound].source;
        sounds[sound].source = tmpsrc;
        playingbuffers[hashed][target].soundname = sound;
        if (tmpsrc == 0) {
            playingbuffers[hashed].erase(playingbuffers[hashed].begin() + target);
        } else {
            VS_LOG(trace, (boost::format("stole %1%\n") % tmpsrc));
            return 2;
        }
    }

    if (playingbuffers[hashed].size() > maxallowedsingle) {
        return 0;
    }
    if (totalplaying > maxallowedtotal) {
        return 0;
    }
#endif
    return 1;
}

void AUDAddWatchedPlayed(const int sound, const Vector &pos) {
#ifdef HAVE_AL
    totalplaying++;
    if (sounds[sound].buffer != (ALuint) 0) {
        unsigned int h = hash_sound(sounds[sound].buffer);
        if (sounds[sound].source == 0) {
            VS_LOG(warning, "adding null sound");
        }
        playingbuffers[h].push_back(ApproxSound());
        playingbuffers[h].back().soundname = sound;
    }
#endif
}

typedef std::vector<int> vecint;
vecint soundstodelete;

void AUDRefreshSounds() {
#ifdef HAVE_AL
    static unsigned int i = 0;
    if (i >= hashsize) {
        i = 0;
    } else {
        for (unsigned int j = 0; j < playingbuffers[i].size(); j++) {
            if (!AUDIsPlaying(playingbuffers[i][j].soundname)) {
                totalplaying--;
                if (sounds[playingbuffers[i][j].soundname].source != (ALuint) 0) {
                    unusedsrcs.push_back(sounds[playingbuffers[i][j].soundname].source);
                    alSourcei(sounds[playingbuffers[i][j].soundname].source, AL_BUFFER, 0);
                    sounds[playingbuffers[i][j].soundname].source = (ALuint) 0;
                }
                ApproxSoundVec::iterator k = playingbuffers[i].begin();
                k += j;
                playingbuffers[i].erase(k);
                j--;
            }
        }
        ++i;
    }
    static unsigned int j = 0;
    if (j >= soundstodelete.size()) {
        j = 0;
    } else {
        int tmp = soundstodelete[j];
        if (!AUDIsPlaying(tmp)) {
            soundstodelete.erase(soundstodelete.begin() + j);
            AUDDeleteSound(tmp, false);
        }
        ++j;
    }
#endif
}

void AUDListener(const QVector &pos, const Vector &vel) {
#ifdef HAVE_AL
    mylistener.pos = pos.Cast();
    mylistener.vel = vel;
    if (g_game.sound_enabled) {
        if (usepositional) {
            alListener3f(AL_POSITION, scalepos * pos.i, scalepos * pos.j, scalepos * pos.k);
        }
        if (usedoppler) {
            alListener3f(AL_VELOCITY, scalevel * vel.i, scalevel * vel.j, scalevel * vel.k);
        }
    }
#endif
}

void AUDListenerSize(const float rSize) {
#ifdef HAVE_AL
    mylistener.rsize = rSize * rSize;
#endif
}

void AUDListenerOrientation(const Vector &p, const Vector &q, const Vector &r) {
#ifdef HAVE_AL
    mylistener.p = p;
    mylistener.q = q;
    mylistener.r = r;
    ALfloat orient[] = {r.i, r.j, r.k, q.i, q.j, q.k};
    if (g_game.sound_enabled) {
        alListenerfv(AL_ORIENTATION, orient);
    }
#endif
}

void AUDSoundGain(int sound, float gain, bool music) {
#ifdef HAVE_AL
    if (sound >= 0 && sound < (int) sounds.size()) {
        sounds[sound].music = music;
        float val = gain * (music ? 1.0f : mylistener.gain);
        if (sounds[sound].source) {
            alSourcef(sounds[sound].source, AL_GAIN, val <= 1. / 16384 ? 0 : val);
        }
        sounds[sound].gain = gain;
        //alSourcefv(sounds[sound].source,AL_VELOCITY,v);
    }
#endif
}

void AUDListenerGain(const float ggain) {
#ifdef HAVE_AL
    float gain = ggain;
    if (gain <= 0) {
        gain = 1. / 16384;
    }
    mylistener.gain = gain;
    for (unsigned int i = 0, ie = sounds.size(); i < ie; ++i) {
        if (!sounds[i].music) {
            AUDSoundGain(i, sounds[i].gain, false);
        }
    }
    if (g_game.sound_enabled) {
        alListenerf(AL_GAIN, 1.0);
    }
#endif
}

float AUDGetListenerGain() {
#ifdef HAVE_AL
    return mylistener.gain;

#else
    return 0;
#endif
}
