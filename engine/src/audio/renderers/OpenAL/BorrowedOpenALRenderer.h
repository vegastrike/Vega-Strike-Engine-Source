/*
 * BorrowedOpenALRenderer.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_BORROWED_OPENAL_RENDERER_H
#define VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_BORROWED_OPENAL_RENDERER_H

//
// C++ Interface: Audio::SceneManager
//

#include "OpenALRenderer.h"
#include "al.h"

namespace Audio {

/**
 * Hooks up the renderer to an already-existing OpenAL context
 *
 * @remarks If you're already using OpenAL in a standard fashion, you can
 *      still benefit from the scene graph provided by the Audio library
 *      by initializing the OpenAL renderer through this method.
 *          The default constructor will assume the current OpenAL context
 *      has been previously initialized and hook up to that context.
 */
class BorrowedOpenALRenderer : public OpenALRenderer {
public:
    /**
     * Hooks up the renderer to an already-existing OpenAL context
     *
     * @remarks If you're already using OpenAL in a standard fashion, you can
     *      still benefit from the scene graph provided by the Audio library
     *      by initializing the OpenAL renderer through this method.
     *          If any parameter was passed as null, it will be retrieved
     *      from the current OpenAL context (which must have been previously
     *      initialized)
     *
     * @param device The OpenAL device associated to this context
     * @param context The OpenAL context associated to this renderer
     */
    BorrowedOpenALRenderer(ALCdevice *device = 0, ALCcontext *context = 0);

    ~BorrowedOpenALRenderer();

    virtual void setOutputFormat(const Format &format);

protected:
    virtual void checkContext();
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_RENDERERS_OPENAL_BORROWED_OPENAL_RENDERER_H
