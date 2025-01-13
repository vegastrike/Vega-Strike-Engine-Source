/*
 * fastmath.cpp
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

/*****************************************************************************
 *  File:  fastmath.cpp
 *  This file is provided without support, instruction, or implied warranty of any
 *  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and is
 *  not liable under any circumstances for any damages or loss whatsoever arising
 *  from the use or inability to use this file or items derived from it.
 *  Comments:
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <wtypes.h>
#else
typedef int DWORD;
#endif

#define FP_BITS(fp) ( ( *(DWORD*) &(fp) ) )
#define FP_ABS_BITS(fp) ( (FP_BITS( fp )&0x7FFFFFFF) )
#define FP_SIGN_BIT(fp) ( (FP_BITS( fp )&0x80000000) )
#define FP_ONE_BITS ( (0x3F800000) )
#define __forceinline inline

//r = 1/p

#define FP_INV(r, p)                       \
    do {                                     \
        int _i = 2*FP_ONE_BITS-*(int*) &(p); \
        r = *(float*) &_i;                   \
        r = r*(2.0f-(p)*r);                  \
    }                                        \
    while (0)

/////////////////////////////////////////////////

//The following comes from Vincent Van Eeckhout
//Thanks for sending us the code!
//It's the same thing in assembly but without this C-needed line:
//r = *(float *)&_i;

float two = 2.0f;

#define FP_INV2(r, p)                \
    do {                               \
        __asm {mov eax, 0x7F000000};   \
        __asm {sub eax, dword ptr[p]}; \
        __asm {mov dword ptr[r], eax}; \
        __asm {fld dword ptr[p]};      \
        __asm {fmul dword ptr[r]};     \
        __asm {fsubr[two]};            \
        __asm {fmul dword ptr[r]};     \
        __asm {fstp dword ptr[r]};     \
    }                                  \
    while (0)

/////////////////////////////////////////////////

#define FP_EXP(e, p)                            \
    do {                                          \
        int _i;                                   \
        e  = -1.44269504f*(float) 0x00800000*(p); \
        _i = (int) e+0x3F800000;                  \
        e  = *(float*) &_i;                       \
    }                                             \
    while (0)

#define FP_NORM_TO_BYTE(i, p)         \
    do {                                \
        float _n = (p)+1.0f;            \
        i = *(int*) &_n;                \
        if (i >= 0x40000000) i = 0xFF;  \
        else if (i <= 0x3F800000)       \
            i = 0;                      \
        else i = ( (i)>>15 )&0xFF;      \
    }                                   \
    while (0)

inline unsigned long FP_NORM_TO_BYTE2(float p) {
    float fpTmp = p + 1.0f;
    return ((*(unsigned *) &fpTmp) >> 15) & 0xFF;
}

inline unsigned long FP_NORM_TO_BYTE3(float p) {
    float ftmp = p + 12582912.0f;
    return (*(unsigned long *) &ftmp) & 0xFF;
}

static unsigned int fast_sqrt_table[0x10000];  //declare table of square roots
typedef union FastSqrtUnion {
    float f;
    unsigned int i;
} FastSqrtUnion;

void build_sqrt_table() {
    unsigned int i;
    FastSqrtUnion s;
    for (i = 0; i <= 0x7FFF; i++) {
        //Build a float with the bit pattern i as mantissa
        //and an exponent of 0, stored as 127

        s.i = (i << 8) | (0x7F << 23);
        s.f = (float) sqrt(s.f);

        //Take the square root then strip the first 7 bits of
        //the mantissa into the table

        fast_sqrt_table[i + 0x8000] = (s.i & 0x7FFFFF);

        //Repeat the process, this time with an exponent of 1,
        //stored as 128

        s.i = (i << 8) | (0x80 << 23);
        s.f = (float) sqrt(s.f);

        fast_sqrt_table[i] = (s.i & 0x7FFFFF);
    }
}

inline float fastsqrt(float n) {
    if (FP_BITS(n) == 0) {
        return 0.0;
    }             //check for square root of 0

    FP_BITS(n) = fast_sqrt_table[(FP_BITS(n) >> 8) & 0xFFFF]
            | ((((FP_BITS(n) - 0x3F800000) >> 1) + 0x3F800000) & 0x7F800000);

    return n;
}

//At the assembly level the recommended workaround for the second FIST bug is the same for the first;
//inserting the FRNDINT instruction immediately preceding the FIST instruction.
__forceinline void FloatToInt(int *int_pointer, float f) {
    *int_pointer = f;
}

int Stupodmain(int argc, char *argv[]) {
    float t, it, test_sqrt;
    int i = 0;
    build_sqrt_table();
    t = 1234.121234f;

    test_sqrt = fastsqrt(t);
    printf("sqrt expected %20.10f  approx %20.10f\n", sqrt(t), test_sqrt);
    FP_INV(it, t);
    printf("inv  expected %20.10f  approx %20.10f\n", 1 / t, it);
    i = 0xdeafbabe;
    FloatToInt(&i, t);
    printf("ftol expected %d  actual %d %08X\n", (int) t, i, i);
    return 0;
}

/////////////////////////////////////////////////////////////////////
//3D and geometry ops /////////////////////////////////////////////
//-----------------------------------------------------------------------------
//Name: CylTest_CapsFirst
//Orig: Greg James - gjames@NVIDIA.com
//Lisc: Free code - no warranty & no money back.  Use it all you want
//Desc:
//This function tests if the 3D point 'testpt' lies within an arbitrarily
//oriented cylinder.  The cylinder is defined by an axis from 'pt1' to 'pt2',
//the axis having a length squared of 'lengthsq' (pre-compute for each cylinder
//to avoid repeated work!), and radius squared of 'radius_sq'.
//The function tests against the end caps first, which is cheap -> only
//a single dot product to test against the parallel cylinder caps.  If the
//point is within these, more work is done to find the distance of the point
//from the cylinder axis.
//Fancy Math (TM) makes the whole test possible with only two dot-products
//a subtract, and two multiplies.  For clarity, the 2nd mult is kept as a
//divide.  It might be faster to change this to a mult by also passing in
//1/lengthsq and using that instead.
//Elminiate the first 3 subtracts by specifying the cylinder as a base
//point on one end cap and a vector to the other end cap (pass in {dx,dy,dz}
//instead of 'pt2' ).
//
//The dot product is constant along a plane perpendicular to a vector.
//The magnitude of the cross product divided by one vector length is
//constant along a cylinder surface defined by the other vector as axis.
//
//Return:  -1.0 if point is outside the cylinder
//Return:  distance squared from cylinder axis if point is inside.
//
//-----------------------------------------------------------------------------

struct Vec3 {
    float x;
    float y;
    float z;
};

float CylTest_CapsFirst(const Vec3 &pt1, const Vec3 &pt2, float lengthsq, float radius_sq, const Vec3 &testpt) {
    float dx, dy, dz;           //vector d  from line segment point 1 to point 2
    float pdx, pdy, pdz;        //vector pd from point 1 to test point
    float dot, dsq;
    dx = pt2.x - pt1.x;          //translate so pt1 is origin.  Make vector from
    dy = pt2.y - pt1.y;          //pt1 to pt2.  Need for this is easily eliminated
    dz = pt2.z - pt1.z;
    pdx = testpt.x - pt1.x;               //vector from pt1 to test point.
    pdy = testpt.y - pt1.y;
    pdz = testpt.z - pt1.z;
    //Dot the d and pd vectors to see if point lies behind the
    //cylinder cap at pt1.x, pt1.y, pt1.z
    dot = pdx * dx + pdy * dy + pdz * dz;
    //If dot is less than zero the point is behind the pt1 cap.
    //If greater than the cylinder axis line segment length squared
    //then the point is outside the other end cap at pt2.
    if (dot < 0.0f || dot > lengthsq) {
        return -1.0f;
    } else {
        //Point lies within the parallel caps, so find
        //distance squared from point to line, using the fact that sin^2 + cos^2 = 1
        //the dot = cos() * |d||pd|, and cross*cross = sin^2 * |d|^2 * |pd|^2
        //Carefull: '*' means mult for scalars and dotproduct for vectors
        //In short, where dist is pt distance to cyl axis:
        //dist = sin( pd to d ) * |pd|
        //distsq = dsq = (1 - cos^2( pd to d)) * |pd|^2
        //dsq = ( 1 - (pd * d)^2 / (|pd|^2 * |d|^2) ) * |pd|^2
        //dsq = pd * pd - dot * dot / lengthsq
        //where lengthsq is d*d or |d|^2 that is passed into this function
        //distance squared to the cylinder axis:
        dsq = (pdx * pdx + pdy * pdy + pdz * pdz) - dot * dot / lengthsq;
        if (dsq > radius_sq) {
            return -1.0f;
        } else {
            return dsq;
        }                         //return distance squared to axis
    }
}

