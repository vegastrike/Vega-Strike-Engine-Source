/*
 * quadsquare.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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


/**
 *
 * Data structures for quadtree terrain storage.
 * This code may be freely modified and redistributed.  I make no
 * warrantees about it; use at your own risk.  If you do incorporate
 * this code into a project, I'd appreciate a mention in the credits.
 * Thatcher Ulrich <tu@tulrich.com>
 *
 * Rewritten and adapted to Vegastrike by Daniel Horn
 */

#include <stdio.h>
#include <float.h>
#include <math.h>
#include <assert.h>
#include "quadsquare.h"
#include "gfxlib.h"
#include "aux_texture.h"
using std::vector;

unsigned int *quadsquare::VertexAllocated;
unsigned int *quadsquare::VertexCount;
GFXVertexList *quadsquare::vertices;
GFXVertexList *quadsquare::blendVertices = NULL;
std::vector<TextureIndex>quadsquare::indices;
std::vector<unsigned int> *quadsquare::unusedvertices;
IdentityTransform *quadsquare::nonlinear_trans;
std::vector<TerrainTexture> *quadsquare::textures;
Vector quadsquare::normalscale;
Vector quadsquare::camerapos;
quadsquare *quadsquare::neighbor[4] = {NULL, NULL, NULL, NULL};

unsigned int quadsquare::SetVertices(GFXVertex *vertexs, const quadcornerdata &pcd) {
    unsigned int half = 1 << pcd.Level;
    Vector v[5];
    v[0].i = pcd.xorg + half;
    v[0].k = pcd.zorg + half;
    v[1].i = pcd.xorg + half * 2;
    v[1].k = pcd.zorg + half;
    v[2].i = pcd.xorg + half;
    v[2].k = pcd.zorg;
    v[3].i = pcd.xorg;
    v[3].k = pcd.zorg + half;
    v[4].i = pcd.xorg + half;
    v[4].k = pcd.zorg + half * 2;
    for (unsigned int i = 0; i < 5; i++) {
        v[i].j = Vertex[i].Y;
        vertexs[Vertex[i].vertindex].SetTexCoord(nonlinear_trans->TransformS(v[i].i,
                        (*textures)[Vertex[i].GetTex()].scales),
                nonlinear_trans->TransformT(v[i].k,
                        (*textures)[Vertex[i].GetTex()].scalet));
        vertexs[Vertex[i].vertindex].SetVertex(nonlinear_trans->Transform(v[i].Cast()).Cast());
    }
    return half;
}

//Verts mapping:
//1-0
//| |
//2-3
//
//Vertex mapping:
//+-2-+
//| | |
//3-0-1
//| | |
//+-4-+
static void InterpolateTextures(VertInfo res[5], VertInfo in[4], const quadcornerdata &cd) {
    //const float epsilon;
    res[0].SetTex(0.25 * ((((float) in[0].Rem) + in[1].Rem + in[2].Rem + in[3].Rem) / 256. + in[0].Tex + in[1].Tex
            + in[2].Tex + in[3].Tex));
    res[1].SetTex(0.5 * ((((float) in[0].Rem) + in[3].Rem) / 256. + (in[3].Tex) + in[0].Tex));
    res[2].SetTex(0.5 * ((((float) in[0].Rem) + in[1].Rem) / 256. + (in[0].Tex) + in[1].Tex));
    res[3].SetTex(0.5 * ((((float) in[1].Rem) + in[2].Rem) / 256. + (in[1].Tex) + in[2].Tex));
    res[4].SetTex(0.5 * ((((float) in[2].Rem) + in[3].Rem) / 256. + (in[2].Tex) + in[3].Tex));
    /*
     *     float pos[5];
     *     int half = 1<< cd.Level;
     *     pos[0] = (cd.xorg + half+ cd.zorg + half);
     *     pos[1] = (cd.xorg + half*2+ cd.zorg + half);
     *     pos[2] = (cd.xorg + half+ cd.zorg);
     *     pos[3] = (cd.xorg+ cd.zorg + half);
     *     pos[4] = (cd.xorg + half+ cd.zorg + half*2);
     *
     *
     *     for (int i = 0; i < 5; i++) {
     *
     *       res[i].SetTex(((int)((pos[i])/5000))%10);
     *     }
     */
    res[0].Y = (unsigned short) (0.25 * (((float) in[0].Y) + in[1].Y + in[2].Y + in[3].Y));
    res[1].Y = (unsigned short) (0.5 * (((float) in[3].Y) + in[0].Y));
    res[2].Y = (unsigned short) (0.5 * (((float) in[0].Y) + in[1].Y));
    res[3].Y = (unsigned short) (0.5 * (((float) in[1].Y) + in[2].Y));
    res[4].Y = (unsigned short) (0.5 * (((float) in[2].Y) + in[3].Y));
}

quadsquare::quadsquare(quadcornerdata *pcd) {
    pcd->Square = this;
    //Set static to true if/when this node contains real data, and
    //not just interpolated values.  When static == false, a node
    //can be deleted by the Update() function if none of its
    //vertices or children are enabled.
    Static = false;
    int i;
    for (i = 0; i < 4; i++) {
        Child[i] = NULL;
    }
    EnabledFlags = 0;
    for (i = 0; i < 2; i++) {
        SubEnabledCount[i] = 0;
    }
    //Set default vertex positions by interpolating from given corners.
    //Just bilinear interpolation.
    InterpolateTextures(Vertex, pcd->Verts, *pcd);
    for (i = 0; i < 2; i++) {
        Error[i] = 0;
    }
    for (i = 0; i < 4; i++) {
        Error[i
                + 2] =
                (unsigned short) (fabs((double) ((Vertex[0].Y
                        + pcd->Verts[i].Y) - (Vertex[i + 1].Y + Vertex[((i + 1) & 3) + 1].Y))) * 0.25);
    }
    //Compute MinY/MaxY based on corner verts.
    MinY = MaxY = (unsigned short) pcd->Verts[0].Y;
    for (i = 1; i < 4; i++) {
        float y = pcd->Verts[i].Y;
        if (y < MinY) {
            MinY = (unsigned short) y;
        }
        if (y > MaxY) {
            MaxY = (unsigned short) y;
        }
    }
    GFXVertex **vertexs = &(vertices->BeginMutate(0)->vertices);
    GFXVertex v[5];
    v[0].SetNormal(((*vertexs)[pcd->Verts[0].vertindex].GetNormal() + (*vertexs)[pcd->Verts[1].vertindex].GetNormal()
            + (*vertexs)[pcd->Verts[2].vertindex].GetNormal()
            + (*vertexs)[pcd->Verts[3].vertindex].GetNormal()).Normalize());
    v[1].SetNormal(
            ((*vertexs)[pcd->Verts[0].vertindex].GetNormal()
                    + (*vertexs)[pcd->Verts[3].vertindex].GetNormal()).Normalize());
    v[2].SetNormal(
            ((*vertexs)[pcd->Verts[0].vertindex].GetNormal()
                    + (*vertexs)[pcd->Verts[1].vertindex].GetNormal()).Normalize());
    v[3].SetNormal(
            ((*vertexs)[pcd->Verts[1].vertindex].GetNormal()
                    + (*vertexs)[pcd->Verts[2].vertindex].GetNormal()).Normalize());
    v[4].SetNormal(
            ((*vertexs)[pcd->Verts[2].vertindex].GetNormal()
                    + (*vertexs)[pcd->Verts[3].vertindex].GetNormal()).Normalize());
    //FIXME fill in st!
    for (i = 0; i < 5; i++) {
        //v[i].y= Vertex[i].Y;
        if (unusedvertices->size()) {
            (*vertexs)[unusedvertices->back()] = v[i];
            Vertex[i].vertindex = unusedvertices->back();
            unusedvertices->pop_back();
        } else {
            Vertex[i].vertindex = *VertexCount;
            if ((*VertexCount) + 1 >= (*VertexAllocated)) {
                (*VertexAllocated) *= 2;
                (*vertexs) = (GFXVertex *) realloc((*vertexs), (*VertexAllocated) * sizeof(GFXVertex));
            }
            (*vertexs)[*VertexCount] = v[i];
            (*VertexCount)++;
        }
    }
    SetVertices(*vertexs, *pcd);
    vertices->EndMutate(*VertexCount);
    //interpolate from other vertices;
}

quadsquare::~quadsquare() {
    //Recursively delete sub-trees.
    int i;
    for (i = 0; i < 5; i++) {
        unusedvertices->push_back(Vertex[i].vertindex);
    }
    for (i = 0; i < 4; i++) {
        if (Child[i] != nullptr) {
            delete Child[i];
            Child[i] = nullptr;
        }
    }
}

/**
 * Sets this node's static flag to true.  If static == true, then the
 * node or its children is considered to contain significant height data
 * and shouldn't be deleted.
 */
void quadsquare::SetStatic(const quadcornerdata &cd) {
    if (Static == false) {
        Static = true;
        //Propagate static status to ancestor nodes.
        if (cd.Parent && cd.Parent->Square) {
            cd.Parent->Square->SetStatic(*cd.Parent);
        }
    }
}

int quadsquare::CountNodes() const {
//Debugging function.  Counts the number of nodes in this subtree.
    int count = 1;              //Count ourself.
    //Count descendants.
    for (int i = 0; i < 4; i++) {
        if (Child[i]) {
            count += Child[i]->CountNodes();
        }
    }
    return count;
}

/**
 * Returns the height of the heightfield at the specified x,z coordinates.
 * Can be used for collision detection
 */
float quadsquare::GetHeight(const quadcornerdata &cd, float x, float z, Vector &normal) //const
{
    int half = 1 << cd.Level;
    float lx = (x - cd.xorg) / float(half);
    float lz = (z - cd.zorg) / float(half);
    int ix = (int) floor(lx);
    int iz = (int) floor(lz);
    //Clamp.
    if (ix < 0) {
        return -FLT_MAX;
    }       //ix = 0;
    if (ix > 1) {
        return -FLT_MAX;
    }       //ix = 1;
    if (iz < 0) {
        return -FLT_MAX;
    }        //iz = 0;
    if (iz > 1) {
        return -FLT_MAX;
    }          ///iz = 1;

    int index = (ix ^ (iz ^ 1))
            + (iz << 1); //FIXME gcc computes ix^((iz^1)+(iz<<1)).. Was this the intent? Who can understand this code?
    if (Child[index] && Child[index]->Static) {
        //Pass the query down to the child which contains it.
        quadcornerdata q;
        SetupCornerData(&q, cd, index);
        return Child[index]->GetHeight(q, x, z, normal);
    }
    //Bilinear interpolation.
    lx -= ix;
    if (lx <= 0.f)
        lx = 0.f;
    else
        lx = 1.f;
    lz -= iz;
    if (lz > 1)
        lz = 1;
    float s00, s01, s10, s11;
    switch (index) {
        default:
        case 0:
            s00 = Vertex[2].Y;
            s01 = cd.Verts[0].Y;
            s10 = Vertex[0].Y;
            s11 = Vertex[1].Y;
            break;
        case 1:
            s00 = cd.Verts[1].Y;
            s01 = Vertex[2].Y;
            s10 = Vertex[3].Y;
            s11 = Vertex[0].Y;
            break;
        case 2:
            s00 = Vertex[3].Y;
            s01 = Vertex[0].Y;
            s10 = cd.Verts[2].Y;
            s11 = Vertex[4].Y;
            break;
        case 3:
            s00 = Vertex[0].Y;
            s01 = Vertex[1].Y;
            s10 = Vertex[4].Y;
            s11 = cd.Verts[3].Y;
            break;
    }
    normal = (Vector(0, s10 - s00, half)).Cross(Vector(half, s01 - s00, 0));
    return (s00 * (1 - lx) + s01 * lx) * (1 - lz) + (s10 * (1 - lx) + s11 * lx) * lz;
}

quadsquare *quadsquare::GetFarNeighbor(int dir, const quadcornerdata &cd) const {
//Traverses the tree in search of the quadsquare neighboring this square to the
//specified direction.  0-3 --> { E, N, W, S }.
//Returns NULL if the neighbor is outside the bounds of the tree.
    //If we don't have a parent, then we don't have a neighbor.
    //(Actually, we could have inter-tree connectivity at this level
    //for connecting separate trees together.)
    if (cd.Parent == 0) {
        return neighbor[dir];
    }
    //Find the parent and the child-index of the square we want to locate or create.
    quadsquare *p = 0;
    int index = cd.ChildIndex ^ 1 ^ ((dir & 1) << 1);
    bool SameParent = ((dir - cd.ChildIndex) & 2) ? true : false;
    if (SameParent) {
        p = cd.Parent->Square;
    } else {
        p = cd.Parent->Square->GetFarNeighbor(dir, *cd.Parent);
        if (p == 0) {
            return 0;
        }
    }
    quadsquare *n = p->Child[index];
    return n;
}

quadsquare *quadsquare::GetNeighbor(int dir, const quadcornerdata &cd) const {
//Traverses the tree in search of the quadsquare neighboring this square to the
//specified direction.  0-3 --> { E, N, W, S }.
//Returns NULL if the neighbor is outside the bounds of the tree.
    //If we don't have a parent, then we don't have a neighbor.
    //(Actually, we could have inter-tree connectivity at this level
    //for connecting separate trees together.)
    if (cd.Parent == 0) {
        return NULL;
    }
    //Find the parent and the child-index of the square we want to locate or create.
    quadsquare *p = 0;
    int index = cd.ChildIndex ^ 1 ^ ((dir & 1) << 1);
    bool SameParent = ((dir - cd.ChildIndex) & 2) ? true : false;
    if (SameParent) {
        p = cd.Parent->Square;
    } else {
        p = cd.Parent->Square->GetNeighbor(dir, *cd.Parent);
        if (p == 0) {
            return 0;
        }
    }
    quadsquare *n = p->Child[index];
    return n;
}

void VertInfo::SetTex(float t) {
    Tex = (unsigned char) t;
    Rem = (unsigned char) ((t - Tex) * 256);
    /*
     *  if (Rem==127||Rem==126||Rem==125)
     *  Rem = 128;
     */
    assert(t - Tex < 1);
}

void quadsquare::SetCurrentTerrain(unsigned int *VertexAllocated,
        unsigned int *VertexCount,
        GFXVertexList *vertices,
        std::vector<unsigned int> *unvert,
        IdentityTransform *nlt,
        std::vector<TerrainTexture> *tex,
        const Vector &NormScale,
        quadsquare *neighbors[4]) {
    normalscale = NormScale;
    neighbor[0] = neighbors[0];
    neighbor[1] = neighbors[1];
    neighbor[2] = neighbors[2];
    neighbor[3] = neighbors[3];
    if (quadsquare::blendVertices == NULL) {
        GFXColorVertex tmp[3];
        blendVertices = new GFXVertexList(GFXTRI, 3, tmp, 3, true);
    }
    quadsquare::VertexAllocated = VertexAllocated;
    quadsquare::VertexCount = VertexCount;
    quadsquare::vertices = vertices;
    quadsquare::unusedvertices = unvert;
    nonlinear_trans = nlt;
    textures = tex;
    if (indices.size() < tex->size()) {
        while (indices.size() < tex->size()) {
            indices.push_back(TextureIndex());
        }
    }
}

static unsigned char texturelookup[256];

void quadsquare::AddHeightMap(const quadcornerdata &cd, const HeightMapInfo &hm) {
    unsigned int i;
    memset(texturelookup, 0, sizeof(unsigned char) * 256);
    for (i = 0; i < textures->size(); i++) {
        texturelookup[(*textures)[i].color] = i;
    }
    AddHeightMapAux(cd, hm);
}

void quadsquare::AddHeightMapAux(const quadcornerdata &cd, const HeightMapInfo &hm) {
//Sets the height of all samples within the specified rectangular
//region using the given array of floats.  Extends the tree to the
//level of detail defined by (1 << hm.Scale) as necessary.
    //If block is outside rectangle, then don't bother.
    int BlockSize = 2 << cd.Level;
    if (cd.xorg > hm.XOrigin + ((int) (hm.XSize + 2) << hm.Scale)
            || cd.xorg + BlockSize < hm.XOrigin - (1 << hm.Scale)
            || cd.zorg > hm.ZOrigin + ((int) (hm.ZSize + 2) << hm.Scale)
            || cd.zorg + BlockSize < hm.ZOrigin - (1 << hm.Scale)) {
        //This square does not touch the given height array area; no need to modify this square or descendants.
        return;
    }
    if (cd.Parent && cd.Parent->Square) {
        cd.Parent
                ->Square
                ->EnableChild(cd.ChildIndex,
                        *cd.Parent);
    }            //causes parent edge verts to be enabled, possibly causing neighbor blocks to be created.
    int i;
    int half = 1 << cd.Level;
    //Create and update child nodes.
    for (i = 0; i < 4; i++) {
        quadcornerdata q;
        SetupCornerData(&q, cd, i);
        if (Child[i] == NULL && cd.Level > hm.Scale) {
            //Create child node w/ current (unmodified) values for corner verts.
            Child[i] = new quadsquare(&q);
        }
        //Recurse.
        if (Child[i]) {
            Child[i]->AddHeightMapAux(q, hm);
        }
    }
    //don't want to bother changing things if the sample won't change things :-)
    int s[5];
    float texture[5];
    s[0] = (int) hm.Sample(cd.xorg + half, cd.zorg + half, texture[0]);
    s[1] = (int) hm.Sample(cd.xorg + half * 2, cd.zorg + half, texture[1]);
    s[2] = (int) hm.Sample(cd.xorg + half, cd.zorg, texture[2]);
    s[3] = (int) hm.Sample(cd.xorg, cd.zorg + half, texture[3]);
    s[4] = (int) hm.Sample(cd.xorg + half, cd.zorg + half * 2, texture[4]);
    //Modify the vertex heights if necessary, and set the dirty
    //flag if any modifications occur, so that we know we need to
    //recompute error data later.
    /*
     *  float pos[5];
     *  pos[0] = (cd.xorg + half+ cd.zorg + half);
     *  pos[1] = (cd.xorg + half*2+ cd.zorg + half);
     *  pos[2] = (cd.xorg + half+ cd.zorg);
     *  pos[3] = (cd.xorg+ cd.zorg + half);
     *  pos[4] = (cd.xorg + half+ cd.zorg + half*2);
     *
     */
    for (i = 0; i < 5; i++) {
        //Vertex[i].SetTex(((int)((pos[i])/5000))%10);
        Vertex[i].SetTex(texture[i]);
        if (s[i] != 0) {
            Dirty = true;
            if (Vertex[i].Y + s[i] > 0) {
                Vertex[i].Y += s[i];
            } else {
                Vertex[i].Y = 0;
            }
            //vertices[Vertex[i].vertindex].x = v[i].i;//FIXME are we necessary?
            //vertices[Vertex[i].vertindex].z = v[i].k;
        }
    }
    if (Dirty) {
        GFXVertex *vertexs = vertices->BeginMutate(0)->vertices;
        SetVertices(vertexs, cd);
        vertices->EndMutate();
    } else {
        //Check to see if any child nodes are dirty, and set the dirty flag if so.
        for (i = 0; i < 4; i++) {
            if (Child[i] && Child[i]->Dirty) {
                Dirty = true;
                break;
            }
        }
    }
    if (Dirty) {
        SetStatic(cd);
    }
}

//
//HeightMapInfo
//

float HeightMapInfo::Sample(int x, int z, float &texture) const
//Returns the height (y-value) of a point in this heightmap.  The given (x,z) are in
//world coordinates.  Heights outside this heightmap are considered to be 0.  Heights
//between sample points are bilinearly interpolated from surrounding points.
//xxx deal with edges: either force to 0 or over-size the query region....
{
    //Break coordinates into grid-relative coords (ix,iz) and remainder (rx,rz)
    int ix = (x - XOrigin) >> Scale;
    int iz = (z - ZOrigin) >> Scale;
    int mask = (1 << Scale) - 1;
    int rx = (x - XOrigin) & mask;
    int rz = (z - ZOrigin) & mask;
    int ixpp = ix + 1;
    int izpp = iz + 1;
    if (ix < 0) {
        ix = 0;
        ixpp = 0;
    }
    if (ix >= (int) (XSize - 1)) {
        ix = XSize - 1;
        ixpp = XSize - 1;
    }
    if (iz < 0) {
        iz = 0;
        izpp = 0;
    }
    if (iz >= (int) (ZSize - 1)) {
        iz = ZSize - 1;
        izpp = ZSize - 1;
    }
    float fx = float(rx) / (mask + 1);
    float fz = float(rz) / (mask + 1);
    float s00 = Data[ix + iz * RowWidth];
    float s01 = Data[(ixpp) + iz * RowWidth];
    float s10 = Data[ix + (izpp) * RowWidth];
    float s11 = Data[(ixpp) + (izpp) * RowWidth];
    float t00 = texturelookup[terrainmap[ix + iz * RowWidth]];
    float t01 = texturelookup[terrainmap[(ixpp) + iz * RowWidth]];
    float t10 = texturelookup[terrainmap[ix + (izpp) * RowWidth]];
    float t11 = texturelookup[terrainmap[(ixpp) + (izpp) * RowWidth]];
    texture = (t00 * (1 - fx) + t01 * fx) * (1 - fz)
            + (t10 * (1 - fx) + t11 * fx) * fz;
    return (s00 * (1 - fx) + s01 * fx) * (1 - fz)
            + (s10 * (1 - fx) + s11 * fx) * fz;
}

