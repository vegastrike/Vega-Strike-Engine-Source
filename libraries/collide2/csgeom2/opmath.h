/*
  Copyright (C) 2005 by Marten Svanfeldt
  Copyright (C) 2022-2025 Stephen G. Tuggy, Benjamen R. Meyer
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_MATH_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_MATH_H

#define HAVE_ISNORMAL 1

#include <cmath>
#include <cfloat>
#include "collide2/opcodealgorithms.h"
/*
inline bool isfinited (float x) {return((x) >= -DBL_MAX && (x) <= DBL_MAX);}
inline bool isfinitef (float x) {return((x) >= -FLT_MAX && (x) <= FLT_MAX);}

inline float isfinite (float f) {  return (sizeof(f) == sizeof(float) ? isfinitef(f) : isfinited(f));}
*/
/**\file 
 * Generic mathematic utility functions.
 */
/**
 * \addtogroup geom_utils
 * @{ */

/**
 * Returns bigger of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T &csMax(const T &a, const T &b) {
    if (b < a) {
        return a;
    }
    return b;
}

/**
 * Returns smaller of a and b. If they are equal, a or b can be returned.
 */
template<class T>
const T &csMin(const T &a, const T &b) {
    if (a < b) {
        return a;
    }
    return b;
}

/**
 * Sort a and b in order of size.
 */
template<class T>
void csSort(T &a, T &b) {
    if (b < a) {
        CS::Swap(a, b);
    }
}

/**
 * Sort a and b in order of size.
 * If swapping them, also swap x and y
 */
template<class T, class U>
void csSort(T &a, T &b, U &x, U &y) {
    if (b < a) {
        CS::Swap(a, b);
        CS::Swap(x, y);
    }
}

/**
 * Clamp a between max and min.
 */
template<class T>
T csClamp(const T &a, T max, T min) {
    return csMin(csMax(a, min), max);
}

/**
 * Performs a smooth interpolation of a on range min to max.
 * \return Smooth interporlated value if \a min \< \a a \< \a max, 
 *  and 0 resp. 1 if \a a is smaller than \a min resp. larger than \a max.
 */
template<class T>
T csSmoothStep(const T &a, T max, T min) {
    T tmp, tmp2;
    if (a <= min) {
        tmp = 0.0f;
    } else if (a >= max) {
        tmp = 1.0f;
    } else {
        tmp2 = (a - min) / (max - min);
        tmp = tmp2 * tmp2 * (3.0 - 2.0 * tmp2);
    }
    return tmp;
}

/**
 * Performs a linear interpolation between \a a and \a b with the factor
 * \a f.
 */
template<class T, class Tfactor>
T csLerp(const T &a, const T &b, const Tfactor &f) {
    return (a + (b - a) * f);
}

/**
 * Returns the square of the argument
 */
template<class T>
T csSquare(const T &x) {
    return x * x;
}

//@{
/// Checks if a floating point value is finite.
CS_FORCEINLINE bool csFinite(float f) {
#if defined (HAVE_FINITEF)
    return finitef (f);
#elif defined (HAVE_STD__ISFINITE)
    return std::isfinite (f);
#elif defined(HAVE_ISFINITE)
    return (isfinite (f));
#elif defined (HAVE_FINITE)
    return finite(f);
#elif defined (HAVE__FINITE)
    return _finite (f) != 0;
#else
#error Your platform has no isfinite()-alike function!
#endif
}

/// Checks if a double-precision floating point value is finite.
CS_FORCEINLINE bool csFinite(double d) {
#if defined (HAVE_STD__ISFINITE)
    return std::isfinite (d);
#elif defined(HAVE_ISFINITE)
    return isfinite (d);
#elif defined (HAVE_FINITE)
    return finite(d);
#elif defined (HAVE__FINITE)
    return _finite (d) != 0;
#else
#error Your platform has no isfinite()-alike function!
#endif
}

/// Checks if a floating point value is not-a-number.
CS_FORCEINLINE bool csNaN(float f) {
#if defined (HAVE_NANF)
    return isnanf (f);
#elif defined (HAVE_STD__ISNAN)
    return std::isnan (f);
#elif defined(HAVE_ISNAN)
    return isnan(f);
#elif defined (HAVE__ISNAN)
    return _isnan (f) != 0;
#else
#error Your platform has no isnan()-alike function!
#endif
}

/// Checks if a double-precision floating point value is not-a-number.
CS_FORCEINLINE bool csNaN(double d) {
#if defined (HAVE_STD__ISNAN)
    return std::isnan (d);
#elif defined(HAVE_ISNAN)
    return isnan(d);
#elif defined (HAVE__ISNAN)
    return _isnan (d) != 0;
#else
#error Your platform has no isnan()-alike function!
#endif
}

/// Checks if a floating point value is normal (not infinite or nan).
CS_FORCEINLINE bool csNormal(float f) {
#if defined (HAVE_NORMALF)
    return normalf (f);
#elif defined (HAVE_STD__ISNORMAL)
    return std::isnormal (f);
#elif defined(HAVE_ISNORMAL)
    return isnormal(f);
#else
    return csFinite(f) && !csNaN(f);
#endif
}

/// Checks if a double-precision floating point value is normal.
CS_FORCEINLINE bool csNormal(double d) {
#if defined (HAVE_STD__ISNORMAL)
    return std::isnormal (d);
#elif defined(HAVE_ISNORMAL)
    return isnormal(d);
#else
    return csFinite(d) && !csNaN(d);
#endif
}
//@}

/** @} */

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_MATH_H
