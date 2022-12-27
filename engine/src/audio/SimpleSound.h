/*
 * SimpleSound.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
// C++ Interface: Audio::SimpleSound
//
#ifndef __AUDIO_SIMPLESOUND_H__INCLUDED__
#define __AUDIO_SIMPLESOUND_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"
#include "Sound.h"
#include "SoundBuffer.h"

#include "vsfilesystem.h"

namespace Audio {

// Forward definitions
class Stream;

/**
 * Simple Sound abstract class
 *
 * @remarks This partial implementation implements foreground loading of files
 *      using the codec registry.
 *      @par No background loading is implemented, meaning all requests for load,
 *      even with wait=false, are processed in the foreground.
 *      @par There's a possibility for streaming given the packetized pulling
 *      architecture. Renderers are not required to pull all packets from the stream,
 *      and access to the Stream object is also provided for seeking back and forth.
 *      @par Renderers still have to override (un)loadImpl() and abortLoad().
 *      This refinement merely adds supporting methods for implementing them.
 * @see Sound, BackgroundLoadingSound
 *
 */
class SimpleSound : public Sound {
private:
    vega_types::SharedPtr<Stream> stream;
    VSFileSystem::VSFileType type;

protected:
    /** Internal constructor used by derived classes */
    SimpleSound(const std::string &name,
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
            bool streaming = false);

public:
    virtual ~SimpleSound();

    /** VSFileSystem File type */
    VSFileSystem::VSFileType getType() const {
        return type;
    }

    // The following section contains supporting methods for accessing the stream.
    // Subclasses need not bother with actual stream management, they need only worry
    // about sending the samples to where they're needed.
protected:

    /** Do we have an open stream? */
    bool isStreamLoaded() const {
        return stream.get() != 0;
    }

    /** Initialize the stream.
     * @remarks Calling this when the stream has already been initialized will
     *      raise an ReasourceAlreadyLoadedException.
     */
    void loadStream();

    /** Uninitialize the stream
     * @remarks Calling this when isStreamLoaded() returns false will raise an
     *      ResourceNotLoadedException.
     */
    void closeStream();

    /** Get a pointer to the stream
     * @remarks Calling this when isStreamLoaded() returns false will raise an
     *      ResourceNotLoadedException.
     */
    vega_types::SharedPtr<Stream> getStream() const;

    /** Read from the stream into the buffer
     * @remarks Will throw EndOfStreamException when the end of the stream
     *      is reached. Any other exception is probably fatal.
     */
    void readBuffer(SoundBuffer &buffer);

    // The following section contains basic Sound interface implementation
    // functions provided by SimpleSound.
protected:
    /** @copydoc Sound::abortLoad */
    virtual void abortLoad();

};

};

#endif//__AUDIO_SIMPLESOUND_H__INCLUDED__
