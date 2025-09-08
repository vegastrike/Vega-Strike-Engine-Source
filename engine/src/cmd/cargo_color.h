/*
 * cargo_color.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H
#define VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H

#include "src/gfxlib_struct.h"
#include "resource/cargo.h"

// TODO: remove this. That's what std::pair is for.
// A stupid struct that is only for grouping 2 different types of variables together in one return value
class CargoColor {
public:
    Cargo cargo;
    GFXColor color;

    CargoColor() : cargo(), color(1, 1, 1, 1) {
    }

    CargoColor(const CargoColor& other) = default;
    CargoColor(CargoColor&& other) = default;
    CargoColor& operator=(CargoColor&& other) = default;
};

#endif //VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H
