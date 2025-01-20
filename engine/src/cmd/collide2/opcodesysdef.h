/**
 * opcodesysdef.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
 * Copyright (C) 2021-2025 Stephen G. Tuggy, Benjamen R. Meyer
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

#include <cstdlib>
#include <cstring>
#include <cassert>

class csObject {
};
struct iBase {
};

#define CS_ASSERT assert

#include "opcodetypes.h"
#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2CS_COMPAT_H
