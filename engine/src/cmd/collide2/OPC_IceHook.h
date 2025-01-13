/*
 * OPC_IceHook.h
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
// NO HEADER GUARD

// Should be included by Opcode.h if needed

#define ICE_DONT_CHECK_COMPILER_OPTIONS

// From Windows...
typedef int BOOL;
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#ifndef OPASSERT
#define    OPASSERT(exp)    {}
#endif
#define ICE_COMPILE_TIME_ASSERT(exp)    extern char ICE_Dummy[ (exp) ? 1 : -1 ]

//	extern void Opcode_Log (const char* msg, ...);
//	#define	Log			Opcode_Log
//#define	Log				{}
//	extern bool Opcode_Err (const char* msg, ...);
//	#define	SetIceError		Opcode_Err
#define    EC_OUTOFMEMORY    "Out of memory"

#include "Ice/IcePreprocessor.h"

#undef ICECORE_API
#define ICECORE_API    OPCODE_API

#include "Ice/IceTypes.h"
#include "Ice/IceFPU.h"
#include "Ice/IceMemoryMacros.h"
#define ICEMATHS_API    OPCODE_API
