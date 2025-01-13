/*
 * CSopcodecollider.cpp
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
/*
	Copyright (C) 2000 by Jorrit Tyberghein

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
-------------------------------------------------------------------------
*           OPCODE library was written by Pierre Terdiman
*                  ported to CS by Charles Quarra
*                  ported to VS from CS by Ed Sweetman
-------------------------------------------------------------------------
*/

/*
 * Copyright (C) 2020 pyramid3d
 * Copyright (C) 2020-2022 Stephen G. Tuggy
 */


#include "CSopcodecollider.h"
#include "opcodeqsqrt.h"
#include "opcodeqint.h"
#include "vs_logging.h"
// #include "opcodegarray.h"
#define _X 1000

#undef _X

using namespace Opcode;
using namespace VegaStrike;

static vs_vector<csCollisionPair> pairs;

csOPCODECollider::csOPCODECollider(const std::vector<mesh_polygon> &polygons) {
    m_pCollisionModel = nullptr;
    vertholder = nullptr;
    //pairs.IncRef();
    TreeCollider.SetFirstContact(true);
    TreeCollider.SetFullBoxBoxTest(false);
    TreeCollider.SetTemporalCoherence(false);
    opcMeshInt.SetCallback(&MeshCallback, this);
    GeometryInitialize(polygons);
    CollisionFace collFace;
    rCollider.SetFirstContact(true);

}

inline float min3(float a, float b, float c) {
    return (a < b ? (a < c ? a : (c < b ? c : b)) : (b < c ? b : c));
}

inline float max3(float a, float b, float c) {
    return (a > b ? (a > c ? a : (c > b ? c : b)) : (b > c ? b : c));
}

void csOPCODECollider::GeometryInitialize(const std::vector<mesh_polygon> &polygons) {
    OPCODECREATE OPCC;
    unsigned int tri_count = 0;
    std::vector<Vector>::size_type vert_count = 0;
    for (std::vector<mesh_polygon>::size_type i = 0; i < polygons.size(); ++i) {
        vert_count += polygons.at(i).v.size();
    }
    tri_count = vert_count / 3;
    if (tri_count) {
        m_pCollisionModel = new Opcode::Model;
        if (!m_pCollisionModel) {
            return;
        }

        vertholder = new Point[vert_count];

        csBox3 tmp;
        tmp.StartBoundingBox();
        unsigned int last = 0;

        /* Copies the Vector's in mesh_polygon to Point's in vertholder.
        * This sucks but i dont see anyway around it */
        for (std::vector<mesh_polygon>::size_type i = 0; i < polygons.size(); ++i) {
            const mesh_polygon *p = (&polygons.at(i));
            for (std::vector<Vector>::size_type j = 0; j < p->v.size(); ++j) {
                vertholder[last++].Set(p->v[j].i, p->v[j].j, p->v[j].k);
                tmp.AddBoundingVertex(p->v[j]);
            }
        }
        radius = max3(tmp.MaxX() - tmp.MinX(), tmp.MaxY() - tmp.MinY(),
                tmp.MaxZ() - tmp.MinZ());
        opcMeshInt.SetNbTriangles(tri_count);
        opcMeshInt.SetNbVertices(last);

        // Mesh data
        OPCC.mIMesh = &opcMeshInt;
        OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
        /* NoLeaf and quantized creates an optimized, both in organization and
        * memory overhead, tree.*/
        OPCC.mNoLeaf = true;
        OPCC.mQuantized = true;
    } else {
        return;
    }

    //bool status = m_pCollisionModel->Build (OPCC);
    m_pCollisionModel->Build(OPCC);
}

csOPCODECollider::~csOPCODECollider() {

    if (m_pCollisionModel) {
        delete m_pCollisionModel;
        m_pCollisionModel = nullptr;
    }
    //pairs.DecRef();
    delete[] vertholder;
}

void csOPCODECollider::MeshCallback(uint32_t triangle_index,
        VertexPointers &triangle,
        void *user_data) {
    csOPCODECollider *collider = (csOPCODECollider *) user_data;
    Point *vertholder = collider->vertholder;
    int index = 3 * triangle_index;
    triangle.Vertex[0] = &vertholder[index];
    triangle.Vertex[1] = &vertholder[index + 1];
    triangle.Vertex[2] = &vertholder[index + 2];
}

bool csOPCODECollider::rayCollide(const Ray &boltbeam, Vector &norm, float &distance) {
    rCollider.SetHitCallback(&csOPCODECollider::RayCallback);
    rCollider.SetUserData(this);
    rCollider.SetFirstContact(false);
    //rCollider.SetClosestHit(true);
    collFace.mDistance = FLT_MAX;
    bool retval = rCollider.Collide(boltbeam, *m_pCollisionModel);
    rCollider.SetUserData(NULL);
    if (retval) {
        retval = collFace.mDistance != FLT_MAX;
        if (retval) {
            distance = collFace.mDistance;
#ifdef VS_DEBUG
            VS_LOG(debug, (boost::format("Opcode actually reported a hit at %1$f meters!") % distance));
#endif
        } else {
            return true;//FIXME: buggy! this should return FALSE but the math is obviously broken with opcode, so opcode is rarely telling us about intersections
        }
        //FIXME set normal
    }
    return retval;
}

void csOPCODECollider::RayCallback(const CollisionFace &faceHit, void *user_data) {
    csOPCODECollider *collider = (csOPCODECollider *) user_data;
    if (collider) {
        if (collider->collFace.mDistance > faceHit.mDistance) {
            collider->collFace = faceHit;
        }
    }
}

bool csOPCODECollider::Collide(csOPCODECollider &otherCollider,
        const csReversibleTransform *trans1,
        const csReversibleTransform *trans2) {
    csOPCODECollider *col2 = (csOPCODECollider *) &otherCollider;
    ColCache.Model0 = this->m_pCollisionModel;
    ColCache.Model1 = col2->m_pCollisionModel;
    csMatrix3 m1;
    if (trans1) {
        m1 = trans1->GetT2O();
    }
    csMatrix3 m2;
    if (trans2) {
        m2 = trans2->GetT2O();
    }
    csVector3 u;
    Matrix4x4 transform1;
    transform1.m[0][3] = 0;
    transform1.m[1][3] = 0;
    transform1.m[2][3] = 0;
    transform1.m[3][3] = 1;
    Matrix4x4 transform2;
    transform2.m[0][3] = 0;
    transform2.m[1][3] = 0;
    transform2.m[2][3] = 0;
    transform2.m[3][3] = 1;
    u = m1.Row1();
    transform1.m[0][0] = u.x;
    transform1.m[1][0] = u.y;
    transform1.m[2][0] = u.z;
    u = m2.Row1();
    transform2.m[0][0] = u.x;
    transform2.m[1][0] = u.y;
    transform2.m[2][0] = u.z;
    u = m1.Row2();
    transform1.m[0][1] = u.x;
    transform1.m[1][1] = u.y;
    transform1.m[2][1] = u.z;
    u = m2.Row2();
    transform2.m[0][1] = u.x;
    transform2.m[1][1] = u.y;
    transform2.m[2][1] = u.z;
    u = m1.Row3();
    transform1.m[0][2] = u.x;
    transform1.m[1][2] = u.y;
    transform1.m[2][2] = u.z;
    u = m2.Row3();
    transform2.m[0][2] = u.x;
    transform2.m[1][2] = u.y;
    transform2.m[2][2] = u.z;
    if (trans1) {
        u = trans1->GetO2TTranslation();
    } else {
        u.Set(0, 0, 0);
    }
    transform1.m[3][0] = u.x;
    transform1.m[3][1] = u.y;
    transform1.m[3][2] = u.z;

    if (trans2) {
        u = trans2->GetO2TTranslation();
    } else {
        u.Set(0, 0, 0);
    }
    transform2.m[3][0] = u.x;
    transform2.m[3][1] = u.y;
    transform2.m[3][2] = u.z;
    if (TreeCollider.Collide(ColCache, &transform1, &transform2)) {
        bool status = (TreeCollider.GetContactStatus() != FALSE);
        if (status) {
            CopyCollisionPairs(this, col2);
        }
        return status;
    } else {
        return false;
    }
}

void csOPCODECollider::ResetCollisionPairs() {
    pairs.clear();
}

csCollisionPair *csOPCODECollider::GetCollisions() {
    return pairs.data();
}

size_t csOPCODECollider::GetCollisionPairCount() {
    return pairs.size();
}

void csOPCODECollider::SetOneHitOnly(bool on) {
    TreeCollider.SetFirstContact(on);
    rCollider.SetFirstContact(on);
}

Vector csOPCODECollider::getVertex(unsigned int which) const {
    // This function is used to position the damage particles
    if (!vertholder) {
        return Vector(0, 0, 0);
    }
    return Vector(vertholder[which].x, vertholder[which].y, vertholder[which].z);
}

void csOPCODECollider::CopyCollisionPairs(csOPCODECollider *col1,
        csOPCODECollider *col2) {
    if (!col1 || !col2) {
        return;
    }

    unsigned int N_pairs = TreeCollider.GetNbPairs();
    if (N_pairs == 0) {
        return;
    }

    const Pair *colPairs = TreeCollider.GetPairs();
    Point *vertholder0 = col1->vertholder;
    Point *vertholder1 = col2->vertholder;
    int j;
    size_t oldlen = pairs.size();
    pairs.resize(oldlen + N_pairs);

    for (unsigned int i = 0; i < N_pairs; ++i) {
        j = 3 * colPairs[i].id0;
        pairs.at(oldlen).a1 = vertholder0[j];
        pairs.at(oldlen).b1 = vertholder0[j + 1];
        pairs.at(oldlen).c1 = vertholder0[j + 2];
        j = 3 * colPairs[i].id1;
        pairs.at(oldlen).a2 = vertholder1[j];
        pairs.at(oldlen).b2 = vertholder1[j + 1];
        pairs.at(oldlen).c2 = vertholder1[j + 2];
        ++oldlen;
    }

}
