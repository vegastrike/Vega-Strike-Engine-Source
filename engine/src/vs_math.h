/*
 * vs_math.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
 *
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */
#ifndef VEGA_STRIKE_ENGINE_VS_MATH_H
#define VEGA_STRIKE_ENGINE_VS_MATH_H

#include <config.h>

#include "root_generic/macosx_math.h"
#include <float.h>
#include <cmath>
#include <numbers>

#ifdef HAVE_IEEEFP_H
    #include <ieeefp.h>
#endif //HAVE_IEEEFP_H

constexpr double kVegaPiDouble = std::numbers::pi_v<double>;
constexpr float kVegaPiFloat = std::numbers::pi_v<float>;
constexpr double kVegaInvPiDouble = std::numbers::inv_pi_v<double>;
constexpr float kVegaInvPiFloat = std::numbers::inv_pi_v<float>;

#define FINITE(x) ( std::isfinite( x ) )
#define ISNAN(x) ( std:: isnan( x ) )


#endif //VEGA_STRIKE_ENGINE_VS_MATH_H
