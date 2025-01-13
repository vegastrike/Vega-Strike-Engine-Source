/*
 * OpenALRenderableListener.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERABLE_LISTENER_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERABLE_LISTENER_H

//
// C++ Interface: Audio::OpenALRenderableListener
//

#include "../../RenderableListener.h"

#include "../../Exceptions.h"
#include "../../Types.h"

namespace Audio {

/**
 * OpenAL Renderable Listener class
 *
 * @remarks This class implements the RenderableListener interface for the
 *      OpenAL renderer.
 *
 */
class OpenALRenderableListener : public RenderableListener {
public:
    OpenALRenderableListener(Listener *listener);

    virtual ~OpenALRenderableListener();

protected:
    /** @see RenderableListener::update. */
    virtual void updateImpl(int flags);
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_RENDERABLE_LISTENER_H
