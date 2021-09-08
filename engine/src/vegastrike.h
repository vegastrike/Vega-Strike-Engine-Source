/*
 * vegastrike.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020-2021 pyramid3d, Stephen G. Tuggy, and
 * other Vega Strike contributors
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
 *
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */


#ifdef __cplusplus
#include <assert.h>
#endif

#ifndef _VEGASTRIKE_H_
#define _VEGASTRIKE_H_

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




#if defined (WIN32) || defined (__CYGWIN__)
/* Note that this will define WIN32 for us, if it isn't defined already
 */
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif //tells VCC not to generate min/max macros
 #include <windows.h>
 //#include <wingdi.h>
 #include <time.h>
#else
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <unistd.h>
 //#include "command.h"
#endif /* defined( WIN32 ) || defined( __CYGWIN__ ) */

#ifndef NO_GFX
 #if defined (__APPLE__) || defined (MACOSX)
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
 #else
  #define __glext_h_
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
  #undef __glext_h_
 #endif
#endif

#define PROG_NAME "vegastrike"

#define BUFF_LEN 512

//#define RANDOMIZE_SIM_ATOMS 1

#endif

#ifdef __cplusplus

#endif

