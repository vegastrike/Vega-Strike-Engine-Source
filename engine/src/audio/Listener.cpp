/*
 * Listener.cpp
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
// C++ Implementation: Audio::Source
//

#include "Listener.h"

#include <math.h>

namespace Audio {

Listener::Listener() :
        cosAngleRange(-1, -1),
        position(0, 0, 0),
        atDirection(0, 0, -1),
        upDirection(0, 1, 0),
        velocity(0, 0, 0),
        radius(1),
        gain(1),
        worldToLocal(1) // set to identity, given default settings
{
}

Listener::~Listener() {
}

Range<Scalar> Listener::getAngleRange() const {
    return Range<Scalar>(Scalar(acos(cosAngleRange.min)),
            Scalar(acos(cosAngleRange.max)));
}

void Listener::setAngleRange(Range<Scalar> r) {
    cosAngleRange.min = Scalar(cos(r.min));
    cosAngleRange.max = Scalar(cos(r.max));
    dirty.attributes = 1;
}

void Listener::update(int flags) {
    if (!dirty.attributes) {
        flags &= ~RenderableListener::UPDATE_ATTRIBUTES;
    }
    if (!dirty.location) {
        flags &= ~RenderableListener::UPDATE_LOCATION;
    }
    if (!dirty.gain) {
        flags &= ~RenderableListener::UPDATE_GAIN;
    }

    if (getRenderable().get() != 0) {
        getRenderable()->update(flags);
    }

    if (flags & RenderableListener::UPDATE_ATTRIBUTES) {
        dirty.attributes = 0;
    }
    if (flags & RenderableListener::UPDATE_GAIN) {
        dirty.gain = 0;
    }
    if (flags & RenderableListener::UPDATE_LOCATION) {
        worldToLocal =
                Matrix3(
                        atDirection.cross(upDirection),
                        upDirection,
                        -atDirection
                ).inverse();
        dirty.location = 0;
    }
}

Vector3 Listener::toLocalDirection(Vector3 dir) const {
    return worldToLocal * dir;
}

};
