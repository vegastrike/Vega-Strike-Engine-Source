/*
 * Sound.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SOUND_H
#define VEGA_STRIKE_ENGINE_AUDIO_SOUND_H

//
// C++ Interface: Audio::Sound
//

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

namespace Audio {

// Forward declaration of Streams
class Stream;

/**
 * Sound abstract class
 *
 * @remarks This is the interface to all kinds of sound resources.
 * @see Stream, Codec, CodecRegistry, for data sources.
 *
 */
class Sound {
private:
    std::string name;
    Format format;

protected:
    /** @note Accessible to derived classes to support external unloading (ie: memory-short events) */
    struct Flags {
        /** Loaded state of the resource.
        */
        int loaded: 1;

        /** Background loading state of the resource.
        * @note Accessible to derived classes to support easier and safer threading
        */
        int loading: 1;

        /** Sound is a streaming resource
        * @note Accessible to derived classes to support easier and safer threading
         */
        int streaming: 1;
    } flags;

protected:
    /** Internal constructor used by derived classes */
    Sound(const std::string &name, bool streaming);

    /** Protected Write access to the sound's format, for implementations. */
    Format &getFormat() {
        return format;
    };

public:
    virtual ~Sound();

    /** Return the path of the associated file. */
    const std::string &getName() const {
        return name;
    };

    /** Return the format of the sound resource. */
    const Format &getFormat() const {
        return format;
    };

    /** Return whether the resource has been loaded or not */
    bool isLoaded() const {
        return flags.loaded;
    }

    /** Return whether the resource is being loaded in the background */
    bool isLoading() const {
        return flags.loading;
    }

    /** Return whether the resource is being loaded in the background */
    bool isStreaming() const {
        return flags.streaming;
    }

    /** Load the resource if not loaded
     * @param wait If true, the function will return only when the resource
     *      has been loaded (or failed to load). Exceptions will be risen on
     *      failure. If false, however, a request for background loading is
     *      issued which may or may not be performed asynchronously. In the
     *      event the implementation does provide background loading of resources,
     *      exceptions and error conditions will be masked because of the
     *      asynchronicity. Only subsequent calls to isLoading() / isLoaded
     *      will allow for such events to be recognized: if it ceases to be
     *      in loading state yet it isn't loaded, either it has been immediately
     *      unloaded (memory short), or an error ocurred during load and it never
     *      became loaded.
     */
    void load(bool wait = true);

    /** Unload the resource if loaded */
    void unload();

    // The following section contains all the virtual functions that need be implemented
    // by a concrete Sound class. All are protected, so the stream interface is independent
    // of implementations.
protected:

    /** loadImpl should call this upon process termination
     * @remarks Either from the foreground thread or the background thread,
     *      whichever is performing the actual load.
     *      The method guaranteed to be threadsafe.
     * @param success Whether or not the process succeeded in loading the resource
     */
    virtual void onLoaded(bool success);

    /** Wait for background loading to finish
     * @remarks The base implementation checks for completion periodically.
     *      Implementations are likely to have better ways to do this, so they're
     *      welcome to override this method.
     */
    virtual void waitLoad();

    /** Load the resource
     * @note Assume it is unloaded and not loading
     */
    virtual void loadImpl(bool wait) = 0;

    /** Abort an in-progress background load procedure
     * @note Although no exceptions should be risen, the abort request may
     *      not be carried out for various reasons. The caller should check
     *      that on return by calling isLoaded() / isLoading().
     */
    virtual void abortLoad() = 0;

    /** Unload the resource.
     * @note Assume it is loaded
     */
    virtual void unloadImpl() = 0;

};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SOUND_H
