/**
 * cargo_color.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H
#define VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H

#include "src/gfxlib_struct.h"
#include "cargo.h"

// TODO: remove this. That's what std::pair is for.
// A stupid struct that is only for grouping 2 different types of variables together in one return value
class CargoColor {
public:
    Cargo cargo;
    GFXColor color;

    CargoColor() : cargo(), color(1, 1, 1, 1) {
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_CARGO_COLOR_H
