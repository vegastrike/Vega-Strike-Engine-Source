/**
 * OpenALSimpleSound.h
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
// C++ Interface: Audio::OpenALSimpleSound
//
#ifndef __AUDIO_OPENALSIMPLESOUND_H__INCLUDED__
#define __AUDIO_OPENALSIMPLESOUND_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Types.h"
#include "../../Format.h"
#include "../../SimpleSound.h"
#include "../../SoundBuffer.h"

#include "al.h"

namespace Audio {

/**
 * OpenAL Simple Sound implementation class
 *
 * @remarks This class implements simple (non-streaming) OpenAL sounds.
 *      This will load the whole sound into a single OpenAL buffer.
 * @see Sound, SimpleSound
 *
 */
class OpenALSimpleSound : public SimpleSound {
    ALBufferHandle bufferHandle;

public:
    /** Internal constructor used by derived classes */
    OpenALSimpleSound(const std::string &name, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile);

    /** Package-private: the OpenAL renderer package uses this, YOU DON'T */
    ALBufferHandle getAlBuffer() const
    {
        return bufferHandle;
    }

public:
    virtual ~OpenALSimpleSound();

    // The following section contains supporting methods for accessing the stream.
    // Subclasses need not bother with actual stream management, they need only worry
    // about sending the samples to where they're needed.
protected:
    /** @copydoc Sound::loadImpl */
    virtual void loadImpl(bool wait);

    /** @copydoc Sound::unloadImpl */
    virtual void unloadImpl();
};

};

#endif//__AUDIO_SIMPLESOUND_H__INCLUDED__
