/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_CUBE_MAP_FIRST_H
#define VEGA_STRIKE_ENGINE_CUBE_MAP_FIRST_H
/* Always include this file first */


//When including windows.h, it must come before other headers;
//but you should tell VCC not to generate min and max macros...
#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//After windows.h, we can include algorithm by default, as it
//has things that are used almost universally, like std::min/max
#include <algorithm>
using std::min;
using std::max;
#ifdef abs
#undef abs
#endif

template<typename T>
inline T operator abs(T const &t) {
    return t < t(0) ? -t : t;
}

//and might as well include math.h; we'll need it all over the place
#include "math.h"

//These handy templates save us a lot of coding operators. Just define
//assignment arithmetic operators, and these others are auto-generated
template<typename T>
inline T operator+(T const &a, T const &b) {
    return T(a) += b;
}

template<typename T>
inline T operator-(T const &a, T const &b) {
    return T(a) -= b;
}

//another thing we need universally is errors.h, as it handles asserts, exceptions and logs
#include "errors/errors.h"

//and finally units, as everything uses parameters, and all parameters should have units
#include "units/number.h"
#include "units/radians.h"
#include "units/steradians.h"
#include "units/shininess.h"

#endif //VEGA_STRIKE_ENGINE_CUBE_MAP_FIRST_H
