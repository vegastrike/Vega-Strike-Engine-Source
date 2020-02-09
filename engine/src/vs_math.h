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

#ifndef _VS_MATH_H_
#define _VS_MATH_H_

#include <config.h>

#include "macosx_math.h"
#include <math.h>
#include <float.h>

#ifdef HAVE_IEEEFP_H
 #include <ieeefp.h>
#endif

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif
#ifndef PI
 #define PI M_PI
#endif
#ifndef M_1_PI
 #define M_1_PI (1/M_PI)
#endif

#if defined (HAVE_MATH_H)
 #include <math.h>
#endif
#if  defined (HAVE_FINITE) || defined (BSD)
 #define FINITE( x ) ( finite( x ) )
 #define ISNAN( x ) ( isnan( x ) )
#else
 #define FINITE( x ) ( _finite( x ) )
 #define ISNAN( x ) ( _isnan( x ) )
#endif

#endif // _VS_MATH_H_
