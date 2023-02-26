/*
 * gl_sphere_list.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#include "gfxlib.h"
#include "gfxlib_struct.h"
#include "gfx/matrix.h"
#include "gl_globals.h"
#include <assert.h>

void GFXSphereVertexList::Draw() {
    glEnable(GL_NORMALIZE);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(radius / 100000.0f, radius / 100000.0f, radius / 100000.0f);
    sphere->Draw();
    glPopMatrix();
    glDisable(GL_NORMALIZE);
}

void GFXSphereVertexList::Draw(enum POLYTYPE *poly, const INDEX index, const int numLists, const int *offsets) {
    glEnable(GL_NORMALIZE);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(radius / 100000.0f, radius / 100000.0f, radius / 100000.0f);
    sphere->Draw(poly, index, numLists, offsets);
    glPopMatrix();
    glDisable(GL_NORMALIZE);
}

void GFXSphereVertexList::BeginDrawState(GFXBOOL lock) {
    //

    sphere->BeginDrawState(lock);
}

void GFXSphereVertexList::GetPolys(vega_types::SharedPtr<vega_types::ContiguousSequenceContainer<GFXVertex>> &vert, int *RESTRICT numPolys, int *RESTRICT numTris) {
    sphere->GetPolys(vert, numPolys, numTris);
    int const numt = *numTris;
    int const numq = *numPolys - numt;
    int const verts = numt * 3 + numq * 4;
    for (int i = 0; i < verts; ++i) {
        vert->at(i).x *= radius;
        vert->at(i).y *= radius;
        vert->at(i).z *= radius;
    }
}

void GFXSphereVertexList::EndDrawState(GFXBOOL lock) {
    //
    sphere->EndDrawState(lock);
}

GFXVertexList::VDAT *GFXSphereVertexList::BeginMutate(int offset) {
    return NULL;
}

void GFXSphereVertexList::EndMutate(int newsize) {
    assert(0);
}

GFXSphereVertexList::GFXSphereVertexList(float radius, int detail, bool Insideout, bool reverse_normals) {
    this->radius = radius;
    radius = 100000.0f;
    static vector<GFXVertexList *> vlists[4];
    int which = (Insideout ? 1 : 0) + (reverse_normals ? 2 : 0);
    while (detail >= static_cast<int>(vlists[which].size())) {
        vlists[which].insert(vlists[which].end(), 1 + detail - vlists[which].size(), NULL);
    }
    if (vlists[which][detail] == 0) {
        int slices;
        int stacks = slices = detail;
        if (stacks > 12) {
            stacks -= 4;
            slices -= 4;
        } else {
            stacks -= 2;
            slices -= 2;
        }
        float rho_max = 3.1415926536f;
        float rho_min = 0.0f;
        float theta_min = 0.0f;
        float theta_max = 2.0f * 3.1415926536f;
        float drho, dtheta;
        float x, y, z, tx, ty;
        float s, t, ds, dt;
        int i, j, imin, imax;
        float nsign = Insideout ? -1.0 : 1.0;
        float normalscale = reverse_normals ? -1.0 : 1.0;
        int fir = 0;       //Insideout?1:0;
        int sec = 1;       //Insideout?0:1;
        /* Code below adapted from gluSphere */
        drho = (rho_max - rho_min) / (float) stacks;
        dtheta = (theta_max - theta_min) / (float) slices;

        ds = 1.0f / slices;
        dt = 1.0f / stacks;
        t = 1.0f;               /* because loop now runs from 0 */

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
        /*   SetOrientation(Vector(1,0,0),
         *    Vector(0,0,-1),
         *    Vector(0,1,0));//that's the way prop*/                                                                          //taken care of in loading

        float rhol[2];
        float thetal[2];
#define g_rho(i) ((rhol[((i))&1]))
#define g_theta(i) ((thetal[((i))&1]))

        g_rho(0) = rho_min;
        for (i = imin; i < imax; i++) {
            GFXVertex *vertexlist = vl + (i * (slices + 1) * 2);
            g_rho(i + 1) = (i + 1) * drho + rho_min; //FIXME These macros are horrible

            s = 0.0;
            g_theta(0) = 0;
            for (j = 0; j <= slices; j++) {
                g_theta(j + 1) = (j + 1) * dtheta;

                tx = -sin(g_theta(j));
                ty = cos(g_theta(j));
                z = sin(g_rho(i));
                x = tx * z;
                y = ty * z;
                z = nsign * cos(g_rho(i));

                //normal
                vertexlist[j * 2 + fir].i = x * normalscale;
                vertexlist[j * 2 + fir].k = -y * normalscale;
                vertexlist[j * 2 + fir].j = z * normalscale;

                //tangent
                vertexlist[j * 2 + fir].tx = -ty * normalscale;
                vertexlist[j * 2 + fir].tz = -tx * normalscale;
                vertexlist[j * 2 + fir].ty = 0;
                vertexlist[j * 2 + fir].tw = nsign * normalscale;
#define GetS(theta, theta_min, theta_max) ( 1-(theta-theta_min)/(theta_max-theta_min) )
#define GetT(rho, rho_min, rho_max) ( 1-(rho-rho_min)/(rho_max-rho_min) )

                //texcoords
                vertexlist[j * 2 + fir].s = GetS(g_theta(j), theta_min, theta_max);                 //1-s;
                vertexlist[j * 2 + fir].t = GetT(g_rho(i), rho_min, rho_max);                 //t;

                //position
                vertexlist[j * 2 + fir].x = x * radius;
                vertexlist[j * 2 + fir].z = -y * radius;
                vertexlist[j * 2 + fir].y = z * radius;

                z = sin(g_rho(i + 1));
                x = tx * z;
                y = ty * z;
                z = nsign * cos(g_rho(i + 1));

                //normal
                vertexlist[j * 2 + sec].i = x * normalscale;
                vertexlist[j * 2 + sec].k = -y * normalscale;
                vertexlist[j * 2 + sec].j = z * normalscale;

                //tangent
                vertexlist[j * 2 + sec].tx = -ty * normalscale;
                vertexlist[j * 2 + sec].tz = -tx * normalscale;
                vertexlist[j * 2 + sec].ty = 0;
                vertexlist[j * 2 + sec].tw = nsign * normalscale;

                //texcoords
                vertexlist[j * 2 + sec].s = GetS(g_theta(j), theta_min, theta_max);                 //1-s;
                vertexlist[j * 2 + sec].t = GetT(g_rho(i + 1), rho_min, rho_max);                 //t - dt;

                //position
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

        vlists[which][detail] = new GFXVertexList(modes, numvertex, vertexlist, numQuadstrips, QSOffsets);

        delete[] QSOffsets;
        delete[] vertexlist;
        delete[] modes;
    }
    sphere = vlists[which][detail];
    numVertices = sphere->numVertices;
    numlists = sphere->numlists;
    data.vertices = sphere->data.vertices;
    index.i = sphere->index.i;
    display_list = sphere->display_list;
    vbo_data = sphere->vbo_data;
    mode = sphere->mode;
    offsets = sphere->offsets;
}

GFXSphereVertexList::~GFXSphereVertexList() {
    numVertices = 0;
    data.vertices = 0;
    data.colors = 0;
    index.i = 0;
    mode = 0;
    numlists = 0;
    display_list = 0;
    offsets = 0;
    vbo_data = 0;
}

