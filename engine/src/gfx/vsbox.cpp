/*
 * vsbox.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#include <vega_cast_utils.h>
#include "vsbox.h"
#include "xml_support.h"

#include "preferred_types.h"

using namespace vega_types;

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
    SharedPtr<Box> oldmesh;
    std::string hash_key = std::string("@@Box") + "#" + tostring(corner1) + "#" + tostring(corner2);
    oldmesh = vega_dynamic_cast_shared_ptr<Box>(meshHashTable.Get(hash_key));
    if (oldmesh) {
        *this = *oldmesh;
//        oldmesh->refcount++;
        orig->push_back(oldmesh);
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
    vlist = MakeShared<GFXVertexList>(polys, 18, vertices, 2, offsets);
    //quadstrips[0] = new GFXVertexList(GFXQUADSTRIP,10,vertices);
    delete[] vertices;

    meshHashTable.Put(hash_key, shared_from_this());
    orig->push_back(shared_from_this());
//    refcount++;
    makeDrawQueue(draw_queue);
#undef VERTEX
}

void Box::ProcessDrawQueue(int) {
    SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>> draw_queue_item_0 = draw_queue->at(0);
    if (draw_queue_item_0->empty()) {
        return;
    }
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXColor(0.0, .90, .3, .4);
    GFXDisable(LIGHTING);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(DEPTHWRITE);
    GFXDisable(CULLFACE);

    unsigned vnum = 24 * draw_queue_item_0->size();
    std::vector<float> verts(vnum * (3 + 4));
    auto v = verts.begin();
    while (!draw_queue_item_0->empty()) {
        GFXLoadMatrixModel(draw_queue_item_0->back()->mat);
        draw_queue_item_0->pop_back();
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

