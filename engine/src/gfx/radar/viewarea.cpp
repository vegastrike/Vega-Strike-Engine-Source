// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * viewarea.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <cmath>
#include "gfxlib_struct.h"
#include "viewarea.h"

namespace Radar {

ViewArea::ViewArea()
        : sprite(0) {
}

void ViewArea::SetSprite(VSSprite *sprite) {
    this->sprite = sprite;
    if (sprite) {
        sprite->GetPosition(position.x, position.y);
        position.z = 0;
        sprite->GetSize(size.x, size.y);
        // TODO: turn floating point comparisons into a function
        size.x = std::fabs(size.x);
        size.y = std::fabs(size.y);
        size.z = 0;
    } else {
        position = Vector(0, 0, 0);
        size = Vector(0, 0, 0);
    }
}

Vector ViewArea::Scale(const Vector &v) const {
    assert(sprite);

    return Vector(position.x + size.x / 2 * v.x,
            position.y + size.y / 2 * v.y,
            position.z + size.y / 2 * v.z);
}

GFXColor ViewArea::GetColor() const {
    return GFXColor(0.0, 0.5, 0.0, 1.0);
}

bool ViewArea::IsActive() const {
    return (sprite != 0);
}

} // namespace Radar
