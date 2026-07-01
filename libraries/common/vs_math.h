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
#ifndef VEGA_STRIKE_LIBRARIES_COMMON_VS_MATH_H
#define VEGA_STRIKE_LIBRARIES_COMMON_VS_MATH_H

#include <config.h>

#include <math.h>
#include <float.h>
#include <cmath>
#include <algorithm>

#ifdef HAVE_IEEEFP_H
    #include <ieeefp.h>
#endif //HAVE_IEEEFP_H

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif //M_PI

#ifndef PI
    #define PI M_PI
#endif //PI

#ifndef M_1_PI
    #define M_1_PI (1/M_PI)
#endif //M_1_PI

#define FINITE(x) ( std::isfinite( x ) )
#define ISNAN(x) ( std:: isnan( x ) )

// Adapted from example given at https://en.cppreference.com/cpp/types/numeric_limits/epsilon . Retrieved 2026-06-24.
template <typename T,
std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
bool equal_within_ulps(T x, T y, std::size_t n)
{
    // Since `epsilon()` is the gap size (ULP, unit in the last place)
    // of floating-point numbers in interval [1, 2), we can scale it to
    // the gap size in interval [2^e, 2^{e+1}), where `e` is the exponent
    // of `x` and `y`.

    // If `x` and `y` have different gap sizes (which means they have
    // different exponents), we take the smaller one. Taking the bigger
    // one is also reasonable, I guess.
    const T m = std::min(std::fabs(x), std::fabs(y));

    // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
    const int exp = m < std::numeric_limits<T>::min()
                  ? std::numeric_limits<T>::min_exponent - 1
                  : std::ilogb(m);

    // We consider `x` and `y` equal if the difference between them is
    // within `n` ULPs.
    return std::fabs(x - y) <= n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
}

#endif //VEGA_STRIKE_LIBRARIES_COMMON_VS_MATH_H
