/*
 * sphere.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include "vega_cast_utils.h"
#include "sphere.h"
#include "ani_texture.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#ifndef M_PI
#define M_PI (3.1415926536F)
#endif
#include "gfx/camera.h"
#include "preferred_types.h"

using XMLSupport::tostring;
using namespace vega_types;

void SphereMesh::ProcessDrawQueue(size_t whichpass, int which, bool zsort, const QVector &sortctr) {
    static SharedPtr<GFXColor> spherecol = MakeShared<GFXColor>(vs_config->getColor("planet_ambient"));
    SharedPtr<GFXColor> const tmpcol = MakeShared<GFXColor>(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(spherecol);
    Mesh::ProcessDrawQueue(whichpass, which, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

void SphereMesh::SelectCullFace(int whichdrawqueue) {
    GFXEnable(CULLFACE);
}

vega_types::SharedPtr<SphereMesh>
SphereMesh::createSphereMesh(float radius, int stacks, int slices, const char *texture, const string &technique,
                             const char *alpha, bool inside_out, const BLENDFUNC a, const BLENDFUNC b, bool env_map,
                             float rho_min, float rho_max, float theta_min, float theta_max, FILTER mipmap,
                             bool reverse_normals) {
//    SphereMesh return_value;
    SharedPtr<SphereMesh> return_value = MakeShared<SphereMesh>();
    return constructSphereMesh(return_value,
                               radius,
                               stacks,
                               slices,
                               texture,
                               technique,
                               alpha,
                               inside_out,
                               a,
                               b,
                               env_map,
                               rho_min,
                               rho_max,
                               theta_min,
                               theta_max,
                               mipmap,
                               reverse_normals,
                               false);
}

std::string const
SphereMesh::calculateHashName(const char *texture, const std::string &technique, int stacks, int slices, const BLENDFUNC a,
                              const BLENDFUNC b, float rho_min, float rho_max) {
    std::array<char, 3> ab{};
    ab.at(2) = '\0';
    ab.at(1) = b + '0';
    ab.at(0) = a + '0';
    std::string return_value = (boost::format("@@Sphere#%1%#%2%#%3%#%4%%5%#%6%#%7%") % texture % technique % stacks % slices % ab.data() % rho_min % rho_max).str();
    return return_value;
}

uint64_t const SphereMesh::calculateHowManyLevelsOfDetail(int stacks, int slices) {
    int return_value = (stacks + slices) / 8;
    if (return_value < 1) {
        return_value = 1;
    }
    return return_value;
}

extern SharedPtr<AnimatedTexture> createAnimatedTexture(char const *c, int i, enum FILTER f);

extern SharedPtr<Texture> createTexture(char const *ccc,
                                        char const *cc,
                                        int k = 0,
                                        enum FILTER f1 = MIPMAP,
                                        enum TEXTURE_TARGET t0 = TEXTURE2D,
                                        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
                                        float f = 1,
                                        int j = 0,
                                        unsigned char c = GFXFALSE,
                                        int i = 65536);

extern SharedPtr<Texture> createTexture(const char *filename,
                                        int stage = 0,
                                        enum FILTER f1 = MIPMAP,
                                        enum TEXTURE_TARGET t0 = TEXTURE2D,
                                        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
                                        unsigned char c = GFXFALSE,
                                        int i = 65536);

extern int pixelscalesize;

SharedPtr<Mesh> SphereMesh::loadFreshLevelOfDetail(SharedPtr<SphereMesh> mesh,
                                                   uint64_t l,
                                                   float radius,
                                                   int &stacks,
                                                   int &slices,
                                                   const char *texture,
                                                   const string &technique,
                                                   const char *alpha,
                                                   bool inside_out,
                                                   const BLENDFUNC a,
                                                   const BLENDFUNC b,
                                                   bool env_map,
                                                   float rho_min,
                                                   float rho_max,
                                                   float theta_min,
                                                   float theta_max,
                                                   FILTER mipmap,
                                                   bool reverse_normals,
                                                   bool subclass) {
    SharedPtr<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>> odq{nullptr};
    mesh->draw_queue = MakeShared<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>>();
    odq = mesh->draw_queue;
    mesh->vlist = nullptr;
    if (subclass) {
        if (stacks > 12) {
            stacks -= 4;
            slices -= 4;
        } else {
            stacks -= 2;
            slices -= 2;
        }
        float drho = NAN;
        float dtheta = NAN;
        float x = NAN;
        float y = NAN;
        float z = NAN;
        float s = NAN;
        float t = NAN;
        float ds = NAN;
        float dt = NAN;
        int i = 0;
        int j = 0;
        int imin = 0;
        int imax = 0;
        float const nsign = inside_out ? -1.0F : 1.0F;
        float const normal_scale = reverse_normals ? -1.0F : 1.0F;
        int const fir = 0;
        int const sec = 1;
        // Code below adapted from gluSphere
        drho = (rho_max - rho_min) / static_cast<float>(stacks);
        dtheta = (theta_max - theta_min) / static_cast<float>(slices);
        ds = 1.0F / slices;
        dt = 1.0F / stacks;
        t = 1.0F;   // because loop now runs from 0
        imin = 0;
        imax = stacks;
        int num_quad_strips = stacks;
        vega_types::ContiguousSequenceContainer<int> qs_offsets{};
        qs_offsets.reserve(num_quad_strips);
        for (int n = 0; n < num_quad_strips; ++n) {
            qs_offsets.push_back({});
        }
        // draw intermediate stacks as quad strips
        int num_vertex = stacks * (slices + 1) * 2;
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
        std::array<float, 2> rhol{};
        std::array<float, 2> thetal{};
        rhol.at(0) = rho_min;
        for (i = imin; i < imax; ++i) {
            int vertex_index = i * (slices + 1) * 2;
            rhol.at((i + 1) & 1) = (i + 1) * drho + rho_min;
            s = 0.0F;
            thetal.at(0) = 0.0F;
            for (j = 0; j <= slices; ++j) {
                thetal.at((j + 1) & 1) = (j + 1) * dtheta;
                x = -sin(thetal.at(j & 1)) * sin(rhol.at(i & 1));
                y = cos(thetal.at(j & 1)) * sin(rhol.at(i & 1));
                z = nsign * cos(rhol.at(i & 1));
                vertices.at(vertex_index + j * 2 + fir).i = x * normal_scale;
                vertices.at(vertex_index + j * 2 + fir).k = -y * normal_scale;
                vertices.at(vertex_index + j * 2 + fir).j = z * normal_scale;
                vertices.at(vertex_index + j * 2 + fir).s = mesh->GetS(thetal.at(j & 1), theta_min, theta_max);
                vertices.at(vertex_index + j * 2 + fir).t = mesh->GetT(rhol.at(i & 1), rho_min, rho_max);
                vertices.at(vertex_index + j * 2 + fir).x = x * radius;
                vertices.at(vertex_index + j * 2 + fir).z = -y * radius;
                vertices.at(vertex_index + j * 2 + fir).y = z * radius;
                x = -sin(thetal.at(j & 1)) * sin(rhol.at((i + 1) & 1));
                y = cos(thetal.at(j & 1)) * sin(rhol.at((i + 1) & 1));
                z = nsign * cos(rhol.at((i + 1) & 1));
                vertices.at(vertex_index + j * 2 + sec).i = x * normal_scale;
                vertices.at(vertex_index + j * 2 + sec).k = -y * normal_scale;
                vertices.at(vertex_index + j * 2 + sec).j = z * normal_scale;   // double negative
                vertices.at(vertex_index + j * 2 + sec).s = mesh->GetS(thetal.at(j & 1), theta_min, theta_max);
                vertices.at(vertex_index + j * 2 + sec).t = mesh->GetT(rhol.at((i + 1) & 1), rho_min, rho_max);
                vertices.at(vertex_index + j * 2 + sec).x = x * radius;
                vertices.at(vertex_index + j * 2 + sec).z = -y * radius;
                vertices.at(vertex_index + j * 2 + sec).y = z * radius;
                s += ds;
            }
            t -= dt;
            qs_offsets.at(i) = (slices + 1) * 2;
            modes.at(i) = GFXQUADSTRIP;
        }
        mesh->vlist = MakeShared<GFXVertexList>(modes.data(), num_vertex, vertices.data(), num_quad_strips, qs_offsets.data());
    } else {
        mesh->vlist = MakeShared<GFXSphereVertexList>(radius, stacks > slices ? stacks : slices, inside_out, reverse_normals);
    }

    mesh->SetBlendMode(a, b);
    std::string input_texture = texture;
    uint64_t count = 0;
    if (mesh->Decal->empty()) {
        mesh->Decal->push_back(nullptr);
    }
    while (!input_texture.empty()) {
        std::string this_texture = truncateByPipe(input_texture);
        while (mesh->Decal->size() <= count) {
            mesh->Decal->push_back(nullptr);
        }
        if (this_texture.find(".ani") != string::npos) {
            mesh->Decal->at(count) = createAnimatedTexture(this_texture.c_str(), 0, mipmap);
        } else {
            if (alpha) {
                mesh->Decal->at(count) = createTexture(this_texture.c_str(), alpha, 0, mipmap, TEXTURE2D, TEXTURE_2D, 1, 0, (inside_out || g_game.use_planet_textures) ? GFXTRUE : GFXFALSE);
            } else {
                mesh->Decal->at(count) = createTexture(this_texture.c_str(), 0, mipmap, TEXTURE2D, TEXTURE_2D, (inside_out || g_game.use_planet_textures) ? GFXTRUE : GFXFALSE);
            }
        }
        ++count;
    }
    if (inside_out) {
        mesh->setEnvMap(GFXFALSE);
        mesh->draw_sequence = 0;
    } else {
        mesh->setEnvMap(env_map);
    }

    SharedPtr<SequenceContainer<SharedPtr<Mesh>>> const old_orig = mesh->orig;
    mesh->orig.reset();
    if (l >= 1) {
        mesh->lodsize = (mesh->numlods + 1 - l) * pixelscalesize;
        if (l == 1) {
            mesh->lodsize *= 2.0F;
        } else if (l == 2) {
            mesh->lodsize *= 1.75F;
        } else if (l == 3) {
            mesh->lodsize *= 1.5F;
        }
    }
    mesh->initTechnique(technique);
    mesh->orig = old_orig;
    mesh->lodsize = FLT_MAX;

    mesh->draw_queue = odq;

    return mesh;
}

void CityLights::ProcessDrawQueue(size_t whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr) {
    SharedPtr<GFXColor> const citycol = MakeShared<GFXColor>(1, 1, 1, 1);
    SharedPtr<GFXColor> const tmpcol = MakeShared<GFXColor>(0, 0, 0, 1);
    GFXGetLightContextAmbient(tmpcol);
    GFXLightContextAmbient(citycol);
    Mesh::ProcessDrawQueue(whichpass, whichdrawqueue, zsort, sortctr);
    GFXLightContextAmbient(tmpcol);
    GFXPolygonOffset(0, 0);
}

