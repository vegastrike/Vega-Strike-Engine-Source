// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/**
 * radar.cpp
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


#include <cassert>
#include <stdexcept>
#include "sensor.h"
#include "radar.h"
#include "null_display.h"
#include "sphere_display.h"
#include "bubble_display.h"
#include "plane_display.h"

namespace Radar {

std::unique_ptr<Display> Factory(Type::Value type)
{
    switch (type) {
        case Type::NullDisplay:
            return std::unique_ptr<Display>(new NullDisplay);

        case Type::SphereDisplay:
            return std::unique_ptr<Display>(new SphereDisplay);

        case Type::BubbleDisplay:
            return std::unique_ptr<Display>(new BubbleDisplay);

        case Type::PlaneDisplay:
            return std::unique_ptr<Display>(new PlaneDisplay);

        default:
            assert(false);
            throw std::invalid_argument("Unknown radar type");
    }
}

} // namespace Radar
