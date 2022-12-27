/*
 * mesh_bin.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include "mesh.h"
#include <stdio.h>
#include "vegastrike.h"
#include "vs_globals.h"
#include "file_main.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "vs_exit.h"

using namespace vega_types;
using namespace VSFileSystem;

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

extern SharedPtr<Logo> createLogo(int numberlogos,
                                  Vector *center,
                                  Vector *normal,
                                  float *sizes,
                                  float *rotations,
                                  float offset,
                                  SharedPtr<Texture> Dec,
                                  Vector *Ref);

#ifdef __cplusplus
extern "C"
{
void winsys_exit(int code);
}
#endif

void Mesh::LoadBinary(const char *filename, int faction) {
    GFXBOOL objtex;
    int TexNameLength;
    char *TexName = NULL;
    int NumPoints;
    float *vb;
    int NumTris;
    int *Tris;
    int NumQuads;
    int *Quads;
    GFXBOOL AlphaMap = GFXFALSE;

    VSFile fp;
    VSError err = fp.OpenReadOnly(filename, MeshFile);
    if (err > Ok) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Failed to load file %1$s") % filename));
        VSExit(1);
    }
    TexNameLength = readi(fp);
    if (TexNameLength) {
        if (TexNameLength < 0) {
            AlphaMap = GFXTRUE;
            TexNameLength = -TexNameLength;
        }
        objtex = GFXTRUE;
        TexName = new char[TexNameLength + 5];
        fp.Read(TexName, TexNameLength);
        TexName[TexNameLength + 4] = '\0';
        TexName[TexNameLength + 0] = '.';
        TexName[TexNameLength + 1] = 'b';
        TexName[TexNameLength + 2] = 'm';
        TexName[TexNameLength + 3] = 'p';
    } else {
        objtex = GFXFALSE;
    }
    NumPoints = readi(fp);
    vb = new float[NumPoints * 6];
    int ii, jj, kk;

#define x(i) (vb[i*6])
#define y(i) (vb[i*6+1])
#define z(i) (vb[i*6+2])
#define i(i) (vb[i*6+3])
#define j(i) (vb[i*6+4])
#define k(i) (vb[i*6+5])

    readf(fp, vb, NumPoints * 6);
    for (ii = jj = 0; jj < NumPoints; ii += 6, jj++) {
        //x,y,z,i,j,k
        if (vb[ii] > mx.i) {
            mx.i = vb[ii];
        }
        if (vb[ii] < mn.i) {
            mn.i = vb[ii];
        }
        if (vb[ii + 1] > mx.j) {
            mx.j = vb[ii + 1];
        }
        if (vb[ii + 1] < mn.j) {
            mn.j = vb[ii + 1];
        }
        if (vb[ii + 2] > mx.k) {
            mx.k = vb[ii + 2];
        }
        if (vb[ii + 2] < mn.k) {
            mn.k = vb[ii + 2];
        }
        vb[ii + 3] = -vb[ii + 3];
        vb[ii + 4] = -vb[ii + 4];
        vb[ii + 5] = -vb[ii + 5];
    }
    /*for (ii=0; ii<NumPoints; ii++)
     *  {
     *
     *       x[ii] = readf (fp);
     *       if (x[ii]>mx.i)
     *         mx.i = x[ii];
     *       if (x[ii]<mn.i)
     *         mn.i=x[ii];
     *       y[ii] = readf (fp);
     *
     *       if (y[ii]>mx.j)
     *         mx.j = y[ii];
     *       if (y[ii]<mn.j)
     *         mn.j=y[ii];
     *       z[ii] = readf (fp);
     *
     *       if (z[ii]>mx.k)
     *         mx.k = z[ii];
     *       if (z[ii]<mn.k)
     *         mn.k=z[ii];
     *       i[ii] = -readf (fp);
     *       j[ii] = -readf (fp);
     *       k[ii] = -readf (fp);
     *
     *  }*/
    //below, the square fo teh radial size, because sqrtf will be useless l8r
    radialSize = .5 * (mx - mn).Magnitude();
    NumTris = readi(fp);
    Tris = new int[NumTris * 3];

    readi(fp, Tris, NumTris * 3);
    /*for (ii=0; ii< NumTris;ii++)
     *       for (int jj=0; jj<3; jj++)
     *               Tris[ii*3+jj] = readi(fp);*/
    NumQuads = readi(fp);
    Quads = new int[NumQuads * 4];
    readi(fp, Quads, NumQuads * 4);
    /*for (ii=0; ii< NumQuads;ii++)
     *       for (int jj=0; jj<4; jj++)
     *               Quads[ii*4+jj] = readi(fp);*/

    //int numtrivertex = NumTris*3;
    //int numquadvertex = NumQuads*4;
    int numvertex = NumTris * 3 + NumQuads * 4;
    GFXVertex *vertexlist;
    //GFXVertex *alphalist;

    vertexlist = new GFXVertex[numvertex];

    jj = 0;
    for (ii = kk = 0; ii < NumTris; ii++, kk += 3) {
        vertexlist[jj].x = x(Tris[kk + 0]);
        vertexlist[jj].y = y(Tris[kk + 0]);
        vertexlist[jj].z = z(Tris[kk + 0]);
        vertexlist[jj].i = i(Tris[kk + 0]);
        vertexlist[jj].j = j(Tris[kk + 0]);
        vertexlist[jj].k = k(Tris[kk + 0]);
        jj++;
        vertexlist[jj].x = x(Tris[kk + 1]);
        vertexlist[jj].y = y(Tris[kk + 1]);
        vertexlist[jj].z = z(Tris[kk + 1]);
        vertexlist[jj].i = i(Tris[kk + 1]);
        vertexlist[jj].j = j(Tris[kk + 1]);
        vertexlist[jj].k = k(Tris[kk + 1]);
        jj++;
        vertexlist[jj].x = x(Tris[kk + 2]);
        vertexlist[jj].y = y(Tris[kk + 2]);
        vertexlist[jj].z = z(Tris[kk + 2]);
        vertexlist[jj].i = i(Tris[kk + 2]);
        vertexlist[jj].j = j(Tris[kk + 2]);
        vertexlist[jj].k = k(Tris[kk + 2]);
        jj++;
    }
    for (ii = kk = 0; ii < NumQuads; ii++, kk += 4) {
        vertexlist[jj].x = x(Quads[kk + 0]);
        vertexlist[jj].y = y(Quads[kk + 0]);
        vertexlist[jj].z = z(Quads[kk + 0]);
        vertexlist[jj].i = i(Quads[kk + 0]);
        vertexlist[jj].j = j(Quads[kk + 0]);
        vertexlist[jj].k = k(Quads[kk + 0]);
        jj++;
        vertexlist[jj].x = x(Quads[kk + 1]);
        vertexlist[jj].y = y(Quads[kk + 1]);
        vertexlist[jj].z = z(Quads[kk + 1]);
        vertexlist[jj].i = i(Quads[kk + 1]);
        vertexlist[jj].j = j(Quads[kk + 1]);
        vertexlist[jj].k = k(Quads[kk + 1]);
        jj++;
        vertexlist[jj].x = x(Quads[kk + 2]);
        vertexlist[jj].y = y(Quads[kk + 2]);
        vertexlist[jj].z = z(Quads[kk + 2]);
        vertexlist[jj].i = i(Quads[kk + 2]);
        vertexlist[jj].j = j(Quads[kk + 2]);
        vertexlist[jj].k = k(Quads[kk + 2]);
        jj++;
        vertexlist[jj].x = x(Quads[kk + 3]);
        vertexlist[jj].y = y(Quads[kk + 3]);
        vertexlist[jj].z = z(Quads[kk + 3]);
        vertexlist[jj].i = i(Quads[kk + 3]);
        vertexlist[jj].j = j(Quads[kk + 3]);
        vertexlist[jj].k = k(Quads[kk + 3]);
        jj++;
    }
#undef x
#undef y
#undef z
#undef i
#undef j
#undef k
    delete[] vb;
    if (objtex) {
        //int temp = NumTris*3;
        //float oo256 = .00390625;
        /*long pos =*/ fp.GetPosition();
        {
            int temp = (NumTris * 3 + NumTris * 3 + NumQuads * 4) * 2;
            float *b = new float[temp];
            readf(fp, b, temp);
            for (ii = jj = 0; ii < temp; ii++, jj += 2) {
                vertexlist[ii].s = b[jj], vertexlist[ii].t = b[jj + 1];
            }
            delete[] b;
        }
        /*for (ii=0; ii< temp; ii++)
         *  {
         *       vertexlist[ii].s = readf(fp);// *oo256;
         *       vertexlist[ii].t = readf (fp);// *oo256;
         *  }
         *
         *  temp = NumTris*3+NumQuads*4;
         *  for (; ii<temp; ii++)
         *  {
         *       vertexlist[ii].s = readf(fp);// *oo256;
         *       vertexlist[ii].t = readf(fp);// *oo256;
         *  }*/
        if (AlphaMap) {
            if (Decal->empty()) {
                Decal->push_back(nullptr);
            }
            Decal->front() = createTexture(TexName, nullptr);
        } else {
            if (Decal->empty()) {
                Decal->push_back(nullptr);
            }
            Decal->front() = createTexture(TexName, nullptr);
        }
        if (!Decal->front()) {
            objtex = GFXFALSE;
        }
    }
    int numforcelogo = readi(fp);
    Vector *PolyNormal = new Vector[numforcelogo];
    Vector *center = new Vector[numforcelogo];
    float *sizes = new float[numforcelogo];
    float *rotations = new float[numforcelogo];
    float *offset = new float[numforcelogo];
    char polytype;
    int offst; //FIXME
    Vector *Ref;
    Ref = new Vector[numforcelogo];
    for (ii = 0; ii < numforcelogo; ii++) {
        Ref[ii] = Vector(0, 0, 0);
        center[ii].i = readf(fp);
        center[ii].j = readf(fp);
        center[ii].k = readf(fp);
        polytype = readc(fp);
        switch (polytype) {
            case 'T':
            case 't':
                offst = 3 * readi(fp);
                break;
            case 'D':
            case 'A':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst + 1].x - vertexlist[offst].x;
                Ref[ii].j = vertexlist[offst + 1].y - vertexlist[offst].y;
                Ref[ii].k = vertexlist[offst + 1].z - vertexlist[offst].z;
                break;
            case 'E':
            case 'B':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst + 2].x - vertexlist[offst + 1].x;
                Ref[ii].j = vertexlist[offst + 2].y - vertexlist[offst + 1].y;
                Ref[ii].k = vertexlist[offst + 2].z - vertexlist[offst + 1].z;
                break;
            case 'F':
            case 'C':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst].x - vertexlist[offst + 2].x;
                Ref[ii].j = vertexlist[offst].y - vertexlist[offst + 2].y;
                Ref[ii].k = vertexlist[offst].z - vertexlist[offst + 2].z;
                break;
            case 'Q':
            case 'q':
                offst = 3 * NumTris + 4 * readi(fp);
                break;
            case '0':
            case '4':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 1].x - vertexlist[offst].x;
                Ref[ii].j = vertexlist[offst + 1].y - vertexlist[offst].y;
                Ref[ii].k = vertexlist[offst + 1].z - vertexlist[offst].z;
                break;
            case '5':
            case '1':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 2].x - vertexlist[offst + 1].x;
                Ref[ii].j = vertexlist[offst + 2].y - vertexlist[offst + 1].y;
                Ref[ii].k = vertexlist[offst + 2].z - vertexlist[offst + 1].z;
                break;
            case '6':
            case '2':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 3].x - vertexlist[offst + 2].x;
                Ref[ii].j = vertexlist[offst + 3].y - vertexlist[offst + 2].y;
                Ref[ii].k = vertexlist[offst + 3].z - vertexlist[offst + 2].z;
                break;
            case '7':
            case '3':
                offst = 3 * NumTris + 4 * readi(fp);             //total number of triangles incl pents
                Ref[ii].i = vertexlist[offst].x - vertexlist[offst + 3].x;
                Ref[ii].j = vertexlist[offst].y - vertexlist[offst + 3].y;
                Ref[ii].k = vertexlist[offst].z - vertexlist[offst + 3].z;
                break;
            default:
                offst = 0.0f; //FIXME added by chuck_starchaser to shut off warnings; please verify correctness
                break;
        }
        switch (polytype) {
            case '4':
            case '5':
            case '6':
            case '7':
            case 'D':
            case 'E':
            case 'F':
                Ref[ii].i = -Ref[ii].i;
                Ref[ii].j = -Ref[ii].j;
                Ref[ii].k = -Ref[ii].k;
                break;
            default:
                break; //FIXME added by chuck_starchaser to shut off warnings; please verify correctness
        }
        PolyNormal[ii] = PolygonNormal(
                Vector(vertexlist[offst].x, vertexlist[offst].y, vertexlist[offst].z),
                Vector(vertexlist[offst + 1].x, vertexlist[offst + 1].y, vertexlist[offst + 1].z),
                Vector(vertexlist[offst + 2].x, vertexlist[offst + 2].y, vertexlist[offst + 2].z)
        );

        sizes[ii] = readf(fp);
        rotations[ii] = readf(fp);
        offset[ii] = readf(fp);
    }
    forcelogos->push_back(createLogo(numforcelogo, center, PolyNormal, sizes, rotations, 0.01F, FactionUtil::getForceLogo(
            faction), Ref));
    delete[] Ref;
    delete[] PolyNormal;
    delete[] center;
    delete[] sizes;
    delete[] rotations;
    delete[] offset;
    for (ii = 0; ii < NumTris * 3; ii += 3) {
        Vector Norm1(vertexlist[ii + 1].x - vertexlist[ii].x,
                vertexlist[ii + 1].y - vertexlist[ii].y,
                vertexlist[ii + 1].z - vertexlist[ii].z);
        Vector Norm2(vertexlist[ii + 2].x - vertexlist[ii].x,
                vertexlist[ii + 2].y - vertexlist[ii].y,
                vertexlist[ii + 2].z - vertexlist[ii].z);
        Vector Normal;
        CrossProduct(Norm2, Norm1, Normal);
        //CrossProduct (Norm1,Norm2,Normal);
        Normalize(Normal);
        vertexlist[ii].i = vertexlist[ii + 1].i = vertexlist[ii + 2].i = Normal.i;
        vertexlist[ii].j = vertexlist[ii + 1].j = vertexlist[ii + 2].j = Normal.j;
        vertexlist[ii].k = vertexlist[ii + 1].k = vertexlist[ii + 2].k = Normal.k;
    }
    for (ii = NumTris * 3; ii < NumTris * 3 + NumQuads * 4; ii += 4) {
        Vector Norm1(vertexlist[ii + 1].x - vertexlist[ii].x,
                vertexlist[ii + 1].y - vertexlist[ii].y,
                vertexlist[ii + 1].z - vertexlist[ii].z);
        Vector Norm2(vertexlist[ii + 3].x - vertexlist[ii].x,
                vertexlist[ii + 3].y - vertexlist[ii].y,
                vertexlist[ii + 3].z - vertexlist[ii].z);
        Vector Normal;
        CrossProduct(Norm2, Norm1, Normal);
        //CrossProduct (Norm1,Norm2,Normal);
        Normalize(Normal);
        vertexlist[ii].i = vertexlist[ii + 1].i = vertexlist[ii + 2].i = vertexlist[ii + 3].i = Normal.i;
        vertexlist[ii].j = vertexlist[ii + 1].j = vertexlist[ii + 2].j = vertexlist[ii + 3].j = Normal.j;
        vertexlist[ii].k = vertexlist[ii + 1].k = vertexlist[ii + 2].k = vertexlist[ii + 3].k = Normal.k;
    }
    int numsquadlogo = readi(fp);
    PolyNormal = new Vector[numsquadlogo];
    center = new Vector[numsquadlogo];
    sizes = new float[numsquadlogo];
    rotations = new float[numsquadlogo];
    offset = new float[numsquadlogo];
    //char polytype;
    //int offset;
    Ref = new Vector[numsquadlogo];
    for (ii = 0; ii < numsquadlogo; ii++) {
        Ref[ii] = Vector(0, 0, 0);
        center[ii].i = readf(fp);
        center[ii].j = readf(fp);
        center[ii].k = readf(fp);
        polytype = readc(fp);
        switch (polytype) {
            case 'T':
            case 't':
                offst = 3 * readi(fp);
                break;
            case 'D':
            case 'A':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst + 1].x - vertexlist[offst].x;
                Ref[ii].j = vertexlist[offst + 1].y - vertexlist[offst].y;
                Ref[ii].k = vertexlist[offst + 1].z - vertexlist[offst].z;
                break;
            case 'E':
            case 'B':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst + 2].x - vertexlist[offst + 1].x;
                Ref[ii].j = vertexlist[offst + 2].y - vertexlist[offst + 1].y;
                Ref[ii].k = vertexlist[offst + 2].z - vertexlist[offst + 1].z;
                break;
            case 'F':
            case 'C':
                offst = 3 * readi(fp);
                Ref[ii].i = vertexlist[offst].x - vertexlist[offst + 2].x;
                Ref[ii].j = vertexlist[offst].y - vertexlist[offst + 2].y;
                Ref[ii].k = vertexlist[offst].z - vertexlist[offst + 2].z;
                break;
            case 'Q':
            case 'q':
                offst = 3 * NumTris + 4 * readi(fp);
                break;
            case '0':
            case '4':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 1].x - vertexlist[offst].x;
                Ref[ii].j = vertexlist[offst + 1].y - vertexlist[offst].y;
                Ref[ii].k = vertexlist[offst + 1].z - vertexlist[offst].z;
                break;
            case '5':
            case '1':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 2].x - vertexlist[offst + 1].x;
                Ref[ii].j = vertexlist[offst + 2].y - vertexlist[offst + 1].y;
                Ref[ii].k = vertexlist[offst + 2].z - vertexlist[offst + 1].z;
                break;
            case '6':
            case '2':
                offst = 3 * NumTris + 4 * readi(fp);
                Ref[ii].i = vertexlist[offst + 3].x - vertexlist[offst + 2].x;
                Ref[ii].j = vertexlist[offst + 3].y - vertexlist[offst + 2].y;
                Ref[ii].k = vertexlist[offst + 3].z - vertexlist[offst + 2].z;
                break;
            case '7':
            case '3':
                offst = 3 * NumTris + 4 * readi(fp);             //total number of triangles incl pents
                Ref[ii].i = vertexlist[offst].x - vertexlist[offst + 3].x;
                Ref[ii].j = vertexlist[offst].y - vertexlist[offst + 3].y;
                Ref[ii].k = vertexlist[offst].z - vertexlist[offst + 3].z;
                break;
            default:
                offst = 0.0f; //FIXME added by chuck_starchaser to shut off warnings; please verify correctness
                break;
        }
        switch (polytype) {
            case '4':
            case '5':
            case '6':
            case '7':
            case 'D':
            case 'E':
            case 'F':
                Ref[ii].i = -Ref[ii].i;
                Ref[ii].j = -Ref[ii].j;
                Ref[ii].k = -Ref[ii].k;
                break;
            default:
                break; //FIXME added by chuck_starchaser to shut off warnings; please verify correctness
        }
        PolyNormal[ii] = PolygonNormal(
                Vector(vertexlist[offst].x, vertexlist[offst].y, vertexlist[offst].z),
                Vector(vertexlist[offst + 1].x, vertexlist[offst + 1].y, vertexlist[offst + 1].z),
                Vector(vertexlist[offst + 2].x, vertexlist[offst + 2].y, vertexlist[offst + 2].z)
        );
        sizes[ii] = readf(fp);
        rotations[ii] = readf(fp);
        offset[ii] = readf(fp);
    }
    squadlogos->push_back(createLogo(numsquadlogo,
                    center,
                    PolyNormal,
                    sizes,
                    rotations,
                    (float) 0.01,
                    FactionUtil::getSquadLogo(faction),
                    Ref));
    delete[] Ref;
    int vert_offset[2];
    vert_offset[0] = NumTris * 3;
    vert_offset[1] = NumQuads * 4;
    enum POLYTYPE modes[2];
    modes[0] = GFXTRI;
    modes[1] = GFXQUAD;
    vlist = MakeShared<GFXVertexList>(modes, NumTris * 3 + NumQuads * 4, vertexlist, 2, vert_offset);
    //vlist = new GFXVertexList(numtris*4,0,numquads*4, vertexlist+numtris*3);
    /*long pos =*/ fp.GetPosition();
    myMatNum = readi(fp);
    fp.Close();

    delete[] vertexlist;
    delete[] PolyNormal;
    delete[] center;
    delete[] sizes;
    delete[] rotations;
    delete[] offset;
}

