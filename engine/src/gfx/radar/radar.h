// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * radar.h
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


#ifndef VEGASTRIKE_GFX_RADAR_RADAR_H
#define VEGASTRIKE_GFX_RADAR_RADAR_H

#include <string>
#include <vector>
#include <memory>

class Unit;
class VSSprite;

namespace Radar {

class Sensor;

// Draws radar on display
class Display {
public:
    virtual ~Display() {
    }

    virtual void Draw(const Sensor &sensor, VSSprite *, VSSprite *) = 0;

    virtual void OnDockEnd() {
    }

    virtual void OnJumpBegin() {
    }

    virtual void OnJumpEnd() {
    }

    virtual void OnPauseBegin() {
    }

    virtual void OnPauseEnd() {
    }
};

struct Type {
    enum Value {
        NullDisplay,
        SphereDisplay,
        BubbleDisplay,
        PlaneDisplay
    };
};

std::unique_ptr<Display> Factory(Type::Value);

} // namespace Radar

#endif
