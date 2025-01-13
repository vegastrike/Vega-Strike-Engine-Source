/*
 * RenderableSource.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERABLESOURCE_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERABLESOURCE_H

//
// C++ Interface: Audio::RenderableSource
//

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

namespace Audio {

// Forward declarations

class Source;
class Listener;

/**
 * Renderable Source abstract class
 *
 * @remarks This is the interface to renderer-specific sources. All abstract sources
 *      must get attached to a renderable source. And each renderable source is attached to
 *      an abstract source.
 *      @par All attributes come from the abstract source, so this class only has renderer-specific
 *      state and resources, plus implementation functions. Since it is intended to be attached
 *      to abstract sources as any user data would, it is a subclass of UserData.
 * @note Since this mutual attachment would create circular references,
 *      it is implemented on this side with raw pointers. No problem should arise since
 *      the smart pointer used in abstract sources already handles everything correctly,
 *      but care must be had not to have detached renderable sources around.
 *
 */
class RenderableSource : public UserData {
private:
    Source *source;

protected:
    /** Internal constructor used by derived classes */
    RenderableSource(Source *source);

public:
    virtual ~RenderableSource();

    enum UpdateFlags {
        UPDATE_ALL = 0x0F,
        UPDATE_LOCATION = 0x01,
        UPDATE_ATTRIBUTES = 0x02,
        UPDATE_EFFECTS = 0x04,
        UPDATE_GAIN = 0x08
    };

    /** Play the source from the specified timestamp
     * @param start The starting position. Defaults to the beginning.
     * @remarks It just plays. Will not synchronize attributes with the underlying API.
     *      That must be done through a separate update() call.
     */
    void startPlaying(Timestamp start = 0);

    /** Stop a playing source
     * @remarks If the source is playing, stop it. Otherwise, do nothing.
     */
    void stopPlaying();

    /** Is the source still playing? */
    bool isPlaying() const;

    /** Get the playing position of a playing source
     * @remarks Will throw if it's not playing!
     */
    Timestamp getPlayingTime() const;

    /** Get the attached source */
    Source *getSource() const {
        return source;
    }

    /** Seek to the specified position
     * @note It may not be supported by the renderer on all sources.
     *      Streaming sources are guaranteed to perform a rough seek on a best effort
     *      basis, meaning the effective time after the seek may be off a bit, and
     *      the process may be costly.
     *       Seeking in non-streaming sources may not be supported at all.
     * @throws EndOfStreamException if you try to seek past the end
     */
    void seek(Timestamp time);

    /** Update the underlying API with dirty attributes
     * @param flags You may specify which attributes to update. Not all attributes are
     *      equally costly, so you'll want to ease up on some, pump up some others.
     *      You may or-combine flags.
     * @param sceneListener A reference listener. If it is the root listener attached
     *      to the renderer, no special translation is done, but if it is not, coordinates
     *      will be first translated to listener-space.
     * @remarks Although the implementation may throw exceptions, the interface will
     *      ignore them (just log them or something like that). Updates are non-critical
     *      and may fail silently.
     */
    void update(int flags, const Listener &sceneListener);

    // The following section contains all the virtual functions that need be implemented
    // by a concrete Sound class. All are protected, so the interface is independent
    // of implementations.
protected:

    /** @see startPlaying
     * @param start The starting position.
     */
    virtual void startPlayingImpl(Timestamp start) = 0;

    /** @see stopPlaying.*/
    virtual void stopPlayingImpl() = 0;

    /** @see isPlaying.*/
    virtual bool isPlayingImpl() const = 0;

    /** @see getPlayingTime.*/
    virtual Timestamp getPlayingTimeImpl() const = 0;

    /** @see update. */
    virtual void updateImpl(int flags, const Listener &sceneListener) = 0;

    /** @see seek. */
    virtual void seekImpl(Timestamp time) = 0;
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERABLESOURCE_H
