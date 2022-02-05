/**
 * quadsquare_update.cpp
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


#include "quadsquare.h"
int MaxCreateDepth = 0;

void quadsquare::EnableEdgeVertex(int index, bool IncrementCount, const quadcornerdata &cd)
{
//Enable the specified edge vertex.  Indices go { e, n, w, s }.
//Increments the appropriate reference-count if IncrementCount is true.
    if ((EnabledFlags & (1 << index)) && IncrementCount == false) {
        return;
    }
    //static const int	Inc[4] = { 1, 0, 0, 8 };
    //Turn on flag and deal with reference count.
    EnabledFlags |= 1 << index;
    if (IncrementCount == true && (index == 0 || index == 3)) {
        SubEnabledCount[index & 1]++;
    }
    //Now we need to enable the opposite edge vertex of the adjacent square (i.e. the alias vertex).
    //This is a little tricky, since the desired neighbor node may not exist, in which
    //case we have to create it, in order to prevent cracks.  Creating it may in turn cause
    //further edge vertices to be enabled, propagating updates through the tree.
    //The sticking point is the quadcornerdata list, which
    //conceptually is just a linked list of activation structures.
    //In this function, however, we will introduce branching into
    //the "list", making it in actuality a tree.  This is all kind
    //of obscure and hard to explain in words, but basically what
    //it means is that our implementation has to be properly
    //recursive.
    //Travel upwards through the tree, looking for the parent in common with our desired neighbor.
    //Remember the path through the tree, so we can travel down the complementary path to get to the neighbor.
    quadsquare *p = this;
    const quadcornerdata *pcd = &cd;
    int ct = 0;
    int stack[32];
    for (;;) {
        int ci = pcd->ChildIndex;
        if (pcd->Parent == NULL || pcd->Parent->Square == NULL) {
            //Neighbor doesn't exist (it's outside the tree), so there's no alias vertex to enable.
            return;
        }
        p = pcd->Parent->Square;
        pcd = pcd->Parent;

        bool SameParent = ((index - ci) & 2) ? true : false;

        ci = ci ^ 1 ^ ((index & 1) << 1);                     //Child index of neighbor node.

        stack[ct] = ci;
        ct++;
        if (SameParent) {
            break;
        }
    }
    //Get a pointer to our neighbor (create if necessary), by walking down
    //the quadtree from our shared ancestor.
    p = p->EnableDescendant(ct, stack, *pcd);
    //Finally: enable the vertex on the opposite edge of our neighbor, the alias of the original vertex.
    index ^= 2;
    p->EnabledFlags |= (1 << index);
    if (IncrementCount == true && (index == 0 || index == 3)) {
        p->SubEnabledCount[index & 1]++;
    }
}

quadsquare *quadsquare::EnableDescendant(int count, int path[], const quadcornerdata &cd)
{
//This function enables the descendant node 'count' generations below
//us, located by following the list of child indices in path[].
//Creates the node if necessary, and returns a pointer to it.
    count--;
    int ChildIndex = path[count];
    if ((EnabledFlags & (16 << ChildIndex)) == 0) {
        EnableChild(ChildIndex, cd);
    }
    if (count > 0) {
        quadcornerdata q;
        SetupCornerData(&q, cd, ChildIndex);
        return Child[ChildIndex]->EnableDescendant(count, path, q);
    } else {
        return Child[ChildIndex];
    }
}

void quadsquare::CreateChild(int index, const quadcornerdata &cd)
{
//Creates a child square at the specified index.
    if (Child[index] == 0) {
        quadcornerdata q;
        SetupCornerData(&q, cd, index);
        Child[index] = new quadsquare(&q);
    }
}

void quadsquare::EnableChild(int index, const quadcornerdata &cd)
{
//Enable the indexed child node.  { ne, nw, sw, se }
//Causes dependent edge vertices to be enabled.
//if (Enabled[index + 4] == false) {
    if ((EnabledFlags & (16 << index)) == 0) {
//Enabled[index + 4] = true;
        EnabledFlags |= (16 << index);
        EnableEdgeVertex(index, true, cd);
        EnableEdgeVertex((index + 1) & 3, true, cd);
        if (Child[index] == 0) {
            CreateChild(index, cd);
        }
    }
}

void quadsquare::NotifyChildDisable(const quadcornerdata &cd, int index)
{
//Marks the indexed child quadrant as disabled.  Deletes the child node
//if it isn't static.
    //Clear enabled flag for the child.
    EnabledFlags &= ~(16 << index);
    //Update child enabled counts for the affected edge verts.
    quadsquare *s;
    if (index & 2) {
        s = this;
    } else {
        s = GetFarNeighbor(1, cd);
    }
    if (s) {
        s->SubEnabledCount[1]--;
    }
    if (index == 1 || index == 2) {
        s = GetFarNeighbor(2, cd);
    } else {
        s = this;
    }
    if (s) {
        s->SubEnabledCount[0]--;
    }
    if (Child[index]->Static == false) {
        delete Child[index];
        Child[index] = 0;
    }
}

static float DetailThreshold = 100;

bool VertexTest(float x, float y, float z, float error, const Vector &Viewer)
{
//Returns true if the vertex at (x,z) with the given world-space error between
//its interpolated location and its true location, should be enabled, given that
//the viewpoint is located at Viewer[].
    float dx = fabs(x - Viewer.i);
    float dy = fabs(y - Viewer.j);
    float dz = fabs(z - Viewer.k);
    float d = dx;
    if (dy > d) {
        d = dy;
    }
    if (dz > d) {
        d = dz;
    }
    return (error * DetailThreshold) > d;
}

bool BoxTest(float x, float z, float size, float miny, float maxy, float error, const Vector &Viewer)
{
//Returns true if any vertex within the specified box (origin at x,z,
//edges of length size) with the given error value could be enabled
//based on the given viewer location.
    //Find the minimum distance to the box.
    float half = size * 0.5;
    float dx = fabs(x + half - Viewer.i) - half;
    float dy = fabs((miny + maxy) * 0.5 - Viewer.j) - (maxy - miny) * 0.5;
    float dz = fabs(z + half - Viewer.k) - half;
    float d = dx;
    if (dy > d) {
        d = dy;
    }
    if (dz > d) {
        d = dz;
    }
    return (error * DetailThreshold) > d;
}

/**
 * calculates the bitmasks for which children to do and not to do Tries to preserve some locality
 * for 16 the results are
 * Stage 0 Number: 00000000000000000000000000010001
 * Stage 1 Number: 00000000000000000000000000010010
 * Stage 2 Number: 00000000000000000000000000010100
 * Stage 3 Number: 00000000000000000000000000011000
 * Stage 4 Number: 00000000000000000000000000100001
 * Stage 5 Number: 00000000000000000000000000100010
 * Stage 6 Number: 00000000000000000000000000100100
 * Stage 7 Number: 00000000000000000000000000101000
 * Stage 8 Number: 00000000000000000000000001000001
 * Stage 9 Number: 00000000000000000000000001000010
 * Stage 0 Number: 00000000000000000000000001000100
 * Stage 1 Number: 00000000000000000000000001001000
 * Stage 2 Number: 00000000000000000000000010000001
 * Stage 3 Number: 00000000000000000000000010000010
 * Stage 4 Number: 00000000000000000000000010000100
 * Stage 5 Number: 00000000000000000000000010001000
 */
static unsigned int calculatestage(unsigned int numstages, unsigned int whichstage)
{
    unsigned int stage = 0;
    int count = 0;
    numstages -= 1;
    while (numstages) {
        int tmp = 1 << (whichstage % 4);
        stage += (tmp << ((count++) * 4));
        whichstage /= 4;
        numstages /= 4;
    }
    return stage;
}

///transforms our stages to little endian notation so most significant half-byte is on the right :-)
static unsigned int transformstage(unsigned int stage, updateparity *updateorder)
{
    int tmp;
    unsigned int transformedstage = 0;
    while ((tmp = (stage & (1 | 2 | 4 | 8))) != 0) {
        stage >>= 4;
        transformedstage <<= 4;
        transformedstage |= ((*updateorder)(tmp));
    }
    return transformedstage;
}

int identityparity(int i)
{
    return i;
}

int sideparityodd(int i)
{
    switch (i) {
        case 1:
            return 2;

        case 2:
            return 1;

        case 4:
            return 8;

        case 8:
            return 4;
    }
    return 0;
}

int upparityodd(int i)
{
    switch (i) {
        case 1:
            return 4;

        case 2:
            return 8;

        case 4:
            return 1;

        case 8:
            return 2;
    }
    return 0;
}

int sideupparityodd(int i)
{
    switch (i) {
        case 8:
            return 1;

        case 4:
            return 2;

        case 2:
            return 4;

        case 1:
            return 8;
    }
    return 0;
}

/// Refresh the vertex enabled states in the tree, according to the
/// location of the viewer.  May force creation or deletion of qsquares
/// in areas which need to be interpolated.
void quadsquare::Update(const quadcornerdata &cd,
                        const Vector &ViewerLocation,
                        float Detail,
                        unsigned short numstages,
                        unsigned short whichstage,
                        updateparity *par)
{
    DetailThreshold = Detail;
    UpdateAux(cd, ViewerLocation, 0, transformstage(calculatestage(numstages, whichstage), par));
}

void quadsquare::UpdateAux(const quadcornerdata &cd,
                           const Vector &ViewerLocation,
                           float CenterError,
                           unsigned int whichChildren)
{
//Does the actual work of updating enabled states and tree growing/shrinking.
    //Make sure error values are current.
    if (Dirty) {
        RecomputeErrorAndLighting(cd);
    }
    int half = 1 << cd.Level;
    //See about enabling child verts.
    if ((EnabledFlags & 1) == 0
            && VertexTest(cd.xorg + (half << 1), Vertex[1].Y, cd.zorg + half, Error[0], ViewerLocation) == true) {
        EnableEdgeVertex(
                0,
                false, cd);             //East vert.
    }
    if ((EnabledFlags & 8) == 0
            && VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + (half << 1), Error[1], ViewerLocation) == true) {
        EnableEdgeVertex(
                3,
                false,
                cd);             //South vert.
    }
    if (cd.Level > 0) {
        if ((EnabledFlags & 32) == 0) {
            if (BoxTest(cd.xorg, cd.zorg, half, MinY, MaxY, Error[3], ViewerLocation) == true) {
                EnableChild(1, cd);
            }
        }
        //nw child.er
        if ((EnabledFlags & 16) == 0) {
            if (BoxTest(cd.xorg + half, cd.zorg, half, MinY, MaxY, Error[2], ViewerLocation) == true) {
                EnableChild(0, cd);
            }
        }
        //ne child.
        if ((EnabledFlags & 64) == 0) {
            if (BoxTest(cd.xorg, cd.zorg + half, half, MinY, MaxY, Error[4], ViewerLocation) == true) {
                EnableChild(2, cd);
            }
        }
        //sw child.
        if ((EnabledFlags & 128) == 0) {
            if (BoxTest(cd.xorg + half, cd.zorg + half, half, MinY, MaxY, Error[5], ViewerLocation) == true) {
                EnableChild(3, cd);
            }
        }
        //se child.
        //Recurse into child quadrants as necessary.
        quadcornerdata q;
        if (whichChildren) {
            //if we want to mask out certain vertices for pipelined execution
            if ((EnabledFlags & 32) && (whichChildren & 0x1)) {
                SetupCornerData(&q, cd, 1);
                Child[1]->UpdateAux(q, ViewerLocation, Error[3], whichChildren >> 4);
            }
            if ((EnabledFlags & 16) && (whichChildren & 0x2)) {
                SetupCornerData(&q, cd, 0);
                Child[0]->UpdateAux(q, ViewerLocation, Error[2], whichChildren >> 4);
            }
            if ((EnabledFlags & 64) && (whichChildren & 0x4)) {
                SetupCornerData(&q, cd, 2);
                Child[2]->UpdateAux(q, ViewerLocation, Error[4], whichChildren >> 4);
            }
            if ((EnabledFlags & 128) && (whichChildren & 0x8)) {
                SetupCornerData(&q, cd, 3);
                Child[3]->UpdateAux(q, ViewerLocation, Error[5], whichChildren >> 4);
            }
        } else {
            //if we want to do all
            if ((EnabledFlags & 32)) {
                SetupCornerData(&q, cd, 1);
                Child[1]->UpdateAux(q, ViewerLocation, Error[3], 0);
            }
            if ((EnabledFlags & 16)) {
                SetupCornerData(&q, cd, 0);
                Child[0]->UpdateAux(q, ViewerLocation, Error[2], 0);
            }
            if ((EnabledFlags & 64)) {
                SetupCornerData(&q, cd, 2);
                Child[2]->UpdateAux(q, ViewerLocation, Error[4], 0);
            }
            if ((EnabledFlags & 128)) {
                SetupCornerData(&q, cd, 3);
                Child[3]->UpdateAux(q, ViewerLocation, Error[5], 0);
            }
        }
    }
    //Test for disabling.  East, South, and center.
    if ((EnabledFlags & 1) && SubEnabledCount[0] == 0
            && VertexTest(cd.xorg + (half << 1), Vertex[1].Y, cd.zorg + half, Error[0], ViewerLocation) == false) {
        EnabledFlags &= ~1;
        quadsquare *s = GetFarNeighbor(0, cd);
        if (s) {
            s->EnabledFlags &= ~4;
        }
    }
    if ((EnabledFlags & 8) && SubEnabledCount[1] == 0
            && VertexTest(cd.xorg + half, Vertex[4].Y, cd.zorg + (half << 1), Error[1], ViewerLocation) == false) {
        EnabledFlags &= ~8;
        quadsquare *s = GetFarNeighbor(3, cd);
        if (s) {
            s->EnabledFlags &= ~2;
        }
    }
    if (EnabledFlags == 0
            && cd.Parent != NULL
            && BoxTest(cd.xorg, cd.zorg, (half << 1), MinY, MaxY, CenterError, ViewerLocation) == false) {
        //Disable ourself.
        cd.Parent->Square->NotifyChildDisable(*cd.Parent, cd.ChildIndex);
    }             //nb: possibly deletes 'this'.
}

inline Vector Normalise(const Vector &vec, const float scale)
{
    return vec * (scale / vec.Magnitude());
}

Vector quadsquare::MakeLightness(float xslope, float zslope, const Vector &loc)
{
    Vector tmp(nonlinear_trans->TransformNormal(loc.Cast(), QVector(-xslope, 1, -zslope)).Cast());
    tmp.Normalize();
    return Vector(tmp.i * normalscale.i, tmp.j * normalscale.j, tmp.k * normalscale.k);
}

/**
 * Recomputes the error values for this tree.  Returns the
 * max error.
 * Also updates MinY & MaxY.
 * Also computes quick & dirty vertex lighting for the demo.
 */
float quadsquare::RecomputeErrorAndLighting(const quadcornerdata &cd)
{
    int i;
    //Measure error of center and edge vertices.
    float maxerror = 0;
    //Compute error of center vert.
    float e;
    if (cd.ChildIndex & 1) {
        e = fabs(Vertex[0].Y - (cd.Verts[1].Y + cd.Verts[3].Y) * 0.5);
    } else {
        e = fabs(Vertex[0].Y - (cd.Verts[0].Y + cd.Verts[2].Y) * 0.5);
    }
    if (e > maxerror) {
        maxerror = e;
    }
    //Initial min/max.
    MaxY = (unsigned short) Vertex[0].Y;
    MinY = (unsigned short) Vertex[0].Y;
    //Check min/max of corners.
    for (i = 0; i < 4; i++) {
        float y = cd.Verts[i].Y;
        if (y < MinY) {
            MinY = (unsigned short) y;
        }
        if (y > MaxY) {
            MaxY = (unsigned short) y;
        }
    }
    //Edge verts.
    e = fabs(Vertex[1].Y - (cd.Verts[0].Y + cd.Verts[3].Y) * 0.5);
    if (e > maxerror) {
        maxerror = e;
    }
    Error[0] = (unsigned short) e;
    e = fabs(Vertex[4].Y - (cd.Verts[2].Y + cd.Verts[3].Y) * 0.5);
    if (e > maxerror) {
        maxerror = e;
    }
    Error[1] = (unsigned short) e;
    //Min/max of edge verts.
    for (i = 0; i < 4; i++) {
        float y = Vertex[1 + i].Y;
        if (y < MinY) {
            MinY = (unsigned short) y;
        }
        if (y > MaxY) {
            MaxY = (unsigned short) y;
        }
    }
    //Check child squares.
    for (i = 0; i < 4; i++) {
        quadcornerdata q;
        if (Child[i]) {
            SetupCornerData(&q, cd, i);
            Error[i + 2] = (unsigned short) Child[i]->RecomputeErrorAndLighting(q);
            if (Child[i]->MinY < MinY) {
                MinY = Child[i]->MinY;
            }
            if (Child[i]->MaxY > MaxY) {
                MaxY = Child[i]->MaxY;
            }
        } else {
            //Compute difference between bilinear average at child center, and diagonal edge approximation.
            Error[i
                    + 2] =
                    (unsigned short) (fabs((double) ((Vertex[0].Y
                            + cd.Verts[i].Y) - (Vertex[i + 1].Y + Vertex[((i + 1) & 3) + 1].Y))) * 0.25);
        }
        if (Error[i + 2] > maxerror) {
            maxerror = Error[i + 2];
        }
    }
    //
    //Compute quickie demo lighting.
    //
    float OneOverSize = 1.0 / (2 << cd.Level);
    GFXVertex *vertexs = vertices->BeginMutate(0)->vertices;
    GFXVertex *V = &vertexs[Vertex[0].vertindex];
    V->SetNormal(MakeLightness((Vertex[1].Y - Vertex[3].Y) * OneOverSize,
                               (Vertex[4].Y - Vertex[2].Y) * OneOverSize,
                               V->GetConstVertex()));
    float v;
    quadsquare *s = GetFarNeighbor(0, cd);
    if (s) {
        v = s->Vertex[0].Y;
    } else {
        v = Vertex[1].Y;
    }
    V = &vertexs[Vertex[1].vertindex];
    V->SetNormal(MakeLightness((v - Vertex[0].Y) * OneOverSize,
                               (cd.Verts[3].Y - cd.Verts[0].Y) * OneOverSize,
                               V->GetConstVertex()));
    s = GetFarNeighbor(1, cd);
    if (s) {
        v = s->Vertex[0].Y;
    } else {
        v = Vertex[2].Y;
    }
    V = &vertexs[Vertex[2].vertindex];
    V->SetNormal(MakeLightness((cd.Verts[0].Y - cd.Verts[1].Y) * OneOverSize,
                               (Vertex[0].Y - v) * OneOverSize,
                               V->GetConstVertex()));
    s = GetFarNeighbor(2, cd);
    if (s) {
        v = s->Vertex[0].Y;
    } else {
        v = Vertex[3].Y;
    }
    V = &vertexs[Vertex[3].vertindex];
    V->SetNormal(MakeLightness((Vertex[0].Y - v) * OneOverSize,
                               (cd.Verts[2].Y - cd.Verts[1].Y) * OneOverSize,
                               V->GetConstVertex()));
    s = GetFarNeighbor(3, cd);
    if (s) {
        v = s->Vertex[0].Y;
    } else {
        v = Vertex[4].Y;
    }
    V = &vertexs[Vertex[4].vertindex];
    V->SetNormal(MakeLightness((cd.Verts[3].Y - cd.Verts[2].Y) * OneOverSize,
                               (v - Vertex[0].Y) * OneOverSize,
                               V->GetConstVertex()));
    vertices->EndMutate();
    //The error, MinY/MaxY, and lighting values for this node and descendants are correct now.
    Dirty = false;
    return maxerror;
}

