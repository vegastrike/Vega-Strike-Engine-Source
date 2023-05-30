/**
 * ring.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include "vegastrike.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#include "ani_texture.h"

extern int pixelscalesize; //from sphere.cpp

void RingMesh::InitRing(float iradius,
        float oradius,
        int slices,
        const char *texture,
        const QVector &R,
        const QVector &S,
        int wrapx,
        int wrapy,
        const BLENDFUNC a,
        const BLENDFUNC b,
        bool envMapping,
        float theta_min,
        float theta_max,
        FILTER mipmap) {
    int numspheres = slices / 4;
    if (numspheres < 1) {
        numspheres = 1;
    }
    Mesh *oldmesh;
    char ab[3];
    ab[2] = '\0';
    ab[1] = b + '0';
    ab[0] = a + '0';
    hash_name = string("@@Ring") + "#" + texture + "#" + XMLSupport::tostring(slices) + ab + "#"
            + XMLSupport::tostring(theta_min) + "#"
            + XMLSupport::tostring(theta_max);
    if (LoadExistant(hash_name, Vector(iradius, iradius, iradius), 0)) {
        return;
    }
    oldmesh =
            AllocNewMeshesEachInSizeofMeshSpace(numspheres);     //FIXME::RISKY::MIGHT HAVE DIFFERENT SIZES!! DON"T YOU DARE ADD XTRA VARS TO SphereMesh calsshave to!
    numlods = numspheres;
    meshHashTable.Put(hash_name = VSFileSystem::GetSharedMeshHashName(hash_name, Vector(iradius,
            iradius,
            iradius), 0), oldmesh);
    this->orig = oldmesh;
    radialSize = oradius;     //MAKE SURE FRUSTUM CLIPPING IS DONE CORRECTLY!!!!!
    //mn = Vector (radialSize,radialSize,radialSize);
    //mx = Vector (-radialSize,-radialSize,-radialSize);
    mn = Vector(0, 0, 0);
    mx = Vector(0, 0, 0);
    vector<MeshDrawContext> *odq = NULL;
    for (int l = 0; l < numspheres; l++) {
        draw_queue = new vector<MeshDrawContext>[NUM_ZBUF_SEQ + 1];
        if (!odq) {
            odq = draw_queue;
        }
        if (slices > 12) {
            slices -= 4;
        } else {
            slices -= 2;
        }
        float theta, dtheta;
        int i, j, imin, imax;
        vlist = NULL;
        /* Code below adapted from gluSphere */
        dtheta = (theta_max - theta_min) / (GLfloat) slices;
        int numQuadstrips = 2;
        imin = 0;
        imax = numQuadstrips;
        int numvertex = (slices + 1) * 4;
        GFXVertex *vertexlist = new GFXVertex[numvertex];
        GFXVertex *vl = vertexlist;
        enum POLYTYPE *modes = new enum POLYTYPE[numQuadstrips];
        int *QSOffsets = new int[numQuadstrips];
        int fir = 0;
        int sec = 1;
        for (i = imin; i < imax; i++, (fir = 1 - fir), (sec = 1 - sec)) {
            GFXVertex *vertexlist = vl + (i * (slices + 1) * 2);
            for (j = 0; j <= slices; j++) {
                theta = j * dtheta + theta_min;
                QVector unitpos(R.Scale(-sin(theta)) + S.Scale(-cos(theta)));
                QVector up = R.Cross(S) * (1 - (2 * fir));
                vertexlist[j * 2 + fir].i = up.i;
                vertexlist[j * 2 + fir].k = up.j;
                vertexlist[j * 2 + fir].j = up.k;
                vertexlist[j * 2 + fir].s = wrapx * theta / (2 * M_PI);
                vertexlist[j * 2 + fir].t = 0;
                vertexlist[j * 2 + fir].x = unitpos.i * iradius;
                vertexlist[j * 2 + fir].z = unitpos.j * iradius;
                vertexlist[j * 2 + fir].y = unitpos.k * iradius;
                mn = vertexlist[j * 2 + fir].GetVertex().Min(mn);
                mx = vertexlist[j * 2 + fir].GetVertex().Max(mx);
                vertexlist[j * 2 + sec].i = unitpos.i;
                vertexlist[j * 2 + sec].k = unitpos.j;
                vertexlist[j * 2 + sec].j = unitpos.k;
                vertexlist[j * 2 + sec].s = wrapx * theta / (2 * M_PI);
                vertexlist[j * 2 + sec].t = wrapy;
                vertexlist[j * 2 + sec].x = unitpos.i * oradius;
                vertexlist[j * 2 + sec].z = unitpos.j * oradius;
                vertexlist[j * 2 + sec].y = unitpos.k * oradius;
                mn = vertexlist[j * 2 + sec].GetVertex().Min(mn);
                mx = vertexlist[j * 2 + sec].GetVertex().Max(mx);
            }
            modes[i] = GFXQUADSTRIP;
            QSOffsets[i] = (slices + 1) * 2;
        }
        //radialSize = .5*(mx-mn).Magnitude();//+.5*oradius;
        local_pos = (mx + mn) * .5;
        //local_pos.Set(0,0,0);
        vlist = new GFXVertexList(modes, numvertex, vertexlist, numQuadstrips, QSOffsets);
        delete[] vertexlist;
        delete[] modes;
        delete[] QSOffsets;
        SetBlendMode(a, b);
        int texlen = strlen(texture);
        bool found_texture = false;
        if (texlen > 3) {
            if (texture[texlen - 1] == 'i' && texture[texlen - 2] == 'n'
                    && texture[texlen - 3] == 'a' && texture[texlen - 4] == '.') {
                found_texture = true;
                if (Decal.empty()) {
                    Decal.push_back(NULL);
                }
                Decal[0] = new AnimatedTexture(texture, 0, mipmap);
            }
        }
        if (!found_texture) {
            if (Decal.empty()) {
                Decal.push_back(NULL);
            }
            Decal[0] = new Texture(texture,
                    0,
                    mipmap,
                    TEXTURE2D,
                    TEXTURE_2D,
                    g_game.use_planet_textures ? GFXTRUE : GFXFALSE);
        }
        setEnvMap(envMapping);
        Mesh *oldorig = orig;
        refcount = 1;
        orig = NULL;
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
        oldmesh[l] = *this;
        refcount = 0;
        orig = oldorig;
        lodsize = FLT_MAX;
    }
    draw_queue = odq;
}

float RingMesh::clipRadialSize() const {
    return /*mx.Magnitude()*.33+*/ rSize();
}

