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
 *
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */
#ifndef VEGA_STRIKE_ENGINE_VS_MATH_H
#define VEGA_STRIKE_ENGINE_VS_MATH_H

#include <config.h>

#include "macosx_math.h"
#include <math.h>
#include <float.h>
#include <cmath>

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


#endif //VEGA_STRIKE_ENGINE_VS_MATH_H
