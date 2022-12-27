/*
 * sphere_generic.cpp
 *
 * Copyright (c) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include <math.h>

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
#include "universe.h"

using namespace vega_types;

extern SharedPtr<Texture> createTexture(const char *filename,
                                        int stage = 0,
                                        enum FILTER f1 = MIPMAP,
                                        enum TEXTURE_TARGET t0 = TEXTURE2D,
                                        enum TEXTURE_IMAGE_TARGET t = TEXTURE_2D,
                                        unsigned char c = GFXFALSE,
                                        int i = 65536);
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
extern SharedPtr<AnimatedTexture> createAnimatedTexture(char const *c, int i, enum FILTER f);

using XMLSupport::tostring;
int pixelscalesize = 30;

float SphereMesh::GetT(float rho, float rho_min, float rho_max) const {
    return 1 - (rho - rho_min) / (rho_max - rho_min);
}

float SphereMesh::GetS(float theta, float theta_min, float theta_max) const {
    return 1 - (theta - theta_min) / (theta_max - theta_min);
}

float CityLights::GetT(float rho, float rho_min, float rho_max) const {
    return wrapy * SphereMesh::GetT(rho, rho_min, rho_max);
}

float CityLights::GetS(float theta, float theta_min, float theta_max) const {
    return wrapx * SphereMesh::GetS(theta, theta_min, theta_max);
}

string truncateByPipe(string &input) {
    string::size_type i = input.find("|");
    string ret = input;
    if (i != string::npos) {
        ret = input.substr(0, i);
        input = input.substr(i + 1);
    } else {
        input = "";
    }
    return ret;
}

void SphereMesh::InitSphere(float radius,
        int stacks,
        int slices,
        const char *texture,
        const std::string &technique,
        const char *alpha,
        bool Insideout,
        const BLENDFUNC a,
        const BLENDFUNC b,
        bool envMapping,
        float rho_min,
        float rho_max,
        float theta_min,
        float theta_max,
        FILTER mipmap,
        bool reverse_normals,
        bool subclass) {
    setConvex(true);
    int numspheres = (stacks + slices) / 8;
    if (numspheres < 1) {
        numspheres = 1;
    }
    SharedPtr<SequenceContainer<SharedPtr<Mesh>>> oldmesh;
    char ab[3];
    ab[2] = '\0';
    ab[1] = b + '0';
    ab[0] = a + '0';
    hash_name = string("@@Sphere") + "#" + texture + "#" + technique + "#" + XMLSupport::tostring(stacks) + "#"
            + XMLSupport::tostring(slices) + ab + "#"
            + XMLSupport::tostring(rho_min) + "#" + XMLSupport::tostring(rho_max);
    if (LoadExistant(hash_name, Vector(radius, radius, radius), 0)) {
        return;
    } else {
    }
    this->orig = MakeShared<SequenceContainer<SharedPtr<Mesh>>>(numspheres);
    oldmesh = this->orig;
    numlods = numspheres;
    // Is this correct? -- Stephen G. Tuggy 2022-12-23
    meshHashTable.Put(hash_name, oldmesh->front());
    radialSize = radius; //MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
    mn = Vector(-radialSize, -radialSize, -radialSize);
    mx = Vector(radialSize, radialSize, radialSize);
    SharedPtr<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>> odq{nullptr};
    for (int l = 0; l < numspheres; l++) {
        draw_queue = MakeShared<SequenceContainer<SharedPtr<SequenceContainer<SharedPtr<MeshDrawContext>>>>>(NUM_ZBUF_SEQ + 1);
        if (subclass || rho_max != M_PI || rho_min != 0.0 || theta_min != 0.0 || theta_max != 2 * M_PI) {
            odq = draw_queue;
        }
        vlist = nullptr;
        if (subclass) {
            if (stacks > 12) {
                stacks -= 4;
                slices -= 4;
            } else {
                stacks -= 2;
                slices -= 2;
            }
            float drho = NAN, dtheta = NAN;
            float x = NAN, y = NAN, z = NAN;
            float s = NAN, t = NAN, ds = NAN, dt = NAN;
            int i = 0, j = 0, imin = 0, imax = 0;
            float const nsign = Insideout ? -1.0 : 1.0;
            float const normalscale = reverse_normals ? -1.0 : 1.0;
            int const fir = 0; //Insideout?1:0;
            int const sec = 1; //Insideout?0:1;
            /* Code below adapted from gluSphere */
            drho = (rho_max - rho_min) / (float) stacks;
            dtheta = (theta_max - theta_min) / (float) slices;
            ds = 1.0 / slices;
            dt = 1.0 / stacks;
            t = 1.0;  /* because loop now runs from 0 */
            imin = 0;
            imax = stacks;
            int numQuadstrips = stacks;
            //numQuadstrips = 0;
            int *QSOffsets = new int[numQuadstrips];
            //draw intermediate stacks as quad strips
            int numvertex = stacks * (slices + 1) * 2;
            GFXVertex *vertexlist = new GFXVertex[numvertex];
            GFXVertex *vl = vertexlist;
            enum POLYTYPE *modes = new enum POLYTYPE[numQuadstrips];
            float rhol[2];
            float thetal[2];
#define g_rho(i) (rhol[(i)&1])
#define g_theta(i) (thetal[(i)&1])
            g_rho(0) = rho_min;
            for (i = imin; i < imax; i++) {
                GFXVertex *vertexlist = vl + (i * (slices + 1) * 2);
                g_rho(i + 1) = (i + 1) * drho + rho_min;
                s = 0.0;
                g_theta(0) = 0;
                for (j = 0; j <= slices; j++) {
                    g_theta(j + 1) = (j + 1) * dtheta;
                    x = -sin(g_theta(j)) * sin(g_rho(i));
                    y = cos(g_theta(j)) * sin(g_rho(i));
                    z = nsign * cos(g_rho(i));
                    vertexlist[j * 2 + fir].i = x * normalscale;
                    vertexlist[j * 2 + fir].k = -y * normalscale;
                    vertexlist[j * 2 + fir].j = z * normalscale;
                    vertexlist[j * 2 + fir].s = GetS(g_theta(j), theta_min, theta_max); //1-s;//insideout?1-s:s;
                    vertexlist[j * 2 + fir].t = GetT(g_rho(i), rho_min, rho_max); //t;
                    vertexlist[j * 2 + fir].x = x * radius;
                    vertexlist[j * 2 + fir].z = -y * radius;
                    vertexlist[j * 2 + fir].y = z * radius;
                    x = -sin(g_theta(j)) * sin(g_rho(i + 1));
                    y = cos(g_theta(j)) * sin(g_rho(i + 1));
                    z = nsign * cos(g_rho(i + 1));
                    vertexlist[j * 2 + sec].i = x * normalscale;
                    vertexlist[j * 2 + sec].k = -y * normalscale;
                    vertexlist[j * 2 + sec].j = z * normalscale; //double negative
                    vertexlist[j * 2 + sec].s = GetS(g_theta(j), theta_min, theta_max); //1-s;//insideout?1-s:s;
                    vertexlist[j * 2 + sec].t = GetT(g_rho(i + 1), rho_min, rho_max); //t - dt;
                    vertexlist[j * 2 + sec].x = x * radius;
                    vertexlist[j * 2 + sec].z = -y * radius;
                    vertexlist[j * 2 + sec].y = z * radius;
                    s += ds;
                }
                t -= dt;
                QSOffsets[i] = (slices + 1) * 2;
                modes[i] = GFXQUADSTRIP;
            }
#undef g_rho
#undef g_theta
            vlist = MakeShared<GFXVertexList>(modes, numvertex, vertexlist, numQuadstrips, QSOffsets);
            delete[] vertexlist;
            delete[] modes;
            delete[] QSOffsets;
        } else {
            vlist = MakeShared<GFXSphereVertexList>(radius, stacks > slices ? stacks : slices, Insideout, reverse_normals);
        }
        SetBlendMode(a, b);
        string inputtex = texture;
        unsigned int count = 0;
        if (Decal->empty()) {
            Decal->push_back(nullptr);
        }
        while (inputtex.length()) {
            string thistex = truncateByPipe(inputtex);
            while (Decal->size() <= count) {
                Decal->push_back(nullptr);
            }
            if (thistex.find(".ani") != string::npos) {
                Decal->at(count) = createAnimatedTexture(thistex.c_str(), 0, mipmap);
            } else {
                if (alpha) {
                    Decal->at(count) =
                            createTexture(thistex.c_str(), alpha, 0, mipmap, TEXTURE2D, TEXTURE_2D, 1, 0,
                                    (Insideout || g_game.use_planet_textures) ? GFXTRUE : GFXFALSE);
                } else {
                    Decal->at(count) =
                            createTexture(
                                    thistex.c_str(), 0, mipmap, TEXTURE2D, TEXTURE_2D,
                                    (Insideout || g_game.use_planet_textures) ? GFXTRUE : GFXFALSE);
                }
            }
            count++;
        }
        Insideout ? setEnvMap(GFXFALSE) : setEnvMap(envMapping);
        if (Insideout) {
            draw_sequence = 0;
        }
        SharedPtr<SequenceContainer<SharedPtr<Mesh>>> const oldorig = orig;
//        refcount = 1;
        orig = nullptr;
        if (l >= 1) {
            lodsize = (numspheres + 1 - l) * pixelscalesize;
            if (l == 1) {
                lodsize *= 2;
            } else if (l == 2) {
                lodsize *= 1.75;
            } else if (l == 3) {
                lodsize *= 1.5;
            }
        }
        initTechnique(technique);
        oldmesh->at(l) = shared_from_this();
//        refcount = 0;
        orig = oldorig;
        lodsize = FLT_MAX;
    }
    draw_queue = odq;
}

void SphereMesh::Draw(float lod, bool centered, const Matrix &m) {
    if (centered) {
        Matrix m1(m);
        m1.p = QVector(_Universe->AccessCamera()->GetPosition().Transform(m1));
        Mesh::Draw(lod, m1);
    } else {
        Mesh::Draw(lod, m);
    }
}

void SphereMesh::RestoreCullFace(int whichdrawqueue) {
    //always right
}

float CityLights::wrapx = 1;
float CityLights::wrapy = 1;

CityLights::CityLights(float radius,
        int stacks,
        int slices,
        const char *texture,
        int zzwrapx,
        int zzwrapy,
        bool insideout,
        const BLENDFUNC a,
        const BLENDFUNC b,
        bool envMap,
        float rho_min,
        float rho_max,
        float theta_min,
        float theta_max,
        bool reversed_normals) : SphereMesh() {
    setConvex(true);
    wrapx = zzwrapx;
    wrapy = zzwrapy;
    FILTER filter =
            (FILTER) XMLSupport::parse_int(vs_config->getVariable("graphics", "CityLightFilter",
                    XMLSupport::tostring(((int) TRILINEAR))));
    InitSphere(radius,
            stacks,
            slices,
            texture,
            "",
            NULL,
            insideout,
            a,
            b,
            envMap,
            rho_min,
            rho_max,
            theta_min,
            theta_max,
            filter,
            reversed_normals,
            zzwrapx != 1 || zzwrapy != 1);
}

