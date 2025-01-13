/*
 * opcodetypes.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
    Copyright (C) 1998-2004 by Jorrit Tyberghein
    Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other
    Vega Strike contributors
    Copyright (C) 2021-2023 Stephen G. Tuggy, Benjamen R. Meyer
  
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_CSTYPES_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_CSTYPES_H

// config.h is the VS ./configure determined header.
#include <float.h>
#include <wchar.h>
#if defined(_WIN32) && !defined(__CYGWIN__) // && defined(_MSC_VER)
// Hack
#define CS_COMPILER_MSVC 1
#ifdef _WIN64
#  define CS_PROCESSOR_SIZE 64 /* Only used if intptr_t is not found. */
#else
#  define CS_PROCESSOR_SIZE 32 /* Only used if intptr_t is not found. (cough VC6 cough) */
#endif
#define CS_HAVE_INTPTR_T 1

#else /* GCC or other platform */

#define CS_COMPILER_GCC 1
#define CS_PROCESSOR_SIZE 64 /* Upper limit on pointer size, only used it intptr_t is not found. */
#define CS_HAVE_INTPTR_T 1

#endif
// End Hack


#if defined(HAVE_STDINT_H)
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#endif

#if defined(HAVE_INTTYPES_H)
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#endif

// /**\name Specific sized types
//  * These types should be used ONLY when you need a variable of an explicit
//  * number of bits.  For all other cases, you should use normal char, short,
//  * int, long, etc., types since they are treated as "natural" types and will
//  * generally have better performance characteristics than the explicitly-sized
//  * types. Use the explicitly-sized types sparingly.
//  * @{ */

// typedef uint8_t opc_uint8;
// typedef int8_t opc_int8;
// typedef uint16_t opc_uint16;
// typedef int16_t opc_int16;
// typedef uint32_t opc_uint32;
// typedef int32_t opc_int32;
// typedef uint64_t opc_uint64;
// typedef int64_t opc_int64;

#ifdef CS_HAVE_INT64_C

/**\def CONST_INT64
 * Specify a 64 bit integer constant.
 * Compilers differ in 64-bit wide number specification. Employ this
 * macro to use the appropriate mechanism automatically.
 */
#define CONST_INT64(x) INT64_C(x)

/**\def CONST_UINT64
 * Specify 64 bit unsigned integer constant.
 * Compilers differ in 64-bit wide number specification. Employ this
 * macro to use the appropriate mechanism automatically.
 */
#define CONST_UINT64(x) UINT64_C(x)

#else // CS_HAVE_INT64_C

#if defined(CS_COMPILER_GCC)
#define CONST_INT64(x)  x ## LL
#define CONST_UINT64(x) x ## ULL
#elif defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)
#define CONST_INT64(x)  x##i64
#define CONST_UINT64(x) x##ui64
#else
#error Do not know how to contruct 64-bit integer constants
#endif // CS_COMPILER_GCC

#endif // CS_HAVE_INT64_C

/** @} */

/**\name Other types
 * @{ */

// Provide intptr_t and uintptr_t. If the configure script determined that
// these types exist in the standard headers, then just employ those types.
// For MSVC, where the configure script is not used, check <stddef.h>, which is
// one of several headers which may provide these types. We can tell if
// <stddef.h> provided the types by checking if _INTPTR_T_DEFINED has been
// #defined; newer versions of MSVC will provide them; older ones will not.  If
// all else fails, then we fake up these types on our own.
//
// 2021-06-26 SGT: I believe these are standard types now in C++11 and later.
#include <stddef.h>
typedef intptr_t opc_ptrdiff_t;

// Provide wchar_t and wint_t. If the configure script determined that these
// types exist in the standard headers, then just employ those types.  For
// MSVC, where the configure script is not used, check <stddef.h>, <wchar.h>,
// and <wctype.h>, which are three of several headers which may provide these
// types. We can tell if these headers provided the types by checking if
// _WCHAR_T_DEFINED and _WCTYPE_T_DEFINED have been #defined; newer versions of
// MSVC will provide them; older ones will not.  If all else fails, then we
// fake up these types on our own. glibc also #defines _WINT_T when wint_t is
// available, so we double-check that, as well. Many modern compilaers also
// allow us to check for wint_t directly, so we also do that (seems to be
// necessary in FreeBSD). FreeBSD does define _WINT_T_DECLARED, so we check fo tha
// too.
#include <stddef.h>
#if defined(HAVE_WCTYPE_H)
#include <wctype.h>
#endif
#if !((defined(CS_HAVE_WINT_T) && defined(_WCTYPE_T_DEFINED)) || defined(_WINT_T) || defined(_WINT_T_DECLARED))
#ifndef wint_t
#if _MSC_VER >= 1300
typedef unsigned short wint_t;
#else
typedef wchar_t wint_t;
#endif
#endif
#define _WCTYPE_T_DEFINED
#define _WINT_T
#define _WINT_T_DECLARED
#endif

/**
 * A time value measured in milliseconds (1/1000 of a second).  Ticks do not
 * represent wall clock time or any other Epoch-based time.  Instead, ticks are
 * useful only for measuring differences between points on a timeline, or for
 * specifying intervals.
 */
typedef unsigned int csTicks;

/// Shortcut for default unsigned int.
typedef unsigned int opc_uint;
/** @} */

/** @} */

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_CS_CSTYPES_H
