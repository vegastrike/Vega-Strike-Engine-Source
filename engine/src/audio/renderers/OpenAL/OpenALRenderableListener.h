/**
 * OpenALRenderableListener.h
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
// C++ Interface: Audio::OpenALRenderableListener
//
#ifndef __AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__
#define __AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__

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

#endif//__AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__
