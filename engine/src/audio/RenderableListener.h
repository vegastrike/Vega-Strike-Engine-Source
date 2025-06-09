/*
 * RenderableListener.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERABLELISTENER_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERABLELISTENER_H

//
// C++ Interface: Audio::RenderableListener
//

#include "Exceptions.h"
#include "Types.h"

namespace Audio {

// Forward declarations

class Listener;

/**
 * Renderable Listener abstract class
 *
 * @remarks This is the interface to renderer-specific listeners.
 *      Listeners attached to a renderer receive one such instance
 *      that allows listener-specific commands to be given to the
 *      renderer, like requesting position updates and such.
 * @note Since this mutual attachment would create circular references,
 *      it is implemented on this side with raw pointers. No problem should arise since
 *      the smart pointer used in abstract listener already handles everything correctly,
 *      but care must be had not to have detached renderable listeners around.
 *
 */
class RenderableListener : public UserData {
private:
    Listener *listener;

protected:
    /** Internal constructor used by derived classes */
    RenderableListener(Listener *listener);

public:
    virtual ~RenderableListener();

    enum UpdateFlags {
        UPDATE_ALL = 0x0F,
        UPDATE_LOCATION = 0x01,
        UPDATE_ATTRIBUTES = 0x02,
        UPDATE_EFFECTS = 0x04,
        UPDATE_GAIN = 0x08
    };

    /** Get the attached listener */
    Listener *getListener() const {
        return listener;
    }

    /** Update the underlying API with dirty attributes
     * @param flags You may specify which attributes to update. Not all attributes are
     *      equally costly, so you'll want to ease up on some, pump up some others.
     *      You can or-combine flags.
     * @remarks Although the implementation may throw exceptions, the interface will
     *      ignore them (just log them or something like that). Updates are non-critical
     *      and may fail silently.
     */
    void update(int flags);

    // The following section contains all the virtual functions that need be implemented
    // by a concrete class. All are protected, so the interface is independent
    // of implementations.
protected:

    /** @see update. */
    virtual void updateImpl(int flags) = 0;
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERABLELISTENER_H
