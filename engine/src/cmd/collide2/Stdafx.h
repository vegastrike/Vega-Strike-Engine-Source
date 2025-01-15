///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 *
 *  Copyright (C) Daniel Horn, chuck starchaser, and pheonixstorm
 *  Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 *  Copyright (C) 2021-2023 Stephen G. Tuggy, Benjamen R. Meyer
 *
 *  https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 *  This file is part of Vega Strike.
 *
 *  Vega Strike is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Vega Strike is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_STDAFX_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_STDAFX_H

// Stephen G. Tuggy getting rid of this hack entirely
// chuck starchaser shutting up the warnings about violations of strict aliasing...
// #ifndef _WIN32 // pheonixstorm shutting up pragma warnings on win32
// #pragma GCC diagnostic ignored "-Wstrict-aliasing"
// #endif
//believe me I tried to fix the code, using a union of float, int and unsigned, but the amount of
//changes needed in the code was immense.
//So, bottom line:
//    assert( sizeof(float) == sizeof(int) == sizeof(unsigned int) == sizeof(dword) ) or this library won't work
//also
//    assert( (dword&)float(0.0f) == (dword&)int(0) ) must pass or this library will blow on your face

#include <memory>
#include "opcodetypes.h"
#include "opcodesysdef.h"
#include "Opcode.h"
#include "vs_vector.h"

#endif    //VEGA_STRIKE_ENGINE_CMD_COLLSION2_STDAFX_H
