/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 * 
 * http://vegastrike.sourceforge.net/
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
 */

#include <GL/gl.h>
#include "gl_globals.h"

//Matrix model = {0};
//Matrix view = {0};

//int sharedcolortable = 0;
GLenum GFXStage0 = 0;
GLenum GFXStage1 = 1;
int Stage0Texture = 1;
int Stage0TextureName;
int Stage1Texture= 0;
int Stage1TextureName;

namespace GFXMatrices {
  Matrix model,view, projection, invprojection;
  Matrix  rotview;
}

//PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
//PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
//PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB;
#ifdef STATS_QUEUE
queue<GFXStats> statsqueue;

#endif
