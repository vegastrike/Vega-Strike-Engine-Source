/*
 * vegastrike.h
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 *
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */
#ifndef VEGA_STRIKE_ENGINE_VEGASTRIKE_H
#define VEGA_STRIKE_ENGINE_VEGASTRIKE_H

#ifdef __cplusplus
#include <assert.h>
#endif //__cplusplus

extern float simulation_atom_var;
extern float audio_atom_var;
//#define SIMULATION_ATOM (simulation_atom_var)
//#define AUDIO_ATOM (audio_atom_var)

// Why do we need two variables to reflect the same thing ?
extern float SIMULATION_ATOM;
extern float AUDIO_ATOM;

#include "vs_math.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "debug_vs.h"

#if !defined(_WIN32)
#   define GL_GLEXT_PROTOTYPES 1
#endif

#if defined (WIN32) || defined (__CYGWIN__)
/* Note that this will define WIN32 for us, if it isn't defined already
 */
#ifndef NOMINMAX
    #define NOMINMAX
#endif //tells VCC not to generate min/max macros
    #include <windows.h>
 //#include <wingdi.h>
    #include <time.h>
#else //defined (WIN32) || defined (__CYGWIN__)
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif //defined (WIN32) || defined (__CYGWIN__)

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if !defined (NO_GFX)
#   if defined(__APPLE__) && defined(__MACH__)
#       define GL_GLEXT_FUNCTION_POINTERS 1
#       include <gl.h>
#       include <glu.h>
#       include <glut.h>
#   else //defined(__APPLE__) && defined(__MACH__)
#       define __glext_h_
#       include <gl.h>
#       include <glu.h>
#       include <glut.h>
#       undef __glext_h_
#   endif //defined(__APPLE__) && defined(__MACH__)
#endif //NO_GFX

#define PROG_NAME "vegastrike"

#define BUFF_LEN 512

//#define RANDOMIZE_SIM_ATOMS 1

#ifdef __cplusplus

#endif //__cplusplus

#endif //VEGA_STRIKE_ENGINE_VEGASTRIKE_H
