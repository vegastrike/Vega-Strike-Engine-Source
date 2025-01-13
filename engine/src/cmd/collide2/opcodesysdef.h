/*
 * opcodesysdef.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2CS_COMPAT_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2CS_COMPAT_H


//#define OPC_USE_CALLBACKS 1

#define ICE_NO_DLL
#define CS_PROCESSOR_X86
#define CS_NO_QSQRT

#ifndef CS_FORCEINLINE
# ifdef CS_COMPILER_GCC
#  define CS_FORCEINLINE inline __attribute__((always_inline))
#  if (__GNUC__ == 3) && (__GNUC_MINOR__ == 4)
// Work around a gcc 3.4 issue where forcing inline doesn't always work
#   define CS_FORCEINLINE_TEMPLATEMETHOD inline
#  endif
# else
#  define CS_FORCEINLINE inline
# endif
#endif
#ifndef CS_FORCEINLINE_TEMPLATEMETHOD
# define CS_FORCEINLINE_TEMPLATEMETHOD CS_FORCEINLINE
#endif

#include "gfx/quaternion.h"
#define SMALL_EPSILON .000001
#define EPSILON .00001
#define ABS(x) (x>=0?x:-x)
#define __CS_CSSYSDEFS_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>

class csObject {
};
struct iBase {
};

#define CS_ASSERT assert

#include "opcodetypes.h"
#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2CS_COMPAT_H
