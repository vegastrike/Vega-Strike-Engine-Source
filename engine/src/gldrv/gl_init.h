/*
 * gl_init.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_GLDRV_GL_INIT_H
#define VEGA_STRIKE_ENGINE_GLDRV_GL_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

//#include "src/vegastrike.h"

/* Hack to fix compiling problem with old gl.h's, reported by Steve
 *  Baker <sjbaker1@airmail.net>.  Some old gl.h's don't include glext.h, but
 *  do this:
 *
 * #define GL_EXT_compiled_vertex_array    1
 *
 *  since they do define the glLockArraysEXT/glUnlockArraysEXT
 *  functions.  However, this prevents PFNGLLOCKARRAYSEXTPROC /
 *  PFNGLUNLOCKARRAYSEXTPROC from being defined in glext.h.  So, we do
 *  the following, which at worst results in a warning (and is awfully
 *  ugly):
 *
 * #undef GL_EXT_compiled_vertex_array
 *
 *  The *correct* thing to do would be for gl.h to #include glext.h, as
 *  recent gl.h's do.  However, versions of Mesa as recent as 3.2.1
 *  don't do this, so we have to work around it.
 */
//#undef GL_EXT_compiled_vertex_array

/* Shouldn't need to include glext.h if gl.h is recent, but alas we can't
 * count on that...  */

#if !defined (_WIN32)
// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#include <glut.h>
#include <glext.h>

#if !defined (GL_GLEXT_VERSION) || GL_GLEXT_VERSION < 6
#   error "*** You need a more recent copy of glext.h.  You can get one at http: //oss.sgi.com/projects/ogl-sample/ABI/glext.h ; it goes in /usr/include/GL. ***"
#endif
#endif
#if defined (PFNGLLOCKARRAYSEXTPROC) && defined (PFNGLUNLOCKARRAYSEXTPROC)
extern PFNGLLOCKARRAYSEXTPROC   glLockArraysEXT_p;
extern PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT_p;
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //VEGA_STRIKE_ENGINE_GLDRV_GL_INIT_H
