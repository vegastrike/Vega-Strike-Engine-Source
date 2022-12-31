/*
 * mesh_poly.cpp
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "mesh.h"
#include "aux_texture.h"
#include "preferred_types.h"
#if !defined (_WIN32) && !defined (__CYGWIN__) && !(defined (__APPLE__) || defined (MACOSX )) && !defined (BSD) && !defined(__HAIKU__)
#include <values.h>
#endif
#include <float.h>
#include <assert.h>

#include <utility>
#include <boost/utility/string_view.hpp>

#define PBEHIND (-1)
#define PFRONT (1)
#define PUNK (0)

#define WHICHSID(v) ( ( (v.x*a+v.y*b+v.z*c+d) > 0 )*2-1 )

using namespace vega_types;

static int whichside(GFXVertex *t, int numvertex, float a, float b, float c, float d) {
    int count = PUNK;
    for (int i = 0; i < numvertex; i++) {
        count += WHICHSID(t[i]);
    }
    if (count == PUNK) {
        count = (rand() % 2) * 2 - 1;
    }
    return ((count > 0) * 2) - 1;
}

void updateMax(Vector &mn, Vector &mx, const GFXVertex &ver);

void Mesh::Fork(SharedPtr<Mesh> &one, SharedPtr<Mesh> &two, float a, float b, float c, float d) {
    if (orig && !orig->empty() && orig->front().get() != this) {
        orig->front()->Fork(one, two, a, b, c, d);
        return;
    }
    int numtris, numquads;
    GFXVertex *Orig;
    vlist->GetPolys(&Orig, &numquads, &numtris);
    numquads -= numtris;
    int numtqx[2] = {0, 0};
    int numtqy[2] = {0, 0};

    GFXVertex *X = new GFXVertex[numquads * 4 + numtris * 3];
    GFXVertex *xnow = X;
    GFXVertex *Y = new GFXVertex[numquads * 4 + numtris * 3];
    GFXVertex *ynow = Y;
    Vector xmax, xmin, ymax, ymin;
    xmax = ymax = Vector(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    xmin = ymin = Vector(FLT_MAX, FLT_MAX, FLT_MAX);

    int i, j;
    int inc = 3;
    int offset = 0;
    int last = numtris;
    mesh_polygon tmppolygon;
    for (int l = 0; l < 2; l++) {
        for (i = 0; i < last; i++) {
            if ((!(numtqy[l] < last / 3
                    && numtqx[l] > 2 * last / 3))
                    && ((numtqx[l] < last / 3
                            && numtqy[l] > 2 * last / 3)
                            || whichside(&Orig[offset + i * inc], inc, a, b, c, d) == PFRONT)) {
                numtqx[l]++;
                for (j = 0; j < inc; j++) {
                    memcpy(xnow, &Orig[offset + i * inc + j], sizeof(GFXVertex));
                    updateMax(xmin, xmax, *xnow);
                    xnow++;
                }
            } else {
                numtqy[l]++;
                for (j = 0; j < inc; j++) {
                    memcpy(ynow, &Orig[offset + i * inc + j], sizeof(GFXVertex));
                    updateMax(ymin, ymax, *ynow);
                    ynow++;
                }
            }
        }
        numtqx[l] *= inc;
        numtqy[l] *= inc;
        inc = 4;
        offset = numtris * 3;
        last = numquads;
    }
    free(Orig);
    enum POLYTYPE polytypes[2] = {GFXTRI, GFXQUAD};
    if ((!(numtqx[0] || numtqx[1])) || (!(numtqy[0] || numtqy[1]))) {
        one = two = nullptr;
        delete[] X;
        delete[] Y;
        return;
    }
    one = MakeShared<Mesh>();
    one->setLighting(getLighting());
    one->setEnvMap(getEnvMap());
    one->forceCullFace(GFXFALSE);

    two = MakeShared<Mesh>();
    two->setLighting(getLighting());
    two->setEnvMap(getEnvMap());

    two->forceCullFace(GFXFALSE);
    one->forcelogos.reset();
    one->squadlogos.reset();
    one->setLighting(getLighting());
    one->setEnvMap(getEnvMap());
    one->blendSrc = two->blendSrc = blendSrc;
    one->blendDst = two->blendDst = blendDst;
    while (one->Decal->size() < Decal->size()) {
        one->Decal->push_back(nullptr);
    }
    {
        for (unsigned int i2 = 0; i2 < Decal->size(); i2++) {
            if (Decal->at(i2)) {
                one->Decal->at(i2) = Decal->at(i2)->Clone();
            }
        }
    }

    two->squadlogos.reset();
    two->forcelogos.reset();
    two->setLighting(getLighting());
    two->setEnvMap(getEnvMap());
    while (two->Decal->size() < Decal->size()) {
        two->Decal->push_back(nullptr);
    }
    {
        for (unsigned int i3 = 0; i3 < Decal->size(); i3++) {
            if (Decal->at(i3)) {
                two->Decal->at(i3) = Decal->at(i3)->Clone();
            }
        }
    }
    if (numtqx[0] && numtqx[1]) {
        one->vlist = vega_types::MakeShared<GFXVertexList>(polytypes, numtqx[0] + numtqx[1], X, 2, numtqx, true);
    } else {
        int exist = 0;
        if (numtqx[1]) {
            exist = 1;
        }
        assert(numtqx[0] || numtqx[1]);
        one->vlist = vega_types::MakeShared<GFXVertexList>(&(polytypes[exist]), numtqx[exist], X, 1, &(numtqx[exist]), true, nullptr);
    }
    if (numtqy[0] || numtqy[1]) {
        two->vlist = vega_types::MakeShared<GFXVertexList>(polytypes, numtqy[0] + numtqy[1], Y, 2, numtqy, true);
    } else {
        int exis = 0;
        if (numtqy[1]) {
            exis = 1;
        }
        assert(numtqx[0] || numtqx[1]);
        two->vlist = vega_types::MakeShared<GFXVertexList>(&polytypes[exis], numtqy[exis], Y, 1, &numtqy[exis], true, nullptr);
    }
    one->local_pos = Vector(.5 * (xmin + xmax));
    two->local_pos = Vector(.5 * (ymin + ymax));
    one->radialSize = .5 * (xmax - xmin).Magnitude();
    two->radialSize = .5 * (ymax - ymin).Magnitude();
    one->mn = xmin;
    one->mx = xmax;
    two->mn = ymin;
    two->mx = ymax;
    one->orig = MakeShared<SequenceContainer<SharedPtr<Mesh>>>(1);
    one->forceCullFace(GFXFALSE);

    two->orig = MakeShared<SequenceContainer<SharedPtr<Mesh>>>(1);
    two->forceCullFace(GFXFALSE);
    one->draw_queue = MakeShared<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>>(NUM_ZBUF_SEQ + 1);
    two->draw_queue = MakeShared<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>>(NUM_ZBUF_SEQ + 1);
    two->orig->push_back(two->shared_from_this());
    one->orig->push_back(one->shared_from_this());
//    x->orig->refcount = 1;
//    y->orig->refcount = 1;
//    x->numforcelogo = 0;
    one->forcelogos = MakeShared<SequenceContainer<SharedPtr<Logo>>>();
//    x->numsquadlogo = 0;
    one->squadlogos = MakeShared<SequenceContainer<SharedPtr<Logo>>>();
//    y->numforcelogo = 0;
    two->forcelogos = MakeShared<SequenceContainer<SharedPtr<Logo>>>();;
//    y->numsquadlogo = 0;
    two->squadlogos = MakeShared<SequenceContainer<SharedPtr<Logo>>>();

    delete[] X;
    delete[] Y;
}

void Mesh::GetPolys(SequenceContainer<mesh_polygon> &polys) {
    int numtris;
    int numquads;
    if (orig && !orig->empty() && orig->front().get() != this) {
        orig->front()->GetPolys(polys);
        return;
    }
    GFXVertex *tmpres;
    Vector vv;
    vlist->GetPolys(&tmpres, &numquads, &numtris);
    numquads -= numtris;
    int i;
    int inc = 3;
    int offset = 0;
    int last = numtris;
    mesh_polygon tmppolygon;
    // Unroll this loop a bit to remove conditional 
    for (i = 0; i < last; i++) {
        polys.push_back(tmppolygon);
        for (int j = 0; j < 3; j++, polys.back().v.push_back(vv)) {
            vv.i = tmpres[offset + i * inc + j].x;                 //+local_pos.i;
            vv.j = tmpres[offset + i * inc + j].y;                 //+local_pos.j;
            vv.k = tmpres[offset + i * inc + j].z;                 //+local_pos.k;
        }
    }
    inc = 4;
    offset = numtris * 3;
    last = numquads;
    for (i = 0; i < last; i++) {
        polys.push_back(tmppolygon);
        for (int j = 1; j < 4; j++, polys.back().v.push_back(vv)) {
            vv.i = tmpres[offset + i * inc + j].x;                     //+local_pos.i;
            vv.j = tmpres[offset + i * inc + j].y;                     //+local_pos.j;
            vv.k = tmpres[offset + i * inc + j].z;                     //+local_pos.k;
        }
    }
    free(tmpres);
}

vega_types::SharedPtr<Mesh>
Mesh::constructMesh(boost::string_view filename, const Vector &scale_x, int faction, Flightgroup *fg, bool is_original,
                    const SequenceContainer<string> &texture_override) {
    Mesh return_value{filename, scale_x, faction, fg, is_original, texture_override};
    bool shared = false;
    return constructMeshPart2(return_value, filename, scale_x, faction, fg, is_original, texture_override, shared);
}

vega_types::SharedPtr<Mesh>
Mesh::constructMeshPart2(Mesh &mesh_in_question, boost::string_view filename, const Vector &scale_x, int faction,
                         Flightgroup *fg, bool is_original, const SequenceContainer<string> &texture_override,
                         bool &shared) {
    vega_types::SharedPtr<vega_types::SequenceContainer<vega_types::SharedPtr<Mesh>>> old_mesh;
    shared = false;
    VSFileSystem::VSFile f;
    VSFileSystem::VSError err = VSFileSystem::Unspecified;
    std::string const filename_string = filename.to_string();
    err = f.OpenReadOnly(filename_string, VSFileSystem::MeshFile);
    if (err > VSFileSystem::Ok) {
        VS_LOG(error, (boost::format("Cannot Open Mesh File %1$s\n") % filename));
//cleanexit=1;
//winsys_exit(1);
        return nullptr;
    }
    shared = (err == VSFileSystem::Shared);

    //LoadXML(filename,scale,faction,fg,orig);
    mesh_in_question.LoadXML(f, scale_x, faction, fg, is_original, texture_override);
    old_mesh = mesh_in_question.orig;
    if (err <= VSFileSystem::Ok) {
        f.Close();
    }
    mesh_in_question.draw_queue = vega_types::MakeShared<vega_types::SequenceContainer<vega_types::SharedPtr<vega_types::SequenceContainer<vega_types::SharedPtr<MeshDrawContext>>>>>(NUM_ZBUF_SEQ + 1);

    if (!mesh_in_question.orig || mesh_in_question.orig->empty() || !mesh_in_question.orig->front()) {
        mesh_in_question.hash_name = shared ? VSFileSystem::GetSharedMeshHashName(filename_string, scale_x, faction) : VSFileSystem::GetHashName(
                filename_string,
                scale_x,
                faction);
        meshHashTable.Put(mesh_in_question.hash_name, old_mesh->front());
        old_mesh->at(0) = mesh_in_question.shared_from_this();
        *(old_mesh->at(0)) = mesh_in_question;
    } else {
        mesh_in_question.orig.reset();
    }
    return mesh_in_question.shared_from_this();
}

