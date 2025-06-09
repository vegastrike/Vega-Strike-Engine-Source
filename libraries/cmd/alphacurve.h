/*
 * alphacurve.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ALPHA_CURVE_H
#define VEGA_STRIKE_ENGINE_CMD_ALPHA_CURVE_H

#include <cmath>

//query = requested pixel alpha					any
//maxrez x = width of the image in pixels				any
//min = minimum alpha						min 0
//max = maximum alpha						max 255
//focus = multiplier of focus center for curve			0-1
//concavity = multiplier from linear -> terminal(0 or 255)	0-1
//tail_mode_start = overriding slope value for start		negative = standard, 0=flat, high=vertical
//tail_mode_end = overriding slope value for start		negative = standard, 0=flat, high=vertical

int get_alpha(int _query,
        int _maxrez_x,
        int _min,
        int _max,
        double _focus,
        double _concavity,
        int _tail_mode_start,
        int _tail_mode_end);

#endif //VEGA_STRIKE_ENGINE_CMD_ALPHA_CURVE_H
