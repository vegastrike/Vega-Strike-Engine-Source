/**
* gl_sphere_list_server.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "gfxlib.h"
#include "gfxlib_struct.h"
#include "gfx/matrix.h"
#include <assert.h>

void GFXSphereVertexList::Draw() {}

void GFXSphereVertexList::Draw( enum POLYTYPE *poly, const INDEX index, const int numLists, const int *offsets ) {}
void GFXSphereVertexList::BeginDrawState( GFXBOOL lock ) {}
void GFXSphereVertexList::GetPolys( GFXVertex **vert, int *numPolys, int *numTris )
{
    sphere->GetPolys( vert, numPolys, numTris );
    int numt  = *numTris;
    int numq  = *numPolys-numt;
    int verts = numt*3+numq*4;
    for (int i = 0; i < verts; ++i) {
        (*vert)[i].x *= radius;
        (*vert)[i].y *= radius;
        (*vert)[i].z *= radius;
    }
}
void GFXSphereVertexList::EndDrawState( GFXBOOL lock ) {}
GFXVertexList::VDAT* GFXSphereVertexList::BeginMutate( int offset )
{
    return NULL;
}
void GFXSphereVertexList::EndMutate( int newsize )
{
    assert( 0 );
}
GFXSphereVertexList::GFXSphereVertexList( float radius, int detail, bool Insideout, bool reverse_normals )
{
    this->radius = radius;
    radius = 100000.0f;
    static vector< GFXVertexList* >vlists[4];
    int which = (Insideout ? 1 : 0)+(reverse_normals ? 2 : 0);
    while ( (unsigned int) detail >= vlists[which].size() )
        vlists[which].insert( vlists[which].end(), 1+detail-vlists[which].size(), NULL );
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
        float rho_max   = 3.1415926536;
        float rho_min   = 0;
        float theta_min = 0.0;
        float theta_max = 2*3.1415926536;
        float rho, drho, theta, dtheta;
        float x, y, z;
        float s, t, ds, dt;
        int   i, j, imin, imax;
        float nsign = Insideout ? -1.0 : 1.0;
        float normalscale = reverse_normals ? -1.0 : 1.0;
        int   fir   = 0;       //Insideout?1:0;
        int   sec   = 1;       //Insideout?0:1;
        /* Code below adapted from gluSphere */
        drho   = (rho_max-rho_min)/(float) stacks;
        dtheta = (theta_max-theta_min)/(float) slices;

        ds     = 1.0/slices;
        dt     = 1.0/stacks;
        t = 1.0;                /* because loop now runs from 0 */

        imin   = 0;
        imax   = stacks;

        int  numQuadstrips   = stacks;
        //numQuadstrips = 0;
        int *QSOffsets       = new int[numQuadstrips];

        //draw intermediate stacks as quad strips
        int  numvertex       = stacks*(slices+1)*2;
        GFXVertex     *vertexlist = new GFXVertex[numvertex];

        GFXVertex     *vl    = vertexlist;
        enum POLYTYPE *modes = new enum POLYTYPE[numQuadstrips];
        /*   SetOrientation(Vector(1,0,0),
         *    Vector(0,0,-1),
         *    Vector(0,1,0));//that's the way prop*/                                                                          //taken care of in loading
        for (i = imin; i < imax; i++) {
            GFXVertex *vertexlist = vl+(i*(slices+1)*2);
            rho = i*drho+rho_min;

            s   = 0.0;
            for (j = 0; j <= slices; j++) {
                theta = j*dtheta;                 //(j == slices) ? theta_min * 2 * M_PI : j * dtheta;
                x     = -sin( theta )*sin( rho );
                y     = cos( theta )*sin( rho );
                z     = nsign*cos( rho );

                vertexlist[j*2+fir].i = x*normalscale;
                vertexlist[j*2+fir].k = -y*normalscale;
                vertexlist[j*2+fir].j = z*normalscale;
#define GetS( theta, theta_min, theta_max ) ( 1-(theta-theta_min)/(theta_max-theta_min) )
#define GetT( rho, rho_min, rho_max ) ( 1-(rho-rho_min)/(rho_max-rho_min) )

                vertexlist[j*2+fir].s = GetS( theta, theta_min, theta_max );                 //1-s;//insideout?1-s:s;
                vertexlist[j*2+fir].t = GetT( rho, rho_min, rho_max );                 //t;
                vertexlist[j*2+fir].x = x*radius;
                vertexlist[j*2+fir].z = -y*radius;
                vertexlist[j*2+fir].y = z*radius;

                x = -sin( theta )*sin( rho+drho );
                y = cos( theta )*sin( rho+drho );
                z = nsign*cos( rho+drho );

                vertexlist[j*2+sec].i = x*normalscale;
                vertexlist[j*2+sec].k = -y*normalscale;
                vertexlist[j*2+sec].j = z*normalscale;                 //double negative
                vertexlist[j*2+sec].s = GetS( theta, theta_min, theta_max );                 //1-s;//insideout?1-s:s;
                vertexlist[j*2+sec].t = GetT( rho+drho, rho_min, rho_max );                 //t - dt;
                vertexlist[j*2+sec].x = x*radius;
                vertexlist[j*2+sec].z = -y*radius;
                vertexlist[j*2+sec].y = z*radius;

                s       += ds;
            }

            t           -= dt;
            QSOffsets[i] = (slices+1)*2;
            modes[i]     = GFXQUADSTRIP;
        }
        vlists[which][detail] = new GFXVertexList( modes, numvertex, vertexlist, numQuadstrips, QSOffsets );
    }
    sphere        = vlists[which][detail];
    numVertices   = sphere->numVertices;
    numlists      = sphere->numlists;
    data.vertices = sphere->data.vertices;
    index.i       = sphere->index.i;
    display_list  = sphere->display_list;
    mode = sphere->mode;
    offsets       = sphere->offsets;
}

GFXSphereVertexList::~GFXSphereVertexList()
{
    numVertices   = 0;
    data.vertices = 0;
    data.colors   = 0;
    index.i      = 0;
    mode         = 0;
    numlists     = 0;
    display_list = 0;
    offsets      = 0;
}

