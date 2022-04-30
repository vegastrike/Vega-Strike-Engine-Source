/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "gfxlib_struct.h"
#include "gfxlib.h"
#include "gldrv/gl_globals.h"
#include <stdio.h>
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_random.h"
#include "vs_logging.h"

#include "options.h"

#include <vector>

GLenum PolyLookup(POLYTYPE poly) {
    switch (poly) {
        case GFXTRI:
            return GL_TRIANGLES;

        case GFXQUAD:
            return GL_QUADS;

        case GFXTRISTRIP:
            return GL_TRIANGLE_STRIP;

        case GFXQUADSTRIP:
            return GL_QUAD_STRIP;

        case GFXTRIFAN:
            return GL_TRIANGLE_FAN;

        case GFXPOLY:
            return GL_POLYGON;

        case GFXLINE:
            return GL_LINES;

        case GFXLINESTRIP:
            return GL_LINE_STRIP;

        case GFXPOINT:
            return GL_POINTS;

        default:
            return GL_TRIANGLES;
    }
}

static void clear_gl_error() {
    glGetError();
}

static void print_gl_error(const char *fmt) {
    GLenum gl_error = glGetError();
    if (!!gl_error) {
        VS_LOG(error, (boost::format(fmt) % gl_error));
    }
}

static void EnableArrays(const GFXColorVertex *data) {
    if (gl_options.Multitexture) {
        glClientActiveTextureARB_p(GL_TEXTURE0);
    }
    glInterleavedArrays(GL_T2F_C4F_N3F_V3F, sizeof(GFXColorVertex), data);
    if (gl_options.Multitexture) {
        glClientActiveTextureARB_p(GL_TEXTURE1);
        glTexCoordPointer(2, GL_FLOAT, sizeof(GFXColorVertex), &data[0].s);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB_p(GL_TEXTURE2);
        glTexCoordPointer(4, GL_FLOAT, sizeof(GFXColorVertex), &data[0].tx);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB_p(GL_TEXTURE0);
    }
}

static void EnableArrays(const GFXVertex *data) {
    if (gl_options.Multitexture) {
        glClientActiveTextureARB_p(GL_TEXTURE0);
    }
    glInterleavedArrays(GL_T2F_N3F_V3F, sizeof(GFXVertex), data);
    if (gl_options.Multitexture) {
        glClientActiveTextureARB_p(GL_TEXTURE1);
        glTexCoordPointer(2, GL_FLOAT, sizeof(GFXVertex), &data[0].s);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB_p(GL_TEXTURE2);
        glTexCoordPointer(4, GL_FLOAT, sizeof(GFXVertex), &data[0].tx);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTextureARB_p(GL_TEXTURE0);
    }
}

void GFXVertexList::RefreshDisplayList() {
#ifndef NO_VBO_SUPPORT
    if (game_options()->vbo && !vbo_data) {
        if (glGenBuffersARB_p == nullptr || glBindBufferARB_p == nullptr || glBufferDataARB_p == nullptr || glMapBufferARB_p == nullptr
                || glUnmapBufferARB_p == nullptr) {
            game_options()->vbo = false;
        } else {
            (*glGenBuffersARB_p)(1, (GLuint *) &vbo_data);
            if (changed & HAS_INDEX) {
                (*glGenBuffersARB_p)(1, (GLuint *) &display_list);
            }
        }
    }
    if (vbo_data) {
        GFXBindBuffer(vbo_data);
        (*glBufferDataARB_p)(GL_ARRAY_BUFFER_ARB, numVertices
                        * ((changed & HAS_COLOR) ? sizeof(GFXColorVertex) : sizeof(GFXVertex)), data.vertices,
                (changed & CHANGE_MUTABLE) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
        if (changed & HAS_INDEX) {
            GFXBindElementBuffer(display_list);
            unsigned int tot = 0;
            for (int i = 0; i < numlists; ++i) {
                tot += offsets[i];
            }
            unsigned int indexsize = (changed & INDEX_BYTE)
                    ? sizeof(char)
                    : ((changed & INDEX_SHORT)
                            ? sizeof(unsigned short)
                            : sizeof(unsigned int));
            (*glBufferDataARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB, tot * indexsize, &index.b[0],
                    (changed & CHANGE_MUTABLE) ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB);
        }
        return;
    }
#endif
    if ((!gl_options.display_lists) || (display_list && !(changed & CHANGE_CHANGE)) || (changed & CHANGE_MUTABLE)) {
        return;
    }          //don't used lists if they're mutable
    if (display_list) {
        GFXDeleteList(display_list);
    }
    int offset = 0;
    display_list = GFXCreateList();
    if (changed & HAS_COLOR) {
        EnableArrays(data.colors);
    } else {
        EnableArrays(data.vertices);
    }
    for (int i = 0; i < numlists; i++) {
        // Populate the display list with array data
        if (changed & HAS_INDEX) {
            switch (changed & (INDEX_BYTE | INDEX_INT | INDEX_SHORT)) {
                case INDEX_BYTE:
                    glDrawElements(PolyLookup(mode[i]), offsets[i], GL_UNSIGNED_BYTE, index.b);
                    break;
                case INDEX_SHORT:
                    glDrawElements(PolyLookup(mode[i]), offsets[i], GL_UNSIGNED_SHORT, index.s);
                    break;
                case INDEX_INT:
                    glDrawElements(PolyLookup(mode[i]), offsets[i], GL_UNSIGNED_INT, index.i);
                    break;
                default:
                    break;
            }
        } else {
            glDrawArrays(PolyLookup(mode[i]), offset, offsets[i]);
        }
        offset += offsets[i];
    }
    if (!GFXEndList()) {
        GFXDeleteList(display_list);
        display_list = 0;
    }
}

void GFXVertexList::BeginDrawState(GFXBOOL lock) {
    if (!numVertices) {
        return;
    }          //don't do anything if there are no vertices

#ifndef NO_VBO_SUPPORT
    if (vbo_data) {
        clear_gl_error();

        GFXBindBuffer(vbo_data);
        print_gl_error("VBO18.5a Error %1%");

        if (changed & HAS_INDEX) {
            GFXBindElementBuffer(display_list);
            print_gl_error("VBO18.5b Error %1%");
        }

        if (changed & HAS_COLOR) {
            EnableArrays((GFXColorVertex *) NULL);
        } else {
            EnableArrays((GFXVertex *) NULL);
        }
    } else
#endif
    if (display_list == 0) {
        if (changed & HAS_COLOR) {
            EnableArrays(data.colors);
        } else {
            EnableArrays(data.vertices);
        }
#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
        if (lock && glLockArraysEXT_p) {
            (*glLockArraysEXT_p)(0, numVertices);
        }
#endif
    }
}

extern void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a);

void GFXVertexList::EndDrawState(GFXBOOL lock) {
    if (vbo_data) {
#ifndef NO_VBO_SUPPORT
        if (gl_options.Multitexture) {
            glClientActiveTextureARB_p(GL_TEXTURE0);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTextureARB_p(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTextureARB_p(GL_TEXTURE2);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glClientActiveTextureARB_p(GL_TEXTURE0);
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
#endif
    } else if (display_list != 0) {
    } else {
#ifndef NO_COMPILEDVERTEXARRAY_SUPPORT
        if (lock && glUnlockArraysEXT_p && numVertices) {
            (*glUnlockArraysEXT_p)();
        }
#endif
    }
    if (changed & HAS_COLOR) {
        GFXColor4f(1, 1, 1, 1);
    }
}

extern GLenum PolyLookup(POLYTYPE poly);

void GFXVertexList::Draw(enum POLYTYPE poly, int numV) {
    INDEX index;
    index.b = NULL;
    Draw(&poly, index, 1, &numV);
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned char *index) {
    char tmpchanged = changed;
    changed = sizeof(unsigned char) | ((~HAS_INDEX) & changed);
    INDEX tmp;
    tmp.b = (index);
    Draw(&poly, tmp, 1, &numV);
    changed = tmpchanged;
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned short *index) {
    char tmpchanged = changed;
    changed = sizeof(unsigned short) | ((~HAS_INDEX) & changed);
    INDEX tmp;
    tmp.s = (index);
    Draw(&poly, tmp, 1, &numV);
    changed = tmpchanged;
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned int *index) {
    char tmpchanged = changed;
    changed = sizeof(unsigned int) | ((~HAS_INDEX) & changed);
    INDEX tmp;
    tmp.i = (index);
    Draw(&poly, tmp, 1, &numV);
    changed = tmpchanged;
}

void GFXVertexList::DrawOnce() {
    LoadDrawState();
    BeginDrawState(GFXFALSE);
    Draw();
    EndDrawState(GFXFALSE);
}

void GFXVertexList::Draw() {
    Draw(mode, index, numlists, offsets);
}

extern void GFXCallList(int list);

void GFXVertexList::Draw(enum POLYTYPE *mode, const INDEX index, const int numlists, const int *offsets) {
    //Hardware support for this seems... sketchy
    if (vbo_data == 0 && display_list != 0) {
        //Big issue: display lists cannot discriminate between lines/points/triangles,
        //so, for now, we'll limit smoothing to single-mode GFXVertexLists, which, by the way,
        //are the only ones being used, AFAIK.
        bool blendchange = false;
        if (unique_mode && (numlists > 0)) {
            switch (*mode) {
                case GFXLINE:
                case GFXLINESTRIP:
                case GFXPOLY:
                case GFXPOINT:
                    if (((*mode == GFXPOINT)
                            && gl_options.smooth_points) || ((*mode != GFXPOINT) && gl_options.smooth_lines)) {
                        BLENDFUNC src, dst;
                        GFXGetBlendMode(src, dst);
                        if ((dst != ZERO) && ((src == ONE) || (src == SRCALPHA))) {
                            GFXPushBlendMode();
                            GFXBlendMode(SRCALPHA, dst);
                            GFXEnable(SMOOTH);
                            blendchange = true;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        GFXCallList(display_list);
        if (blendchange) {
            GFXPopBlendMode();
            GFXDisable(SMOOTH);
        }

        ++gl_batches_this_frame;
    } else {
        int totoffset = 0;
        if (changed & HAS_INDEX) {
            long stride = changed & HAS_INDEX;
            GLenum indextype = (changed & INDEX_BYTE)
                    ? GL_UNSIGNED_BYTE
                    : ((changed & INDEX_SHORT)
                            ? GL_UNSIGNED_SHORT
                            : GL_UNSIGNED_INT);
            bool use_vbo = vbo_data != 0;
            use_vbo = use_vbo && memcmp(&index, &this->index, sizeof(INDEX)) == 0;
            if (use_vbo) {
                #ifndef NO_VBO_SUPPORT
                GFXBindElementBuffer(display_list);
                #else
                use_vbo = false;
                #endif
            } else {
                #ifndef NO_VBO_SUPPORT
                if (vbo_data) {
                    GFXBindElementBuffer(0);
                }
                #endif
            }
            if (glMultiDrawElements_p != NULL && numlists > 1) {
                static std::vector<bool> drawn;
                static std::vector<const GLvoid *> glindices;
                static std::vector<GLsizei> glcounts;

                drawn.clear();
                drawn.resize(numlists, false);
                for (int i = 0; i < numlists; totoffset += offsets[i++]) {
                    if (!drawn[i]) {
                        glindices.clear();
                        glcounts.clear();
                        int totcount = 0;
                        for (long j = i, offs = totoffset; j < numlists; offs += offsets[j++]) {
                            totcount += offsets[j];
                            if (!drawn[j] && (mode[j] == mode[i])) {
                                glindices.push_back(use_vbo ? (GLvoid *) (stride * offs)
                                        : (GLvoid *) &index.b[stride * offs]);
                                glcounts.push_back(offsets[j]);
                                drawn[j] = true;
                            }
                        }
                        if (glindices.size() == 1) {
                            glDrawElements(PolyLookup(mode[i]), glcounts[0], indextype, glindices[0]);
                        } else {
                            glMultiDrawElements_p(PolyLookup(
                                    mode[i]), &glcounts[0], indextype, &glindices[0], glindices.size());
                        }
                        ++gl_batches_this_frame;
                        gl_vertices_this_frame += totcount;
                    }
                }
            } else {
                for (int i = 0; i < numlists; i++) {
                    glDrawElements(PolyLookup(mode[i]), offsets[i], indextype,
                            use_vbo ? (GLvoid *) (stride * totoffset)
                                    : (GLvoid *) &index.b[stride
                                    * totoffset]);                     //changed&INDEX_BYTE == stride!
                    totoffset += offsets[i];
                    ++gl_batches_this_frame;
                    gl_vertices_this_frame += offsets[i];
                }
            }
        } else {
            if (glMultiDrawArrays_p) {
                static std::vector<bool> drawn;
                static std::vector<GLint> gloffsets;
                static std::vector<GLsizei> glcounts;

                drawn.clear();
                drawn.resize(numlists, false);
                for (int i = 0; i < numlists; totoffset += offsets[i++]) {
                    if (!drawn[i]) {
                        gloffsets.clear();
                        glcounts.clear();
                        int totcount = 0;
                        for (int j = i, offs = totoffset; j < numlists; offs += offsets[j++]) {
                            totcount += offsets[j];
                            if (!drawn[j] && (mode[j] == mode[i])) {
                                gloffsets.push_back(offs);
                                glcounts.push_back(offsets[j]);
                                drawn[j] = true;
                            }
                        }
                        bool blendchange = false;
                        switch (mode[i]) {
                            case GFXLINE:
                            case GFXLINESTRIP:
                            case GFXPOLY:
                            case GFXPOINT:
                                if (((mode[i] == GFXPOINT)
                                        && gl_options.smooth_points)
                                        || ((mode[i] != GFXPOINT) && gl_options.smooth_lines)) {
                                    BLENDFUNC src, dst;
                                    GFXGetBlendMode(src, dst);
                                    if ((dst != ZERO) && ((src == ONE) || (src == SRCALPHA))) {
                                        GFXPushBlendMode();
                                        GFXBlendMode(SRCALPHA, dst);
                                        GFXEnable(SMOOTH);
                                        blendchange = true;
                                    }
                                }
                                break;
                            default:
                                break;
                        }
                        if (gloffsets.size() == 1) {
                            glDrawArrays(PolyLookup(mode[i]), gloffsets[0], glcounts[0]);
                        } else {
                            glMultiDrawArrays_p(PolyLookup(mode[i]), &gloffsets[0], &glcounts[0], gloffsets.size());
                        }
                        if (blendchange) {
                            GFXPopBlendMode();
                            GFXDisable(SMOOTH);
                        }
                        ++gl_batches_this_frame;
                        gl_vertices_this_frame += totcount;
                    }
                }
            } else {
                for (int i = 0; i < numlists; i++) {
                    bool blendchange = false;
                    switch (mode[i]) {
                        case GFXLINE:
                        case GFXLINESTRIP:
                        case GFXPOLY:
                        case GFXPOINT:
                            if (((mode[i] == GFXPOINT)
                                    && gl_options.smooth_points)
                                    || ((mode[i] != GFXPOINT) && gl_options.smooth_lines)) {
                                BLENDFUNC src, dst;
                                GFXGetBlendMode(src, dst);
                                if ((dst != ZERO) && ((src == ONE) || (src == SRCALPHA))) {
                                    GFXPushBlendMode();
                                    GFXBlendMode(SRCALPHA, dst);
                                    GFXEnable(SMOOTH);
                                    blendchange = true;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                    glDrawArrays(PolyLookup(mode[i]), totoffset, offsets[i]);
                    totoffset += offsets[i];
                    if (blendchange) {
                        GFXPopBlendMode();
                        GFXDisable(SMOOTH);
                    }
                    ++gl_batches_this_frame;
                    gl_vertices_this_frame += offsets[i];
                }
            }
        }
    }
}

GFXVertexList::~GFXVertexList() {
// #ifndef NO_VBO_SUPPORT
//     if (vbo_data) {
//         (*glDeleteBuffersARB_p)(1, (GLuint*) &vbo_data);
//         if (display_list) {
//             (*glDeleteBuffersARB_p)(1, (GLuint*) &display_list);
//         }
//     } else
// #endif
    if (vbo_elements != nullptr) {
        (*glDeleteBuffersARB_p)(1, vbo_elements);
        vbo_elements = nullptr;
    }
    if (display_list != 0) {
        GFXDeleteList(display_list);          //delete dis
        display_list = 0;
    }
    if (offsets != nullptr) {
        delete[] offsets;
        offsets = nullptr;
    }
    if (mode != nullptr) {
        delete[] mode;
        mode = nullptr;
    }
    if (changed & HAS_COLOR) {
        if (data.colors != nullptr) {
            free(data.colors);
            data.colors = nullptr;
        }
    } else if (data.vertices != nullptr) {
        free(data.vertices);
        data.vertices = nullptr;
    }
}

union GFXVertexList::VDAT *GFXVertexList::Map(bool read, bool write) {
#ifndef NO_VBO_SUPPORT
    if (GFX_BUFFER_MAP_UNMAP) {
        if (vbo_data) {
            if (display_list) {
                GFXBindElementBuffer(display_list);
                index.b =
                        (unsigned char *) (*glMapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                read ? (write ? GL_READ_WRITE_ARB : GL_READ_ONLY_ARB)
                                        : GL_WRITE_ONLY_ARB);
            }
            GFXBindBuffer(vbo_data);
            void *ret =
                    (*glMapBufferARB_p)(GL_ARRAY_BUFFER_ARB,
                            read ? (write ? GL_READ_WRITE_ARB : GL_READ_ONLY_ARB) : GL_WRITE_ONLY_ARB);
            if (changed & HAS_COLOR) {
                data.colors = (GFXColorVertex *) ret;
            } else {
                data.vertices = (GFXVertex *) ret;
            }
        }
    }

#endif

    return &data;
}

void GFXVertexList::UnMap() {
#ifndef NO_VBO_SUPPORT
    if (GFX_BUFFER_MAP_UNMAP) {
        if (vbo_data) {
            if (display_list) {
                GFXBindElementBuffer(display_list);
                (*glUnmapBufferARB_p)(GL_ELEMENT_ARRAY_BUFFER_ARB);
            }
            GFXBindBuffer(vbo_data);
            (*glUnmapBufferARB_p)(GL_ARRAY_BUFFER_ARB);
            data.colors = NULL;
            data.vertices = NULL;
        }
    }

#endif

}

///Returns the array of vertices to be mutated
union GFXVertexList::VDAT *GFXVertexList::BeginMutate(int offset) {
    return this->Map(false, true);
}

///Ends mutation and refreshes display list
void GFXVertexList::EndMutate(int newvertexsize) {
    this->UnMap();
    if (!(changed & CHANGE_MUTABLE)) {
        changed |= CHANGE_CHANGE;
    }
    if (newvertexsize) {
        numVertices = newvertexsize;
        //Must keep synchronized - we'll only permit changing vertex count on single-list objects
        if (numlists == 1) {
            *offsets = numVertices;
        }
    }
    if (!vbo_data) {
        RenormalizeNormals();
        RefreshDisplayList();
    } else {
        RefreshDisplayList();
    }
    if (changed & CHANGE_CHANGE) {
        changed &= (~CHANGE_CHANGE);
    }
}

//private, only for inheriters
GFXVertexList::GFXVertexList() :
        numVertices(0),
        mode(0),
        unique_mode(0),
        display_list(0),
        vbo_data(0),
        numlists(0),
        offsets(0),
        changed(0) {
    // ctor
}

POLYTYPE *GFXVertexList::GetPolyType() const {
    return mode;
}

int *GFXVertexList::GetOffsets() const {
    return offsets;
}

int GFXVertexList::GetNumLists() const {
    return numlists;
}

///local helper funcs for procedural Modification
void SetVector(const double factor, Vector *pv) {
    pv->i = pv->i * factor;
    pv->j = pv->j * factor;
    pv->k = pv->k * factor;
}

void GFXSphereVertexList::ProceduralModification() {
    GFXVertex *v = sphere->BeginMutate(0)->vertices;
    const int ROWS = 28;
    int row[ROWS];
    for (int i = 0; i < ROWS; i++) {
        row[i] = numVertices / ROWS * i;
    }

    Vector vert[ROWS];
    int direction[ROWS / 2];

    for (int i = 0; i < numVertices; i++) {
        for (int j = 0; j < ROWS; j++) {
            if (row[j] < numVertices / ROWS * (j + 1)) {
                vert[j] = v[row[j]].GetPosition();
            }
        }

        for (int j = 0; j < ROWS / 2; j++) {
            direction[j] = (int) vsrandom.uniformInc(0.0, 5.0);
        }
        if (i % 4 == 1) {
            for (int j = 0; j < ROWS; j += 2) {
                if (direction[j / 2] > 2) {
                    SetVector(1.003, &vert[j]);
                }
            }

        }

        if (i % 4 == 0) {
            for (int j = 1; j < ROWS; j += 2) {
                if (direction[(j - 1) / 2] > 2) {
                    SetVector(1.003, &vert[j]);
                }
            }
        }

        for (int j = 0; j < ROWS; j++) {
            if (row[j] < numVertices / ROWS * (j + 1)) {
                v[row[j]].SetVertex(vert[j]);
            }
        }

        for (int j = 0; j < ROWS; j++) {
            row[j]++;
        }
    }

    sphere->EndMutate( /*numVertices*/ );
}
