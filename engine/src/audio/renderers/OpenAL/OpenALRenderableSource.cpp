/**
 * OpenALRenderableSource.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


//
// C++ Implementation: Audio::OpenALRenderableListener
//
#include "OpenALRenderableSource.h"
#include "OpenALSimpleSound.h"
#include "OpenALHelpers.h"

#include "al.h"

#include "../../Source.h"
#include "../../Listener.h"

#include "vs_math.h"

namespace Audio {

static inline void alSource3f(ALuint source, ALenum param, const Vector3 &v) {
    ::alSource3f(source, param, ALfloat(v.x), ALfloat(v.y), ALfloat(v.z));
}

static inline void alSource3f(ALuint source, ALenum param, const LVector3 &v) {
    ::alSource3f(source, param, ALfloat(v.x), ALfloat(v.y), ALfloat(v.z));
}

OpenALRenderableSource::OpenALRenderableSource(Source *source)
        : RenderableSource(source),
        alSource(0),
        alBuffersAttached(false) {
    alGenSources(1, &alSource);
}

OpenALRenderableSource::~OpenALRenderableSource() {
    alDeleteSources(1, &alSource);
}

void OpenALRenderableSource::startPlayingImpl(Timestamp start) {
    if (!isPlayingImpl()) {
        // Make sure we have an attached sound
        attachALBuffers();

        // Tell the AL to start playing (from the specified position)
        clearAlError();
        ALuint als = getALSource();
        alSourcePlay(als);
        checkAlError();

        if (start != 0) {
            seekImpl(start);
        }
    }
}

void OpenALRenderableSource::stopPlayingImpl() {
    alSourceStop(alSource);
}

bool OpenALRenderableSource::isPlayingImpl() const {
    ALint state = 0;
    alGetSourcei(getALSource(), AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

Timestamp OpenALRenderableSource::getPlayingTimeImpl() const {
    ALfloat offs = -1.f;
    alGetSourcef(getALSource(), AL_SEC_OFFSET, &offs);

    if (offs < 0.f) {
        throw NotImplementedException("getPlayingTimeImpl");
    }

    return Timestamp(offs);
}

void OpenALRenderableSource::updateImpl(int flags, const Listener &sceneListener) {
    Source *source = getSource();
    ALSourceHandle als = getALSource();

    clearAlError();

    if (flags & UPDATE_ATTRIBUTES) {
        // Distance attenuation
        if (source->isAttenuated()) {
            alSourcef(als, AL_REFERENCE_DISTANCE, source->getRadius());
            alSourcef(als, AL_ROLLOFF_FACTOR, 1.f);
        } else {
            alSourcef(als, AL_ROLLOFF_FACTOR, 0.f);
        }
        // Cone
        {
            Range<Scalar> angleRange = source->getAngleRange();
            alSourcef(als, AL_CONE_INNER_ANGLE, float(angleRange.min) * M_1_PI * 360.f);
            alSourcef(als, AL_CONE_OUTER_ANGLE, float(angleRange.max) * M_1_PI * 360.f);
            alSourcef(als, AL_CONE_OUTER_GAIN, 0.f);
        }
        // Relativity
        alSourcei(als, AL_SOURCE_RELATIVE, source->isRelative() ? AL_TRUE : AL_FALSE);
        // Looping
        alSourcei(als, AL_LOOPING, source->isLooping() ? AL_TRUE : AL_FALSE);
    }
    if (flags & UPDATE_GAIN) {
        // Gain
        alSourcef(als, AL_GAIN, source->getGain());
    }
    if (flags & UPDATE_LOCATION) {
        if (source->isRelative()) {
            alSource3f(als, AL_POSITION, source->getPosition());
            alSource3f(als, AL_VELOCITY, source->getVelocity());
            alSource3f(als, AL_DIRECTION, source->getDirection());
        } else {
            alSource3f(als, AL_POSITION,
                    source->getPosition() - sceneListener.getPosition());
            alSource3f(als, AL_VELOCITY,
                    sceneListener.toLocalDirection(
                            source->getVelocity() - sceneListener.getVelocity()
                    ));
            alSource3f(als, AL_DIRECTION,
                    sceneListener.toLocalDirection(
                            source->getDirection()
                    ));
        }
    }

    checkAlError();
}

void OpenALRenderableSource::attachALBuffers() {
    if (!alBuffersAttached) {
        SharedPtr<Sound> sound = getSource()->getSound();

        if (!sound->isLoaded()) {
            sound->load();
        }

        assert(!sound->isStreaming() && "OpenALRenderableSource can only handle streaming sounds");

        // Attachment to a simple sound, just assign the AL buffer to this AL source
        ALBufferHandle alBuffer = dynamic_cast<OpenALSimpleSound *>(sound.get())->getAlBuffer();
        ALSourceHandle alSource = getALSource();
        alSourcei(alSource, AL_BUFFER, alBuffer);
        alBuffersAttached = true;

        checkAlError();
    }
}

void OpenALRenderableSource::seekImpl(Timestamp time) {
    // Tell the AL to jump to the specified position
    // NOTE: lots of implementations don't support it
    //      but according to OpenAL 1.1 specs they should
    clearAlError();
    ALuint als = getALSource();
    alSourcef(als, AL_SEC_OFFSET, time);

    ALenum error = alGetError();
    if (error == ALC_INVALID_ENUM) {
        // This version of the AL does not support seeking
        // fail silently
        // TODO: must log the fact to console as a warning
    } else {
        checkAlErrorCode(error);
    }
}

};
