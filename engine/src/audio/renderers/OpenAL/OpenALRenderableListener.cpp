/*
 * OpenALRenderableListener.cpp
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


//
// C++ Implementation: Audio::OpenALRenderableListener
//

#include "OpenALRenderableListener.h"

#include "al.h"

#include "../../Listener.h"

namespace Audio {

OpenALRenderableListener::OpenALRenderableListener(Listener *listener)
        : RenderableListener(listener) {
}

OpenALRenderableListener::~OpenALRenderableListener() {
}

void OpenALRenderableListener::updateImpl(int flags) {
    if (flags & UPDATE_LOCATION) {
        const Vector3 pos(getListener()->getPosition()); // no option but to cast it down to float :(
        const Vector3 vel(getListener()->getVelocity());
        const Vector3 at(getListener()->getAtDirection());
        const Vector3 up(getListener()->getUpDirection());
        ALfloat ori[] = {at.x, at.y, at.z, up.x, up.y, up.z};

        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
        alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
        alListenerfv(AL_ORIENTATION, ori);
    }
    if (flags & UPDATE_ATTRIBUTES) {
        alListenerf(AL_GAIN, getListener()->getGain());
    }
}

};
