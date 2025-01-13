/*
 * vsbox.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include "vsbox.h"
#include "xml_support.h"

/*
 *  inline ostream &operator<<(ostrstream os, const Vector &obj) {
 *  return os << "(" << obj.i << "," << obj.j << "," << obj.k << ")";
 *  }
 */
string tostring(const Vector &v) {
    return std::string("(") + XMLSupport::tostring(v.i) + ", " + XMLSupport::tostring(v.j) + ", "
            + XMLSupport::tostring(v.k) + ")";
}

Box::Box(const Vector &corner1, const Vector &corner2) : corner_min(corner1), corner_max(corner2) {
    InitUnit();
    draw_sequence = 3;
    setEnvMap(GFXFALSE);
    blendSrc = ONE;
    blendDst = ONE;
    Box *oldmesh;
    string hash_key = string("@@Box") + "#" + tostring(corner1) + "#" + tostring(corner2);
    if (0 != (oldmesh = (Box *) meshHashTable.Get(hash_key))) {
        *this = *oldmesh;
        oldmesh->refcount++;
        orig = oldmesh;
        return;
    }
    int a = 0;
    GFXVertex *vertices = new GFXVertex[18];

#define VERTEX(ax, ay, az) \
    do {vertices[a].x = ax;  \
        vertices[a].y = ay;  \
        vertices[a].z = az;  \
        vertices[a].i = ax;  \
        vertices[a].j = ay;  \
        vertices[a].k = az;  \
        vertices[a].s = 0;   \
        vertices[a].t = 0;   \
        a++;                 \
    }                        \
    while (0)

    VERTEX(corner_max.i, corner_min.j, corner_max.k);
    VERTEX(corner_min.i, corner_min.j, corner_max.k);
    VERTEX(corner_min.i, corner_min.j, corner_min.k);
    VERTEX(corner_max.i, corner_min.j, corner_min.k);

    VERTEX(corner_max.i, corner_max.j, corner_min.k);
    VERTEX(corner_min.i, corner_max.j, corner_min.k);
    VERTEX(corner_min.i, corner_max.j, corner_max.k);
    VERTEX(corner_max.i, corner_max.j, corner_max.k);

    a = 8;

    VERTEX(corner_max.i, corner_min.j, corner_max.k);
    VERTEX(corner_min.i, corner_min.j, corner_max.k);
    VERTEX(corner_min.i, corner_max.j, corner_max.k);
    VERTEX(corner_max.i, corner_max.j, corner_max.k);

    VERTEX(corner_max.i, corner_max.j, corner_min.k);
    VERTEX(corner_min.i, corner_max.j, corner_min.k);

    VERTEX(corner_min.i, corner_min.j, corner_min.k);
    VERTEX(corner_max.i, corner_min.j, corner_min.k);

    VERTEX(corner_max.i, corner_min.j, corner_max.k);
    VERTEX(corner_min.i, corner_min.j, corner_max.k);

    int offsets[2];
    offsets[0] = 8;
    offsets[1] = 10;
    enum POLYTYPE polys[2];
    polys[0] = GFXQUAD;
    polys[1] = GFXQUADSTRIP;
    vlist = new GFXVertexList(polys, 18, vertices, 2, offsets);
    //quadstrips[0] = new GFXVertexList(GFXQUADSTRIP,10,vertices);
    delete[] vertices;

    meshHashTable.Put(hash_key, this);
    orig = this;
    refcount++;
    draw_queue = new vector<MeshDrawContext>[NUM_ZBUF_SEQ + 1];
#undef VERTEX
}

void Box::ProcessDrawQueue(int) {
    if (!draw_queue[0].size()) {
        return;
    }
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColor(0.0, .90, .3, .4);
    GFXDisable(LIGHTING);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(DEPTHWRITE);
    GFXDisable(CULLFACE);

    unsigned vnum = 24 * draw_queue[0].size();
    std::vector<float> verts(vnum * (3 + 4));
    std::vector<float>::iterator v = verts.begin();
    while (draw_queue[0].size()) {
        GFXLoadMatrixModel(draw_queue[0].back().mat);
        draw_queue[0].pop_back();
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 1.0;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.7;
        *v++ = 0.0;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_max.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_min.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_min.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
        *v++ = corner_min.i;
        *v++ = corner_max.j;
        *v++ = corner_max.k;
        *v++ = 0.0;
        *v++ = 0.9;
        *v++ = 0.3;
        *v++ = 0.2;
    }
    GFXDraw(GFXQUAD, &verts[0], vnum, 3, 4);
    GFXEnable(DEPTHWRITE);
}

