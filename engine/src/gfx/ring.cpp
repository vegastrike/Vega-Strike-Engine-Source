/*
 * ring.cpp
 *
 * Copyright (c) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike Contributors
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


#include "ring.h"

#include <math.h>
#include <vega_cast_utils.h>
#include "vegastrike.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#include "ani_texture.h"
#include "preferred_types.h"

using namespace vega_types;

extern int pixelscalesize; //from sphere.cpp

float RingMesh::clipRadialSize() const {
    return /*mx.Magnitude()*.33+*/ rSize();
}

vega_types::SharedPtr<RingMesh> RingMesh::constructRingMesh(vega_types::SharedPtr<RingMesh> ring_mesh,
                                                            float iradius,
                                                            float oradius,
                                                            int slices,
                                                            const char *texture,
                                                            const QVector &r,
                                                            const QVector &s,
                                                            int texwrapx,
                                                            int texwrapy,
                                                            const BLENDFUNC a,
                                                            const BLENDFUNC b,
                                                            bool envMap,
                                                            float theta_min,
                                                            float theta_max,
                                                            FILTER mipmap) {
    const std::string hash_name = calculateHashName(texture, slices, a, b, theta_min, theta_max);
    const vega_types::SharedPtr<Mesh> ptr_to_return_value = ring_mesh;
    if (ring_mesh->LoadExistant(hash_name, Vector(iradius, iradius, iradius), 0)) {
        return vega_dynamic_cast_shared_ptr<RingMesh>(ptr_to_return_value);
    } else {
        ring_mesh->orig = MakeShared<SequenceContainer<SharedPtr<Mesh>>>();
        const uint64_t num_levels_of_detail = calculateHowManyLevelsOfDetail(slices);
        ring_mesh->numlods = num_levels_of_detail;
        ring_mesh->radialSize = oradius;     //MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
        ring_mesh->mn = Vector(0, 0, 0);
        ring_mesh->mx = Vector(0, 0, 0);
        SharedPtr<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>> odq{nullptr};
        for (uint64_t l = 0; l < num_levels_of_detail; ++l) {
            ring_mesh->orig->push_back(loadFreshLevelOfDetail(ring_mesh, l, iradius, oradius, slices, texture, r, s, texwrapx, texwrapy, a, b, envMap, theta_min, theta_max, mipmap));
        }
    }
    meshHashTable.Put(VSFileSystem::GetSharedMeshHashName(hash_name, Vector(iradius, iradius, iradius), 0), ptr_to_return_value);
    return vega_dynamic_cast_shared_ptr<RingMesh>(ptr_to_return_value);
}

vega_types::SharedPtr<RingMesh> RingMesh::createRingMesh(float iradius,
                                                         float oradius,
                                                         int slices,
                                                         const char *texture,
                                                         const QVector &r,
                                                         const QVector &s,
                                                         int texwrapx,
                                                         int texwrapy,
                                                         const BLENDFUNC a,
                                                         const BLENDFUNC b,
                                                         bool envMap,
                                                         float theta_min,
                                                         float theta_max,
                                                         FILTER mipmap) {
    vega_types::SharedPtr<RingMesh> return_value = MakeShared<RingMesh>();
    return constructRingMesh(return_value, iradius, oradius, slices, texture, r, s, texwrapx, texwrapy, a, b, envMap, theta_min, theta_max, mipmap);
}

const std::string RingMesh::calculateHashName(const char *texture,
                                         int slices,
                                         const BLENDFUNC a,
                                         const BLENDFUNC b,
                                         float theta_min,
                                         float theta_max) {
    std::array<char, 3> ab{};
    ab.at(2) = '\0';
    ab.at(1) = b + '0';
    ab.at(0) = a + '0';
    std::string return_value = (boost::format("@@Ring#%1%#%2%%3%#%4%#%5%") % texture % slices % ab.data() % theta_min % theta_max).str();
    return return_value;
}

const uint64_t RingMesh::calculateHowManyLevelsOfDetail(int slices) {
    int return_value = slices / 4;
    if (return_value < 1) {
        return_value = 1;
    }
    return return_value;
}

SharedPtr<Mesh> RingMesh::loadFreshLevelOfDetail(vega_types::SharedPtr<RingMesh> ring_mesh,
                                      uint64_t l,
                                      float iradius,
                                      float oradius,
                                      int slices,
                                      const char *texture,
                                      const QVector &r,
                                      const QVector &s,
                                      int texwrapx,
                                      int texwrapy,
                                      const BLENDFUNC a,
                                      const BLENDFUNC b,
                                      bool env_map,
                                      float theta_min,
                                      float theta_max,
                                      FILTER mipmap) {
    int num_spheres = slices / 4;
    if (num_spheres < 1) {
        num_spheres = 1;
    }
    SharedPtr<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>> odq{nullptr};
    ring_mesh->draw_queue = MakeShared<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>>();
    for (uint32_t i = 0; i < NUM_ZBUF_SEQ + 1; ++i) {
        ring_mesh->draw_queue->at(i) = MakeShared<SequenceContainer<SharedPtr<MeshDrawContext>>>();
    }
    odq = ring_mesh->draw_queue;
    if (slices > 12) {
        slices -= 4;
    } else {
        slices -= 2;
    }
    float theta = NAN;
    float dtheta = NAN;
    int i = 0;
    int j = 0;
    int imin = 0;
    int imax = 0;
    ring_mesh->vlist.reset();
    // Code below adapted from gluSphere
    dtheta = (theta_max - theta_min) / static_cast<GLfloat>(slices);
    const int num_quad_strips = 2;
    imin = 0;
    imax = num_quad_strips;
    const int num_vertex = (slices + 1) * 4;
    vega_types::ContiguousSequenceContainer<GFXVertex> vertices{};
    vertices.reserve(num_vertex);
    for (int n = 0; n < num_vertex; ++n) {
        vertices.push_back({});
    }
    vega_types::ContiguousSequenceContainer<POLYTYPE> modes{};
    modes.reserve(num_quad_strips);
    for (int n = 0; n < num_quad_strips; ++n) {
        modes.push_back({});
    }
    vega_types::ContiguousSequenceContainer<int> qs_offsets{};
    qs_offsets.reserve(num_quad_strips);
    for (int n = 0; n < num_quad_strips; ++n) {
        qs_offsets.push_back({});
    }
    int fir = 0;
    int sec = 1;
    for (i = imin; i < imax; i++, (fir = 1 - fir), (sec = 1 - sec)) {
        int vertex_index = i * (slices + 1) * 2;
        for (j = 0; j <= slices; ++j) {
            theta = j * dtheta + theta_min;
            QVector unitpos(r.Scale(-sin(theta)) + s.Scale(-cos(theta)));
            QVector up = r.Cross(s) * (1 - (2 * fir));
            vertices.at(vertex_index + j * 2 + fir).i = up.i;
            vertices.at(vertex_index + j * 2 + fir).j = up.j;
            vertices.at(vertex_index + j * 2 + fir).k = up.k;
            vertices.at(vertex_index + j * 2 + fir).s = texwrapx * theta / (2 * M_PI);
            vertices.at(vertex_index + j * 2 + fir).t = 0;
            vertices.at(vertex_index + j * 2 + fir).x = unitpos.i * iradius;
            vertices.at(vertex_index + j * 2 + fir).z = unitpos.j * iradius;
            vertices.at(vertex_index + j * 2 + fir).y = unitpos.k * iradius;
            ring_mesh->mn = vertices.at(vertex_index + j * 2 + fir).GetVertex().Min(ring_mesh->mn);
            ring_mesh->mx = vertices.at(vertex_index + j * 2 + fir).GetVertex().Max(ring_mesh->mx);
            vertices.at(vertex_index + j * 2 + sec).i = unitpos.i;
            vertices.at(vertex_index + j * 2 + sec).k = unitpos.j;
            vertices.at(vertex_index + j * 2 + sec).j = unitpos.k;
            vertices.at(vertex_index + j * 2 + sec).s = texwrapx * theta / (2 * M_PI);
            vertices.at(vertex_index + j * 2 + sec).t = texwrapy;
            vertices.at(vertex_index + j * 2 + sec).x = unitpos.i * oradius;
            vertices.at(vertex_index + j * 2 + sec).z = unitpos.j * oradius;
            vertices.at(vertex_index + j * 2 + sec).y = unitpos.k * oradius;
            ring_mesh->mn = vertices.at(vertex_index + j * 2 + sec).GetVertex().Min(ring_mesh->mn);
            ring_mesh->mx = vertices.at(vertex_index + j * 2 + sec).GetVertex().Max(ring_mesh->mx);
        }
        modes.at(i) = GFXQUADSTRIP;
        qs_offsets.at(i) = (slices + 1) * 2;
    }
    ring_mesh->local_pos = (ring_mesh->mx + ring_mesh->mn) * 0.5;
    ring_mesh->vlist = MakeShared<GFXVertexList>(modes.data(), num_vertex, vertices.data(), num_quad_strips, qs_offsets.data());
    vertices.clear();
    modes.clear();
    qs_offsets.clear();
    ring_mesh->SetBlendMode(a, b);
    int texlen = strlen(texture);
    bool found_texture = false;
    if (texlen > 3) {
        if (texture[texlen - 1] == 'i' && texture[texlen - 2] == 'n' && texture[texlen - 3] == 'a' && texture[texlen - 4] == '.') {
            found_texture = true;
            if (ring_mesh->Decal->empty()) {
                ring_mesh->Decal->push_back(nullptr);
            }
            ring_mesh->Decal->front() = AnimatedTexture::createAnimatedTexture(texture, 0, mipmap);
        }
    }
    if (!found_texture) {
        if (ring_mesh->Decal->empty()) {
            ring_mesh->Decal->push_back(nullptr);
        }
        ring_mesh->Decal->front() = Texture::createTexture(texture, 0, mipmap, TEXTURE2D, TEXTURE_2D, g_game.use_planet_textures ? GFXTRUE : GFXFALSE);
    }
    ring_mesh->setEnvMap(env_map);
    SharedPtr<SequenceContainer<SharedPtr<Mesh>>> oldorig = ring_mesh->orig;
    ring_mesh->orig.reset();
    if (l >= 1) {
        ring_mesh->lodsize = (num_spheres + 1 - l) * pixelscalesize;
        if (l == 1) {
            ring_mesh->lodsize *= 2;
        } else if (l == 2) {
            ring_mesh->lodsize *= 1.75;
        } else if (l == 3) {
            ring_mesh->lodsize *= 1.5;
        }
    }
    ring_mesh->orig = oldorig;
    ring_mesh->lodsize = FLT_MAX;

    ring_mesh->draw_queue = odq;

    return ring_mesh;
}

