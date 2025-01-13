/*
 * IcePreprocessor.h
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains preprocessor stuff. This should be the first included header.
 *	\file		IcePreprocessor.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2022-01-06
 *  Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PREPROCESSOR_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PREPROCESSOR_H

// Check platform
#if defined( _WIN32 ) || defined( WIN32 )
//#pragma message("Compiling on Windows...")
#define PLATFORM_WINDOWS
#else
//#pragma message("Compiling on unknown platform...")
#endif

// Check compiler
#if defined(_MSC_VER)
//#pragma message("Compiling with VC++...")
#define COMPILER_VISUAL_CPP
#else
//#pragma message("Compiling with unknown compiler...")
#endif

// Check compiler options. If this file is included in user-apps, this
// shouldn't be needed, so that they can use what they like best.
#ifndef ICE_DONT_CHECK_COMPILER_OPTIONS
#ifdef COMPILER_VISUAL_CPP
#if defined(_CHAR_UNSIGNED)
#endif

#if defined(_CPPRTTI)
#error Please disable RTTI...
#endif

#if defined(_CPPUNWIND)
#error Please disable exceptions...
#endif

#if defined(_MT)
// Multithreading
#endif
#endif
#endif

// Check debug mode
#ifdef    DEBUG            // May be defined instead of _DEBUG. Let's fix it.
#ifndef	_DEBUG
#define _DEBUG
#endif
#endif

#ifdef  _DEBUG
// Here you may define items for debug builds
#endif

#ifndef THIS_FILE
#define THIS_FILE            __FILE__
#endif

#ifndef ICE_NO_DLL
#ifdef ICECORE_EXPORTS
#define ICECORE_API			__declspec(dllexport)
#else
#define ICECORE_API			__declspec(dllimport)
#endif
#else
#define ICECORE_API
#endif

// Don't override new/delete
//	#define DEFAULT_NEWDELETE
#define DONT_TRACK_MEMORY_LEAKS

#define FUNCTION                extern "C"

// Cosmetic stuff [mainly useful with multiple inheritance]
#define    override(base_class)    virtual

// Our own inline keyword, so that:
// - we can switch to __forceinline to check it's really better or not
// - we can remove __forceinline if the compiler doesn't support it
//	#define inline_				__forceinline
//	#define inline_				inline

// Contributed by Bruce Mitchener
#if defined(COMPILER_VISUAL_CPP)
#define inline_			__forceinline
//			#define inline_			inline
#elif defined(__GNUC__) && __GNUC__ < 3
#define inline_ inline
#elif defined(__GNUC__)
#define inline_ inline __attribute__ ((always_inline))
#else
#define inline_ inline
#endif

#ifdef COMPILER_VISUAL_CPP
// Down the hatch
#pragma inline_depth( 255 )
#pragma intrinsic(labs)
#endif

// Silly Cygwin defines _X in ctypes.h
#ifdef _X
#undef _X
#endif

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PREPROCESSOR_H
