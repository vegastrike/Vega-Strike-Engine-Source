/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
#include "gfx/matrix.h"
#include "gl_matrix.h"
#include "lin_time.h"
#include <stdio.h>
using namespace GFXMatrices;  //causes problems with g_game
double BoxFrust[6][4];
double frust[6][4];

float /*GFXDRVAPI*/ GFXSphereInFrustum(const QVector &Cnt, float radius) {
    return GFXSphereInFrustum(frust, Cnt, radius);
}

CLIPSTATE GFXBoxInFrustum(const Vector &min, const Vector &max) {
    return GFXBoxInFrustum(BoxFrust, min, max);
}

CLIPSTATE GFXTransformedBoxInFrustum(const Vector &min, const Vector &max) {
    return GFXBoxInFrustum(frust, min, max);
}

CLIPSTATE /*GFXDRVAPI*/ GFXSpherePartiallyInFrustum(const Vector &Cnt, float radius) {
    return GFXSpherePartiallyInFrustum(BoxFrust, Cnt, radius);
}

CLIPSTATE /*GFXDRVAPI*/ GFXTransformedSpherePartiallyInFrustum(const Vector &Cnt, float radius) {
    return GFXSpherePartiallyInFrustum(frust, Cnt, radius);
}

CLIPSTATE /*GFXDRVAPI*/ GFXSpherePartiallyInFrustum(double f[6][4], const Vector &Cnt, const float radius) {
    int p;
    float d;
    CLIPSTATE retval = GFX_TOTALLY_VISIBLE;
    for (p = 0; p < 6; p++) {
        //does not evaluate for yon
        d = f[p][0] * Cnt.i + f[p][1] * Cnt.j + f[p][2] * Cnt.k + f[p][3];
        if (d <= -radius) {
            return GFX_NOT_VISIBLE;
        } else if (d <= radius) {
            retval = GFX_PARTIALLY_VISIBLE;
        }
    }
    return retval;
}

CLIPSTATE GFXBoxInFrustum(double f[6][4], const Vector &min, const Vector &max) {
    //Doesn't do a perfect test for NOT_VISIBLE.  Just checks to
    //see if all box vertices are outside at least one frustum
    //plane.  Some pathological boxes could return SOME_CLIP even
    //though they're really fully outside the frustum.  But that
    //won't hurt us too much if it isn't a common case; the
    //contents will just be culled/clipped at a later stage in the
    //pipeline.

    //Check each vertex of the box against the view frustum, and compute
    //bit codes for whether the point is outside each plane.
    int OrCodes = 0, AndCodes = ~0;
    for (int i = 0; i < 8; i++) {
        Vector v(min.i, min.j, min.k);
        if (i & 1) {
            v.i = (max.i);
        }
        if (i & 2) {
            v.j = (max.j);
        }
        if (i & 4) {
            v.k = (max.k);
        }
        //Now check against the frustum planes.
        int Code = 0;
        int Bit = 1;
        for (int j = 0; j < 6; j++, Bit <<= 1) {
            if (v.i * f[j][0] + v.j * f[j][1] + v.k * f[j][2] + f[j][3] < 0) {
                //The point is outside this plane.
                Code |= Bit;
            }
        }
        OrCodes |= Code;
        AndCodes &= Code;
    }
    //Based on bit-codes, return culling results.
    if (OrCodes == 0) {
        //The box is completely within the frustum.
        return GFX_TOTALLY_VISIBLE;
    } else if (AndCodes != 0) {
        //All the points are outside one of the frustum planes.
        return GFX_NOT_VISIBLE;
    } else {
        return GFX_PARTIALLY_VISIBLE;
    }
}

void DrawFrustum(double f[6][4]) {
    GFXDisable(LIGHTING);
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXBlendMode(ONE, ONE);
    const GFXColor cols[6] = {
            GFXColor(0, 0, 1),
            GFXColor(0, 1, 0),
            GFXColor(1, 0, 0),
            GFXColor(1, 1, 0),
            GFXColor(1, 0, 1),
            GFXColor(0, 1, 1)
    };
    static VertexBuilder<float, 3, 0, 3> verts;
    verts.clear();
    for (unsigned int i = 0; i < 4; i++) {
        Vector n(f[i][0], f[i][1], f[i][2]);
        Vector r(9284, -3259, -1249);
        Vector t = n.Cross(r);
        Vector q = t.Cross(n);
        t.Normalize();
        q.Normalize();
        t = t * 10000;
        q = q * 10000;
        n = n * f[i][3];
        Vector a = t + n;
        Vector b = q + n;
        Vector c = n - t;
        Vector d = n - q;

        verts.insert(GFXColorVertex(a, cols[i]));
        verts.insert(GFXColorVertex(b, cols[i]));
        verts.insert(GFXColorVertex(c, cols[i]));
        verts.insert(GFXColorVertex(d, cols[i]));
        verts.insert(GFXColorVertex(d, cols[i]));
        verts.insert(GFXColorVertex(c, cols[i]));
        verts.insert(GFXColorVertex(b, cols[i]));
        verts.insert(GFXColorVertex(a, cols[i]));
    }
    GFXDraw(GFXQUAD, verts);
}

float /*GFXDRVAPI*/ GFXSphereInFrustum(double f[6][4], const QVector &Cnt, float radius) {
    /*
     *  static float lasttime = GetElapsedTime();
     *  if (lasttime!=GetElapsedTime()) {
     *  DrawFrustum (f);
     *  lasttime = GetElapsedTime();
     *  }*/
    int p;
    double d;
    for (p = 0; p < 5; p++) {
        //does not evaluate for yon
        d = f[p][0] * Cnt.i + f[p][1] * Cnt.j + f[p][2] * Cnt.k + f[p][3];
        if (d < 0) {
            if (d <= -radius) {
                return 0;
            }
        }
    }
    return d;
}

void GFXGetFrustumVars(bool retr, float *l, float *r, float *b, float *t, float *n, float *f) {
    static float nnear, ffar, left, right, bot, top;     //Visual C++ reserves near and far
    if (!retr) {
        nnear = *n;
        ffar = *f;
        left = *l;
        right = *r;
        bot = *b;
        top = *t;
    } else {
        *l = left;
        *r = right;
        *b = bot;
        *t = top;
        *n = nnear;
        *f = ffar;
    }
}

void /*GFXDRVAPI*/ GFXGetFrustum(double f[6][4]) {
    f = frust;
}

void /*GFXDRVAPI*/ GFXBoxInFrustumModel(const Matrix &model) {
    Matrix tmp;
    MultMatrix(tmp, view, model);
    GFXCalculateFrustum(BoxFrust, tmp, projection);
}

void /*GFXDRVAPI*/ GFXCalculateFrustum() {
    GFXCalculateFrustum(frust, view, projection);
}

void WackyMultFloatMatrix(double dest[], const float m1[], const Matrix &m2) {
    QVector p(InvTransformNormal(m2, m2.p));
    p = (TransformNormal(m2, -m2.p));
    //p=m2.p;
    dest[0] = m1[0] * (double) m2.r[0] + m1[4] * (double) m2.r[1] + m1[8] * (double) m2.r[2];
    dest[1] = m1[1] * (double) m2.r[0] + m1[5] * (double) m2.r[1] + m1[9] * (double) m2.r[2];
    dest[2] = m1[2] * (double) m2.r[0] + m1[6] * (double) m2.r[1] + m1[10] * (double) m2.r[2];
    dest[3] = m1[3] * (double) m2.r[0] + m1[7] * (double) m2.r[1] + m1[11] * (double) m2.r[2];

    dest[4] = m1[0] * (double) m2.r[3] + m1[4] * (double) m2.r[4] + m1[8] * (double) m2.r[5];
    dest[5] = m1[1] * (double) m2.r[3] + m1[5] * (double) m2.r[4] + m1[9] * (double) m2.r[5];
    dest[6] = m1[2] * (double) m2.r[3] + m1[6] * (double) m2.r[4] + m1[10] * (double) m2.r[5];
    dest[7] = m1[3] * (double) m2.r[3] + m1[7] * (double) m2.r[4] + m1[11] * (double) m2.r[5];

    dest[8] = m1[0] * (double) m2.r[6] + m1[4] * (double) m2.r[7] + m1[8] * (double) m2.r[8];
    dest[9] = m1[1] * (double) m2.r[6] + m1[5] * (double) m2.r[7] + m1[9] * (double) m2.r[8];
    dest[10] = m1[2] * (double) m2.r[6] + m1[6] * (double) m2.r[7] + m1[10] * (double) m2.r[8];
    dest[11] = m1[3] * (double) m2.r[6] + m1[7] * (double) m2.r[7] + m1[11] * (double) m2.r[8];

    dest[12] = m1[0] * p.i + m1[4] * p.j + m1[8] * p.k + m1[12];
    dest[13] = m1[1] * p.i + m1[5] * p.j + m1[9] * p.k + m1[13];
    dest[14] = m1[2] * p.i + m1[6] * p.j + m1[10] * p.k + m1[14];
    dest[15] = m1[3] * p.i + m1[7] * p.j + m1[11] * p.k + m1[15];
}

void /*GFXDRVAPI*/ GFXCalculateFrustum(double frustum[6][4], const Matrix &modl, const float *proj) {
    double clip[16];
    WackyMultFloatMatrix(clip, proj, modl);
    double t;
    /* Extract the numbers for the RIGHT plane */
    frustum[0][0] = clip[3] - clip[0];
    frustum[0][1] = clip[7] - clip[4];
    frustum[0][2] = clip[11] - clip[8];
    frustum[0][3] = clip[15] - clip[12];

    /* Normalize the result */
    t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
    frustum[0][0] /= t;
    frustum[0][1] /= t;
    frustum[0][2] /= t;
    frustum[0][3] /= t;

    /* Extract the numbers for the LEFT plane */
    frustum[1][0] = clip[3] + clip[0];
    frustum[1][1] = clip[7] + clip[4];
    frustum[1][2] = clip[11] + clip[8];
    frustum[1][3] = clip[15] + clip[12];

    /* Normalize the result */
    t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
    frustum[1][0] /= t;
    frustum[1][1] /= t;
    frustum[1][2] /= t;
    frustum[1][3] /= t;

    /* Extract the BOTTOM plane */
    frustum[2][0] = clip[3] + clip[1];
    frustum[2][1] = clip[7] + clip[5];
    frustum[2][2] = clip[11] + clip[9];
    frustum[2][3] = clip[15] + clip[13];

    /* Normalize the result */
    t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
    frustum[2][0] /= t;
    frustum[2][1] /= t;
    frustum[2][2] /= t;
    frustum[2][3] /= t;

    /* Extract the TOP plane */
    frustum[3][0] = clip[3] - clip[1];
    frustum[3][1] = clip[7] - clip[5];
    frustum[3][2] = clip[11] - clip[9];
    frustum[3][3] = clip[15] - clip[13];

    /* Normalize the result */
    t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
    frustum[3][0] /= t;
    frustum[3][1] /= t;
    frustum[3][2] /= t;
    frustum[3][3] /= t;

    /* Extract the FAR plane */
    frustum[5][0] = clip[3] - clip[2];
    frustum[5][1] = clip[7] - clip[6];
    frustum[5][2] = clip[11] - clip[10];
    frustum[5][3] = clip[15] - clip[14];

    /* Normalize the result */
    t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
    frustum[5][0] /= t;
    frustum[5][1] /= t;
    frustum[5][2] /= t;
    frustum[5][3] /= t;

    /* Extract the NEAR plane */
    frustum[4][0] = clip[3] + clip[2];
    frustum[4][1] = clip[7] + clip[6];
    frustum[4][2] = clip[11] + clip[10];
    frustum[4][3] = clip[15] + clip[14];

    /* Normalize the result */
    t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
    frustum[4][0] /= t;
    frustum[4][1] /= t;
    frustum[4][2] /= t;
    frustum[4][3] /= t;
}

/**
 * GFXGetZPerspective () returns the relative scale of an object placed
 *      at distance z from the camera with the current projection matrix.
 */
float GFXGetZPerspective(const float z) {
    /*
     *
     * | xs 0  a 0 |[x]   [xs + az]          [1/xs 0   0  a/xs][x]   [x/xs+ aw/xs]
     * | 0  ys b 0 |[y] = [ys + bz]    ^-1   [ 0  1/ys 0  b/ys][y] = [y/ys+ bw/ys]
     * | 0  0  c d |[z]   [cz + dw]          [ 0   0   0  -1  ][z]   [0          ]
     * | 0  0 -1 0 |[w]   [-z     ]          [ 0   0  1/d c/d ][w]   [z/d + cw/d ]
     *
     */

    float left, right, bottom, top, nearval, farval;
    GFXGetFrustumVars(true, &left, &right, &bottom, &top, &nearval, &farval);

    float xs = 2 * nearval / (right - left);
    float a = (right + left) / (right - left);

    //Compute homogeneus x,w for (1,0,z,0)
    float hx = xs + z * a;
    float hw = -z;

    //Translate into euclidean coordinates and return euclidean x
    return fabs(hx / hw);
}

#if 0
PROJECTION DOESNt FIT INTO REDUCED MAT
/* Extract the numbers for the RIGHT plane */
    frustum[0][0] = /*clip[ 3]*/ -clip.r[0];
frustum[0][1] = /*clip[ 7]*/ -clip.r[3];
frustum[0][2] = /*clip[11]*/ -clip.r[6];
frustum[0][3] = /*clip[15]*/ 1-clip.p.i;

/* Normalize the result */
t = sqrt( frustum[0][0]*frustum[0][0]+frustum[0][1]*frustum[0][1]+frustum[0][2]*frustum[0][2] );
frustum[0][0] /= t;
frustum[0][1] /= t;
frustum[0][2] /= t;
frustum[0][3] /= t;

/* Extract the numbers for the LEFT plane */
frustum[1][0]  = /*clip[ 3]*/ +clip.r[0];
frustum[1][1]  = /*clip[ 7]*/ +clip.r[3];
frustum[1][2]  = /*clip[11]*/ +clip.r[6];
frustum[1][3]  = /*clip[15]*/ 1+clip.p.i;

/* Normalize the result */
t = sqrt( frustum[1][0]*frustum[1][0]+frustum[1][1]*frustum[1][1]+frustum[1][2]*frustum[1][2] );
frustum[1][0] /= t;
frustum[1][1] /= t;
frustum[1][2] /= t;
frustum[1][3] /= t;

/* Extract the BOTTOM plane */
frustum[2][0]  = /*clip[ 3]*/ +clip.r[1];
frustum[2][1]  = /*clip[ 7]*/ +clip.r[4];
frustum[2][2]  = /*clip[11]*/ +clip.r[7];
frustum[2][3]  = /*clip[15]*/ 1+clip.p.j;

/* Normalize the result */
t = sqrt( frustum[2][0]*frustum[2][0]+frustum[2][1]*frustum[2][1]+frustum[2][2]*frustum[2][2] );
frustum[2][0] /= t;
frustum[2][1] /= t;
frustum[2][2] /= t;
frustum[2][3] /= t;

/* Extract the TOP plane */
frustum[2][0]  = /*clip.r[ 3]*/ -clip.r[1];
frustum[2][1]  = /*clip[ 7]*/ -clip.r[4];
frustum[2][2]  = /*clip[11]*/ -clip.r[7];
frustum[2][3]  = /*clip[15]*/ 1-clip.p.j;

/* Normalize the result */
t = sqrt( frustum[3][0]*frustum[3][0]+frustum[3][1]*frustum[3][1]+frustum[3][2]*frustum[3][2] );
frustum[3][0] /= t;
frustum[3][1] /= t;
frustum[3][2] /= t;
frustum[3][3] /= t;

/* Extract the FAR plane */
frustum[5][0]  = /*clip.r[ 3]*/ -clip.r[2];
frustum[5][1]  = /*clip[ 7]*/ -clip.r[5];
frustum[5][2]  = /*clip[11]*/ -clip.r[8];
frustum[5][3]  = /*clip[15]*/ 1-clip.p.k;

/* Normalize the result */
t = sqrt( frustum[5][0]*frustum[5][0]+frustum[5][1]*frustum[5][1]+frustum[5][2]*frustum[5][2] );
frustum[5][0] /= t;
frustum[5][1] /= t;
frustum[5][2] /= t;
frustum[5][3] /= t;

/* Extract the NEAR plane */
frustum[4][0]  = /*clip[ 3]*/ +clip.r[2];
frustum[4][1]  = /*clip[ 7]*/ +clip.r[5];
frustum[4][2]  = /*clip[11]*/ +clip.r[8];
frustum[4][3]  = /*clip[15]*/ 1+clip.p.k;

/* Normalize the result */
t = sqrt( frustum[4][0]*frustum[4][0]+frustum[4][1]*frustum[4][1]+frustum[4][2]*frustum[4][2] );
frustum[4][0] /= t;
frustum[4][1] /= t;
frustum[4][2] /= t;
frustum[4][3] /= t;

#endif

