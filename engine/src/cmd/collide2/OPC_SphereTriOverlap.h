/*
 * OPC_SphereTriOverlap.h
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
// NO HEADER GUARD

// This is collision detection. If you do another distance test for collision *response*,
// if might be useful to simply *skip* the test below completely, and report a collision.
// - if sphere-triangle overlap, result is ok
// - if they don't, we'll discard them during collision response with a similar test anyway
// Overall this approach should run faster.

// Original code by David Eberly in Magic.
bool SphereCollider::SphereTriOverlap(const Point &vert0, const Point &vert1, const Point &vert2) {
    // Stats
    mNbVolumePrimTests++;

    // Early exit if one of the vertices is inside the sphere
    Point kDiff = vert2 - mCenter;
    float fC = kDiff.SquareMagnitude();
    if (fC <= mRadius2) {
        return TRUE;
    }

    kDiff = vert1 - mCenter;
    fC = kDiff.SquareMagnitude();
    if (fC <= mRadius2) {
        return TRUE;
    }

    kDiff = vert0 - mCenter;
    fC = kDiff.SquareMagnitude();
    if (fC <= mRadius2) {
        return TRUE;
    }

    // Else do the full distance test
    Point TriEdge0 = vert1 - vert0;
    Point TriEdge1 = vert2 - vert0;

//Point kDiff	= vert0 - mCenter;
    float fA00 = TriEdge0.SquareMagnitude();
    float fA01 = TriEdge0 | TriEdge1;
    float fA11 = TriEdge1.SquareMagnitude();
    float fB0 = kDiff | TriEdge0;
    float fB1 = kDiff | TriEdge1;
//float fC	= kDiff.SquareMagnitude();
    float fDet = fabsf(fA00 * fA11 - fA01 * fA01);
    float u = fA01 * fB1 - fA11 * fB0;
    float v = fA01 * fB0 - fA00 * fB1;
    float SqrDist;

    if (u + v <= fDet) {
        if (u < 0.0f) {
            if (v < 0.0f)  // region 4
            {
                if (fB0 < 0.0f) {
//					v = 0.0f;
                    if (-fB0 >= fA00) { /*u = 1.0f;*/        SqrDist = fA00 + 2.0f * fB0 + fC;
                    } else {
                        u = -fB0 / fA00;
                        SqrDist = fB0 * u + fC;
                    }
                } else {
//					u = 0.0f;
                    if (fB1 >= 0.0f) { /*v = 0.0f;*/        SqrDist = fC;
                    } else if (-fB1 >= fA11) { /*v = 1.0f;*/        SqrDist = fA11 + 2.0f * fB1 + fC;
                    } else {
                        v = -fB1 / fA11;
                        SqrDist = fB1 * v + fC;
                    }
                }
            } else  // region 3
            {
//				u = 0.0f;
                if (fB1 >= 0.0f) { /*v = 0.0f;*/        SqrDist = fC;
                } else if (-fB1 >= fA11) { /*v = 1.0f;*/        SqrDist = fA11 + 2.0f * fB1 + fC;
                } else {
                    v = -fB1 / fA11;
                    SqrDist = fB1 * v + fC;
                }
            }
        } else if (v < 0.0f)  // region 5
        {
//			v = 0.0f;
            if (fB0 >= 0.0f) { /*u = 0.0f;*/        SqrDist = fC;
            } else if (-fB0 >= fA00) { /*u = 1.0f;*/        SqrDist = fA00 + 2.0f * fB0 + fC;
            } else {
                u = -fB0 / fA00;
                SqrDist = fB0 * u + fC;
            }
        } else  // region 0
        {
            // minimum at interior point
            if (fDet == 0.0f) {
//				u = 0.0f;
//				v = 0.0f;
                SqrDist = MAX_FLOAT;
            } else {
                float fInvDet = 1.0f / fDet;
                u *= fInvDet;
                v *= fInvDet;
                SqrDist = u * (fA00 * u + fA01 * v + 2.0f * fB0) + v * (fA01 * u + fA11 * v + 2.0f * fB1) + fC;
            }
        }
    } else {
        float fTmp0, fTmp1, fNumer, fDenom;

        if (u < 0.0f)  // region 2
        {
            fTmp0 = fA01 + fB0;
            fTmp1 = fA11 + fB1;
            if (fTmp1 > fTmp0) {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00 - 2.0f * fA01 + fA11;
                if (fNumer >= fDenom) {
//					u = 1.0f;
//					v = 0.0f;
                    SqrDist = fA00 + 2.0f * fB0 + fC;
                } else {
                    u = fNumer / fDenom;
                    v = 1.0f - u;
                    SqrDist = u * (fA00 * u + fA01 * v + 2.0f * fB0) + v * (fA01 * u + fA11 * v + 2.0f * fB1) + fC;
                }
            } else {
//				u = 0.0f;
                if (fTmp1 <= 0.0f) { /*v = 1.0f;*/        SqrDist = fA11 + 2.0f * fB1 + fC;
                } else if (fB1 >= 0.0f) { /*v = 0.0f;*/        SqrDist = fC;
                } else {
                    v = -fB1 / fA11;
                    SqrDist = fB1 * v + fC;
                }
            }
        } else if (v < 0.0f)  // region 6
        {
            fTmp0 = fA01 + fB1;
            fTmp1 = fA00 + fB0;
            if (fTmp1 > fTmp0) {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00 - 2.0f * fA01 + fA11;
                if (fNumer >= fDenom) {
//					v = 1.0f;
//					u = 0.0f;
                    SqrDist = fA11 + 2.0f * fB1 + fC;
                } else {
                    v = fNumer / fDenom;
                    u = 1.0f - v;
                    SqrDist = u * (fA00 * u + fA01 * v + 2.0f * fB0) + v * (fA01 * u + fA11 * v + 2.0f * fB1) + fC;
                }
            } else {
//				v = 0.0f;
                if (fTmp1 <= 0.0f) { /*u = 1.0f;*/        SqrDist = fA00 + 2.0f * fB0 + fC;
                } else if (fB0 >= 0.0f) { /*u = 0.0f;*/        SqrDist = fC;
                } else {
                    u = -fB0 / fA00;
                    SqrDist = fB0 * u + fC;
                }
            }
        } else  // region 1
        {
            fNumer = fA11 + fB1 - fA01 - fB0;
            if (fNumer <= 0.0f) {
//				u = 0.0f;
//				v = 1.0f;
                SqrDist = fA11 + 2.0f * fB1 + fC;
            } else {
                fDenom = fA00 - 2.0f * fA01 + fA11;
                if (fNumer >= fDenom) {
//					u = 1.0f;
//					v = 0.0f;
                    SqrDist = fA00 + 2.0f * fB0 + fC;
                } else {
                    u = fNumer / fDenom;
                    v = 1.0f - u;
                    SqrDist = u * (fA00 * u + fA01 * v + 2.0f * fB0) + v * (fA01 * u + fA11 * v + 2.0f * fB1) + fC;
                }
            }
        }
    }

    return fabsf(SqrDist) < mRadius2;
}
