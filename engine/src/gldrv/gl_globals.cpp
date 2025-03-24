/*
 * Copyright (C) 2001-2022 Daniel Horn, Alan Shieh, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "gl_globals.h"
#include "src/gfxlib.h"
#include "gfx_generic/matrix.h"
//Matrix model = {0};
//Matrix view = {0};

//int sharedcolortable = 0;
gl_options_t gl_options;

namespace GFXMatrices {
Matrix model, view;
float projection[16];
float invprojection[16];
Matrix rotview;
}

int gl_vertices_this_frame = 0;
int gl_batches_this_frame = 0;


//PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
//PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
//PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB;
#ifdef STATS_QUEUE
queue< GFXStats >statsqueue;

#endif

