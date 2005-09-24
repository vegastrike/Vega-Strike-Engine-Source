/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * Portions of this code from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
#include <assert.h>
#endif

#ifndef _VEGASTRIKE_H_
#define _VEGASTRIKE_H_

#if 0
static const float SIMULATION_ATOM = (float)(1.0/10.0);
  extern float simulation_atom_var; // not used, just dummy to compile
#else
extern float simulation_atom_var;
#define SIMULATION_ATOM (simulation_atom_var)
#endif

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include "macosx_math.h"
#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "debug_vs.h"
#if defined( WIN32 ) || defined( __CYGWIN__ )
    /* Note that this will define WIN32 for us, if it isn't defined already
     */
#  include <windows.h>
//#include <wingdi.h>
#include <time.h>
#else
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include "command.h"
#endif /* defined( WIN32 ) || defined( __CYGWIN__ ) */

#if defined(__APPLE__) || defined(MACOSX)
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

#ifndef PI
#   define PI 3.14159265358979323846
#endif
#ifndef M_PI
#   define M_PI 3.1415926535
#endif

#define PROG_NAME "vegastrike"

#include <float.h>

#if defined(HAVE_MATH_H)
#include <math.h>
#endif
#if  defined(HAVE_FINITE) || defined(BSD)
#   define FINITE(x) (finite(x))
#else
#	define FINITE(x) (_finite(x))
#endif


#define BUFF_LEN 512


#endif

#ifdef __cplusplus

#endif

