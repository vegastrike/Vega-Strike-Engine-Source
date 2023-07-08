/*
    Copyright (C) 1998-2004 by Jorrit Tyberghein
  
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

#ifndef __CS_CSTYPES_H__
#define __CS_CSTYPES_H__

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

/**\name Specific sized types
 * These types should be used ONLY when you need a variable of an explicit
 * number of bits.  For all other cases, you should use normal char, short,
 * int, long, etc., types since they are treated as "natural" types and will
 * generally have better performance characteristics than the explicitly-sized
 * types. Use the explicitly-sized types sparingly.
 * @{ */

#if !defined(HAVE_STDINT_H) && !defined(HAVE_INTTYPES_H)
/// unsigned 8-bit integer (0..255)
typedef unsigned char uint8;
/// signed 8-bit integer (-128..127)
typedef char int8;
/// unsigned 16-bit integer (0..65 535)
typedef unsigned short uint16;
/// signed 16-bit integer (-32 768..32 767)
typedef short int16;
/// unsigned 32-bit integer (0..4 294 967 295)
typedef unsigned int uint32;
/// signed 32-bit integer (-2 147 483 648..2 147 483 647)
typedef int int32;
#if defined(CS_COMPILER_GCC)
#ifndef __STRICT_ANSI__
/// unsigned 64-bit integer
typedef unsigned long long uint64;
/// signed 64-bit integer
typedef long long int64;
#endif
#elif defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)
/// unsigned 64 bit integer
typedef unsigned __int64 uint64;
/// signed 64 bit integer
typedef __int64 int64;
#else
#error Do not know how to declare 64-bit integers
#endif // CS_COMPILER_GCC

#else // CS_HAVE_STDINT_H || CS_HAVE_INTTYPES_H

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
#endif

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
#include <stddef.h>
#if !defined(CS_HAVE_INTPTR_T) && !defined(_INTPTR_T_DEFINED)

#if CS_PROCESSOR_SIZE == 64
typedef int64 intptr_t;
typedef uint64 uintptr_t;
typedef int64 ptrdiff_t;
#else
/// Integer at least as wide as a pointer
typedef int intptr_t;
/// Unsigned integer at least as wide as a pointer
typedef unsigned int uintptr_t;
/// Difference of 2 pointers
typedef int ptrdiff_t;
#endif

#define _INTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
#define _PTRDIFF_T_DEFINED
#endif

#if !defined(CS_HAVE_INTMAX_T) && !defined(__APPLE__)
/// Greatest-width integer
typedef int64 intmax_t;
/// Greatest-width unsigned integer
typedef uint64 uintmax_t;
#endif


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


#if defined(CS_COMPILER_GCC)
#ifndef __STRICT_ANSI__
/**
 * Type to pass to cs_snprintf() as an argument to the "%lld" format specifier.
 */
typedef long long longlong;
/**
 * Type to pass to cs_snprintf() as an argument to the "%llu" format specifier.
 */
typedef unsigned long long ulonglong;
#else
// @@@ Correct?
typedef int64 longlong;
typedef uint64 ulonglong;
#endif
#elif defined(CS_COMPILER_MSVC) || defined(CS_COMPILER_BCC)
typedef int64 longlong;
typedef uint64 ulonglong;
#else
#ifdef HAVE_STDINT_H
typedef int_least64_t longlong;
typedef uint_least64_t ulonglong;
#else 
#error Do not know how to declare (u)longlong types
#endif 
#endif 

/**
 * A time value measured in milliseconds (1/1000 of a second).  Ticks do not
 * represent wall clock time or any other Epoch-based time.  Instead, ticks are
 * useful only for measuring differences between points on a timeline, or for
 * specifying intervals.
 */
typedef unsigned int csTicks;

/// Shortcut for default unsigned int.
typedef unsigned int uint;
/** @} */

/** @} */

#endif // __CS_CSTYPES_H__
