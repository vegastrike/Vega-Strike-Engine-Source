/*
 * gl_misc.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Alan Shieh specifically
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


#include <cstring>
#define GL_MISC_CPP
#include "gl_globals.h"
#undef GL_MISC_CPP
#include "vegastrike.h"
#include "gfxlib.h"
#include "vs_globals.h"
#include "gl_light.h"
#include "winsys.h"
#include "options.h"



// disable clientside draw for debugging purposes
//#define NODRAW 1

// cached gl state
static unsigned int s_array_buffer = 0;
static unsigned int s_element_array_buffer = 0;

bool GFXMultiTexAvailable() {
    return gl_options.Multitexture != 0;
}

void GFXCircle(float x, float y, float wid, float hei) {
    float segmag =
            (Vector(wid * g_game.x_resolution, 0,
                    0)
                    - Vector(static_cast<double>(wid) * g_game.x_resolution * cos(2.0 * M_PI / 360.0),
                            static_cast<double>(hei) * g_game.y_resolution * sin(2.0 * M_PI / 360.0),
                            0)).Magnitude();
    int accuracy = (int) (360.0f * vs_options::instance().circle_accuracy * (1.0f < segmag ? 1.0 : segmag));
    if (accuracy < 4) {
        accuracy = 4;
    }
    float iaccuracy = 1.0f / accuracy;

    std::vector<float> verts(3 * (accuracy + 1));
    float *v = &verts[0];
    for (int i = 0; i <= accuracy; i++) {
        *v++ = x + wid * cos(i * 2 * M_PI * iaccuracy);
        *v++ = y + hei * sin(i * 2 * M_PI * iaccuracy);
        *v++ = 0.0f;
    }
    GFXDraw(GFXLINESTRIP, &verts[0], accuracy + 1);
}

static void /*GFXDRVAPI*/ GFXDrawSetup(POLYTYPE type, const float data[], int vnum,
        int vsize, int csize, int tsize0, int tsize1) {
    assert(data && vsize);
    int stride = sizeof(float) * (vsize + csize + tsize0 + tsize1);

    GFXBindBuffer(0);

    glVertexPointer(vsize, GL_FLOAT, stride, data);
    glEnableClientState(GL_VERTEX_ARRAY);

    if (csize) {
        glColorPointer(csize, GL_FLOAT, stride, data + vsize);
        glEnableClientState(GL_COLOR_ARRAY);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    if (gl_options.Multitexture) {
        if (tsize0) {
            glClientActiveTextureARB_p(GL_TEXTURE0);
            glTexCoordPointer(tsize0, GL_FLOAT, stride, data + vsize + csize);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        if (tsize1) {
            glClientActiveTextureARB_p(GL_TEXTURE1);
            glTexCoordPointer(tsize1, GL_FLOAT, stride, data + vsize + csize + tsize0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    } else if (tsize0) {
        glTexCoordPointer(tsize0, GL_FLOAT, stride, data + vsize + csize);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
}

static void /*GFXDRVAPI*/ GFXDrawCleanup(POLYTYPE type, const float data[], int vnum,
        int vsize, int csize, int tsize0, int tsize1) {
    if (gl_options.Multitexture) {
        if (tsize1) {
            glClientActiveTextureARB_p(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        if (tsize0) {
            if (tsize1) {
                glClientActiveTextureARB_p(GL_TEXTURE0);
            }
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    } else if (tsize0) {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

    if (csize) {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
}

void /*GFXDRVAPI*/ GFXDraw(POLYTYPE type, const float data[], int vnum,
        int vsize, int csize, int tsize0, int tsize1) {
#ifndef NODRAW
    if (vnum <= 0) {
        return;
    }

    GFXDrawSetup(type, data, vnum, vsize, csize, tsize0, tsize1);

    glDrawArrays(PolyLookup(type), 0, vnum);

    GFXDrawCleanup(type, data, vnum, vsize, csize, tsize0, tsize1);
#endif
}

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type,
        const float data[], int vnum, const unsigned char indices[], int nelem,
        int vsize, int csize, int tsize0, int tsize1) {
#ifndef NODRAW
    if (vnum <= 0 || nelem <= 0) {
        return;
    }

    GFXDrawSetup(type, data, vnum, vsize, csize, tsize0, tsize1);

    // Note: glDrawRangeElements doesn't seem to work well in MESA,
    //       stay away from it
    GFXBindElementBuffer(0);
    glDrawElements(PolyLookup(type), nelem, GL_UNSIGNED_BYTE, indices);

    GFXDrawCleanup(type, data, vnum, vsize, csize, tsize0, tsize1);
#endif
}

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type,
        const float data[], int vnum, const unsigned short indices[], int nelem,
        int vsize, int csize, int tsize0, int tsize1) {
#ifndef NODRAW
    if (vnum <= 0 || nelem <= 0) {
        return;
    }

    GFXDrawSetup(type, data, vnum, vsize, csize, tsize0, tsize1);

    // Note: glDrawRangeElements doesn't seem to work well in MESA,
    //       stay away from it
    GFXBindElementBuffer(0);
    glDrawElements(PolyLookup(type), nelem, GL_UNSIGNED_SHORT, indices);

    GFXDrawCleanup(type, data, vnum, vsize, csize, tsize0, tsize1);
#endif
}

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type,
        const float data[], int vnum, const unsigned int indices[], int nelem,
        int vsize, int csize, int tsize0, int tsize1) {
#ifndef NODRAW
    if (vnum <= 0 || nelem <= 0) {
        return;
    }

    GFXDrawSetup(type, data, vnum, vsize, csize, tsize0, tsize1);

    // Note: glDrawRangeElements doesn't seem to work well in MESA,
    //       stay away from it
    GFXBindElementBuffer(0);
    glDrawElements(PolyLookup(type), nelem, GL_UNSIGNED_INT, indices);

    GFXDrawCleanup(type, data, vnum, vsize, csize, tsize0, tsize1);
#endif
}

void /*GFXDRVAPI*/ GFXBindBuffer(unsigned int vbo_data) {
#ifndef NO_VBO_SUPPORT
    if (s_array_buffer != vbo_data) {
        s_array_buffer = vbo_data;
        (*glBindBufferARB_p)(GL_ARRAY_BUFFER_ARB, vbo_data);
    }
#endif
}

void /*GFXDRVAPI*/ GFXBindElementBuffer(unsigned int element_data) {
#ifndef NO_VBO_SUPPORT
    if (s_element_array_buffer != element_data) {
        s_element_array_buffer = element_data;
        (*glBindBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, element_data);
    }
#endif
}

void /*GFXDRVAPI*/ GFXBeginScene() {
    GFXLoadIdentity(MODEL);     //bad this should instead load the cached view matrix
    light_rekey_frame();
}

void /*GFXDRVAPI*/ GFXEndScene() {
    winsys_swap_buffers();     //swap the buffers
#ifdef NODRAW
    GFXClear( GFXTRUE );
#endif
}

void /*GFXDRVAPI*/ GFXClear(const GFXBOOL colorbuffer, const GFXBOOL depthbuffer, const GFXBOOL stencilbuffer) {
    glClear((colorbuffer ? GL_COLOR_BUFFER_BIT : 0)
            | (depthbuffer ? GL_DEPTH_BUFFER_BIT : 0)
            | (stencilbuffer ? GL_STENCIL_BUFFER_BIT : 0));
}

GFXBOOL /*GFXDRVAPI*/ GFXCapture(char *filename) {
    return GFXFALSE;
}

static float last_factor = 0;
static float last_units = 0;

void GFXGetPolygonOffset(float *factor, float *units) {
    *factor = last_factor;
    *units = last_units;
}

void /*GFXDRVAPI*/ GFXPolygonOffset(float factor, float units) {
    last_factor = factor;
    last_units = units;
    if (!factor && !units) {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_POINT);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(0, 0);
    } else {
        glPolygonOffset(factor, units);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_POINT);
        glEnable(GL_POLYGON_OFFSET_LINE);
    }
}

void /*GFXDRVAPI*/ GFXPolygonMode(const enum POLYMODE polymode) {
    GLenum mode;
    switch (polymode) {
        default:
        case GFXFILLMODE:
            mode = GL_FILL;
            break;
        case GFXLINEMODE:
            mode = GL_LINE;
            break;
        case GFXPOINTMODE:
            mode = GL_POINT;
            break;
    }
    glPolygonMode(GL_FRONT_AND_BACK, mode);
}

void /*GFXDRVAPI*/ GFXCullFace(const enum POLYFACE polyface) {
    GLenum face;
    switch (polyface) {
        case GFXFRONT:
            face = GL_FRONT;
            break;
        default:
        case GFXBACK:
            face = GL_BACK;
            break;
        case GFXFRONTANDBACK:
            face = GL_FRONT_AND_BACK;
            break;
    }
    glCullFace(face);
}

void GFXPointSize(const float size) {
    glPointSize(size);
}

void GFXLineWidth(const float size) {
    glLineWidth(size);
}

void /*GFXDRVAPI*/ GFXColorf(const GFXColor &col) {
    glColor4fv(&col.r);
}

GFXColor GFXColorf() {
    float col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);     //It's best this way, we don't use it much, anyway.
    return GFXColor(col[0], col[1], col[2], col[3]);
}

#if 0
//HELL slow on the TNT...we can't have it
void /*GFXDRVAPI*/ GFXBlendColor( const GFXColor &col )
{
#ifndef WIN32
    glBlendColor( col.r, col.g, col.b, col.a );
#endif
}
#endif

void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a) {
    glColor4f(r, g, b, a);
}

int GFXCreateList() {
    glGetError();
    int list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    return list;
}

GFXBOOL GFXEndList() {
    glEndList();
    return glGetError() != GL_OUT_OF_MEMORY;
}

void GFXCallList(int list) {
    glCallList(list);
}

void GFXDeleteList(int list) {
    if (glIsList(list)) {
        glDeleteLists(list, 1);
    }
}

void GFXSubwindow(int x, int y, int xsize, int ysize) {
    glViewport(x, y, xsize, ysize);
    glScissor(x, y, xsize, ysize);
    if (x == 0 && y == 0 && xsize == g_game.x_resolution && ysize == g_game.y_resolution) {
        glDisable(GL_SCISSOR_TEST);
    } else {
        glEnable(GL_SCISSOR_TEST);
    }
}

void GFXSubwindow(float x, float y, float xsize, float ysize) {
    GFXSubwindow(int(x * g_game.x_resolution), int(y * g_game.y_resolution), int(xsize * g_game.x_resolution),
            int(ysize * g_game.y_resolution));
}

Vector GFXDeviceToEye(int x, int y) {
    float l, r, b, t, n, f;
    GFXGetFrustumVars(true, &l, &r, &b, &t, &n, &f);
    return Vector((l + (r - l) * float(x) / g_game.x_resolution),
            (t + (b - t) * float(y) / g_game.y_resolution),
            n);
}
