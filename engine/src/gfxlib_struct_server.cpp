/*
 * gfxlib_struct_server.cpp
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

#include "src/gfxlib_struct.h"
#include <stdlib.h>
#include <string.h>

GFXLight::GFXLight(const bool enabled,
        const GFXColor &vect,
        const GFXColor &diffuse,
        const GFXColor &specular,
        const GFXColor &ambient,
        const GFXColor &attenuate,
        const GFXColor &direction,
        float exp,
        float cutoff,
        float size) {
}

void GFXVertexList::RefreshDisplayList() {
}

void GFXVertexList::BeginDrawState(GFXBOOL lock) {
}

void GFXVertexList::EndDrawState(GFXBOOL lock) {
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV) {
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned char *index) {
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned short *index) {
}

void GFXVertexList::Draw(enum POLYTYPE poly, int numV, unsigned int *index) {
}

GFXVertexList::VDAT *GFXVertexList::Map(bool read, bool write) {
    return &data;
}

void GFXVertexList::UnMap() {
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

///Returns the array of vertices to be mutated
union GFXVertexList::VDAT *GFXVertexList::BeginMutate(int offset) {
    return &data;
}

///Ends mutation and refreshes display list
void GFXVertexList::EndMutate(int newvertexsize) {
    if (!(changed & CHANGE_MUTABLE)) {
        changed |= CHANGE_CHANGE;
    }
    RenormalizeNormals();
    RefreshDisplayList();
    if (changed & CHANGE_CHANGE) {
        changed &= (~CHANGE_CHANGE);
    }
    if (newvertexsize) {
        numVertices = newvertexsize;
    }
}

void GFXVertexList::DrawOnce() {
}

void GFXVertexList::Draw() {
}

void GFXVertexList::Draw(enum POLYTYPE *mode, const INDEX index, const int numlists, const int *offsets) {
}

extern GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number, GFXMaterial &material) {
    return GFXFALSE;
}

extern void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material) {
}

///Creates a Display list. 0 is returned if no memory is avail for a display list
extern int /*GFXDRVAPI*/ GFXCreateList() {
    return 0;
}

///Ends the display list call.  Returns false if unsuccessful
extern GFXBOOL /*GFXDRVAPI*/ GFXEndList() {
    return GFXFALSE;
}

///Removes a display list from application memory
extern void /*GFXDRVAPI*/ GFXDeleteList(int list) {
}

GFXVertexList::~GFXVertexList() {
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

void GFXSphereVertexList::ProceduralModification() {
}
