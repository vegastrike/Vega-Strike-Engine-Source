/*
 * quadsquare_render.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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


#include "quadsquare.h"
#include "aux_texture.h"

float SphereTransformRenderlevel = 0;

static void TerrainMakeActive(const TerrainTexture &text) {
    if (text.tex.t) {
        GFXEnable(TEXTURE0);
        text.tex.t->MakeActive();
    } else {
        GFXDisable(TEXTURE0);
    }
    GFXBlendMode(text.blendSrc, text.blendDst);
    if (text.reflect) {
        GFXEnable(TEXTURE1);
    } else {
        GFXDisable(TEXTURE1);
    }
    GFXSelectMaterial(text.material);
}

static void TerrainMakeClearActive(const TerrainTexture &text) {
    TerrainMakeActive(text);
    GFXBlendMode(text.blendDst != ZERO ? text.blendSrc : SRCALPHA, INVSRCALPHA);
}

static void TerrainMakeDeactive(const TerrainTexture &text) {
    if (text.tex.t) {
    }
}

typedef std::vector<TextureIndex> vecTextureIndex;
typedef std::vector<TerrainTexture> vecTextureStar;

/**
 * Draws the heightfield represented by this tree
 * Returns teh number of triangles rendered (not including multipass
 */
int quadsquare::Render(const quadcornerdata &cd, const Vector &camvec) {
    quadsquare::camerapos = camvec;
    vertices->LoadDrawState();
    vertices->BeginDrawState(GFXFALSE);
    int totsize = 0;
    RenderAux(cd, GFX_PARTIALLY_VISIBLE);
    vecTextureIndex::iterator i = indices.begin();
    vecTextureStar::iterator k;
    for (k = textures->begin(); k != textures->end(); i++, k++) {
        TerrainMakeActive(*k);
        unsigned int isize = (*i).q.size();
        totsize += isize;
        vertices->Draw(GFXTRI, isize, (*i).q.begin());
        TerrainMakeDeactive(*k);
    }
    vertices->EndDrawState();
    i = indices.begin();
    int j = 0;
    for (k = textures->begin(); k != textures->end(); i++, j++, k++) {
        if ((*i).c.size() > 2) {
            TerrainMakeClearActive(*k);
            GFXPolygonOffset(0, -j);
            GFXColorMaterial(AMBIENT | DIFFUSE);
            GFXColorVertex **cv = (&blendVertices->BeginMutate(0)->colors);
            GFXColorVertex *tmp = *cv;
            *cv = (*i).c.begin();
            blendVertices->EndMutate((*i).c.size());
            blendVertices->LoadDrawState();
            blendVertices->BeginDrawState(GFXFALSE);
            blendVertices->Draw(GFXTRI, (*i).c.size());
            blendVertices->EndDrawState(GFXFALSE);
            cv = (&blendVertices->BeginMutate(0)->colors);
            *cv = tmp;
            blendVertices->EndMutate(3);
            GFXColorMaterial(0);
            GFXPolygonOffset(0, 0);
            TerrainMakeDeactive(*k);
        }
        (*i).Clear();
    }
    return totsize;
}

//#define DONOTDRAWBLENDEDQUADS
inline void RotateTriRight(unsigned int &aa,
        unsigned short &ta,
        unsigned int &bb,
        unsigned short &tb,
        unsigned int &cc,
        unsigned short &tc) {
    unsigned int baki;
    unsigned short baks;
    baks = ta;
    baki = aa;
    aa = cc;
    ta = tc;
    cc = baki;
    tc = baks;
    baks = tb;
    baki = bb;
    tb = ta;
    bb = aa;
    aa = baki;
    ta = baks;
}

inline void RotateTriLeft(unsigned int &aa,
        unsigned short &ta,
        unsigned int &bb,
        unsigned short &tb,
        unsigned int &cc,
        unsigned short &tc) {
    unsigned int baki;
    unsigned short baks;
    baks = ta;
    baki = aa;
    aa = bb;
    ta = tb;
    bb = baki;
    tb = baks;
    baks = tc;
    baki = cc;
    tc = ta;
    cc = aa;
    aa = baki;
    ta = baks;
}

void quadsquare::tri(unsigned int aa,
        unsigned short ta,
        unsigned int bb,
        unsigned short tb,
        unsigned int cc,
        unsigned short tc) {
    assert(0);     //see below #if VERTEX_LIST functions... this whole contraption sorely needs a rewrite
#ifdef DONOTDRAWBLENDEDQUADS
    if (ta == tb && tb == tc) {
        indices[ta]->q.push_back( aa, bb, cc );
        return;
    } else {
        return;
    }
#endif
    if (!(ta == tb && tb == tc)) {
        if (((*textures)[ta].blendDst == ZERO) && ((*textures)[tb].blendDst != ZERO)) {
            RotateTriRight(aa, ta, bb, tb, cc, tc);
        } else if (((*textures)[ta].blendDst == ZERO) && ((*textures)[tc].blendDst != ZERO)) {
            RotateTriLeft(aa, ta, bb, tb, cc, tc);
        }
        GFXColorVertex cv[3];
#ifdef VERTEX_LIST
        cv[0].SetVtx( *vertices->GetVertex( aa ) );
        cv[1].SetVtx( *vertices->GetVertex( bb ) );
        cv[2].SetVtx( *vertices->GetVertex( cc ) );
#endif
        cv[0].SetColor(GFXColor(1, 1, 1, 1));
        cv[1].SetColor(GFXColor(1, 1, 1, 1));
        cv[2].SetColor(GFXColor(1, 1, 1, 1));
        if (tb == tc) {
            cv[0].a = 0;
            cv[1].a = 1;
            cv[2].a = 1;
            indices[tb].c.push_back3(cv);
        } else {
            if (tb != ta) {
                cv[0].a = 0;
                cv[1].a = 1;
                cv[2].a = 0;
                indices[tb].c.push_back3(cv);
            }
            if (tc != ta) {
                cv[0].a = 0;
                cv[1].a = 0;
                cv[2].a = 1;
                indices[tc].c.push_back3(cv);
            }
        }
    }
    indices[ta].q.push_back(aa, bb, cc);
}

unsigned short VertInfo::GetTex() const {
    return (Rem > 127) ? (Tex + 1) : Tex;
    //return Tex/texmultiply + (((Tex%texmultiply)>texmultiply/2)?1:0);
}

void quadsquare::RenderAux(const quadcornerdata &cd, CLIPSTATE vis) {
//Does the work of rendering this square.  Uses the enabled vertices only.
//Recurses as necessary.
    unsigned int whole = 2 << cd.Level;
    SphereTransformRenderlevel++;
    //If this square is outside the frustum, then don't render it.
    if (vis != GFX_TOTALLY_VISIBLE) {
        Vector min, max;
        min.i = cd.xorg;
        min.j = MinY;
        min.k = cd.zorg;
        max.i = cd.xorg + whole;
        max.j = MaxY;
        max.k = cd.zorg + whole;
        vis = nonlinear_trans->BoxInFrustum(min, max, quadsquare::camerapos);
        if (vis == GFX_NOT_VISIBLE) {
            SphereTransformRenderlevel--;
            //This square is completely outside the view frustum.
            return;
        }
    }
    int i;

    int flags = 0;
    int mask = 1;
    quadcornerdata q;
    for (i = 0; i < 4; i++, mask <<= 1) {
        if (EnabledFlags & (16 << i)) {
            SetupCornerData(&q, cd, i);
            Child[i]->RenderAux(q, vis);
        } else {
            flags |= mask;
        }
    }
    SphereTransformRenderlevel--;
    if (flags == 0) {
        return;
    }
//Local macro to make the triangle logic shorter & hopefully clearer.
    //#define tri(aa,ta,bb,tb,cc,tc) (indices[ta].q.push_back (aa), indices[ta].q.push_back (bb), indices[ta].q.push_back (cc))
#define V0 (Vertex[0].vertindex)
#define T0 ( Vertex[0].GetTex() )
#define V1 (Vertex[1].vertindex)
#define T1 ( Vertex[1].GetTex() )
#define V2 (cd.Verts[0].vertindex)
#define T2 ( cd.Verts[0].GetTex() )
#define V3 (Vertex[2].vertindex)
#define T3 ( Vertex[2].GetTex() )
#define V4 (cd.Verts[1].vertindex)
#define T4 ( cd.Verts[1].GetTex() )
#define V5 (Vertex[3].vertindex)
#define T5 ( Vertex[3].GetTex() )
#define V6 (cd.Verts[2].vertindex)
#define T6 ( cd.Verts[2].GetTex() )
#define V7 (Vertex[4].vertindex)
#define T7 ( Vertex[4].GetTex() )
#define V8 (cd.Verts[3].vertindex)
#define T8 ( cd.Verts[3].GetTex() )
    //Make the list of triangles to draw.
    if ((EnabledFlags & 1) == 0) {
        tri(V0, T0, V8, T8, V2, T2);
    } else {
        if (flags & 8) {
            tri(V0, T0, V8, T8, V1, T1);
        }
        if (flags & 1) {
            tri(V0, T0, V1, T1, V2, T2);
        }
    }
    if ((EnabledFlags & 2) == 0) {
        tri(V0, T0, V2, T2, V4, T4);
    } else {
        if (flags & 1) {
            tri(V0, T0, V2, T2, V3, T3);
        }
        if (flags & 2) {
            tri(V0, T0, V3, T3, V4, T4);
        }
    }
    if ((EnabledFlags & 4) == 0) {
        tri(V0, T0, V4, T4, V6, T6);
    } else {
        if (flags & 2) {
            tri(V0, T0, V4, T4, V5, T5);
        }
        if (flags & 4) {
            tri(V0, T0, V5, T5, V6, T6);
        }
    }
    if ((EnabledFlags & 8) == 0) {
        tri(V0, T0, V6, T6, V8, T8);
    } else {
        if (flags & 4) {
            tri(V0, T0, V6, T6, V7, T7);
        }
        if (flags & 8) {
            tri(V0, T0, V7, T7, V8, T8);
        }
    }
#undef V1
#undef V2
#undef V3
#undef V4
#undef V5
#undef V6
#undef V7
#undef V8
#undef T1
#undef T2
#undef T3
#undef T4
#undef T5
#undef t6
#undef T7
#undef T8
}

void quadsquare::SetupCornerData(quadcornerdata *q, const quadcornerdata &cd, int ChildIndex) {
//Fills the given structure with the appropriate corner values for the
//specified child block, given our own vertex data and our corner
//vertex data from cd.
//
//ChildIndex mapping:
//+-+-+
//|1|0|
//+-+-+
//|2|3|
//+-+-+
//
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
    int half = 1 << cd.Level;
    q->Parent = &cd;
    q->Square = Child[ChildIndex];
    q->Level = cd.Level - 1;
    q->ChildIndex = ChildIndex;
    switch (ChildIndex) {
        default:
        case 0:
            q->xorg = cd.xorg + half;
            q->zorg = cd.zorg;
            q->Verts[0] = cd.Verts[0];
            q->Verts[1] = Vertex[2];
            q->Verts[2] = Vertex[0];
            q->Verts[3] = Vertex[1];
            break;
        case 1:
            q->xorg = cd.xorg;
            q->zorg = cd.zorg;
            q->Verts[0] = Vertex[2];
            q->Verts[1] = cd.Verts[1];
            q->Verts[2] = Vertex[3];
            q->Verts[3] = Vertex[0];
            break;
        case 2:
            q->xorg = cd.xorg;
            q->zorg = cd.zorg + half;
            q->Verts[0] = Vertex[0];
            q->Verts[1] = Vertex[3];
            q->Verts[2] = cd.Verts[2];
            q->Verts[3] = Vertex[4];
            break;
        case 3:
            q->xorg = cd.xorg + half;
            q->zorg = cd.zorg + half;
            q->Verts[0] = Vertex[1];
            q->Verts[1] = Vertex[0];
            q->Verts[2] = Vertex[4];
            q->Verts[3] = cd.Verts[3];
            break;
    }
}

