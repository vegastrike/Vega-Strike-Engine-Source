/*
 * Vega Strike
 * Copyright (C) 2001-2002 Alan Shieh
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _MATRIX_H
#define _MATRIX_H

#include "gfx_transform_vector.h"

typedef float Matrix[16];
/** moves a vector struct to a matrix */
inline void VectorToMatrix(float matrix[], const Vector &v1, const Vector &v2, const Vector &v3)
{
	matrix[0] = v1.i;
	matrix[1] = v1.j;
	matrix[2] = v1.k;
	matrix[3] = 0;

	matrix[4] = v2.i;
	matrix[5] = v2.j;
	matrix[6] = v2.k;
	matrix[7] = 0;

	matrix[8] = v3.i;
	matrix[9] = v3.j;
	matrix[10] = v3.k;
	matrix[11] = 0;
	
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}

/** zeros out a 4x4 matrix quickly
 */
inline void Zero(float matrix[])
{
	matrix[0] = 0;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;

	matrix[4] = 0;
	matrix[5] = 0;
	matrix[6] = 0;
	matrix[7] = 0;

	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 0;
	matrix[11] = 0;
	
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 0;
}

/** Computes a 4x4 identity matrix
 */
inline void Identity(float matrix[])
{
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;

	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;

	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;
	
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}
/** Computes a Translation matrix based on x,y,z translation
 */

inline void Translate(float matrix[], float x, float y, float z)
{
	matrix[0] = 1;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;

	matrix[4] = 0;
	matrix[5] = 1;
	matrix[6] = 0;
	matrix[7] = 0;

	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;
	
	matrix[12] = x;
	matrix[13] = y;
	matrix[14] = z;
	matrix[15] = 1;
}

inline void Translate(float matrix[], const Vector &v) {
  Translate(matrix, v.i, v.j, v.k);
}

/** Multiplies m1 and m2 and pops the result into dest;
 *  dest != m1, dest !=m2
 */
inline void MultMatrix(float dest[], float m1[], float m2[])
{
	Zero(dest);
	for(int rowcount = 0; rowcount<4; rowcount++)
		for(int colcount = 0; colcount<4; colcount++)
			for(int mcount = 0; mcount <4; mcount ++)
			dest[colcount*4+rowcount] += m1[mcount*4+rowcount]*m2[colcount*4+mcount];
}
/**
 * Copies Matrix source into the destination Matrix 
 */ 
inline void CopyMatrix(Matrix dest, const Matrix source)
{
	for(int matindex = 0; matindex<16; matindex++)
		dest[matindex] = source[matindex];
}
/**
 * moves a vector in the localspace to world space through matrix t
*/
inline Vector Transform (const Matrix t, const Vector & v) {
  //    Vector tLocation (t[12],t[13],t[14]);
//    Vector tP (t[0],t[1],t[2]);//the p vector of the plane being selected on
//    Vector tQ (t[4],t[5],t[6]);//the q vector of the plane being selected on
//    Vector tR (t[8],t[9],t[10]);//the q vector of the plane being selected on
  return Vector (t[12]+v.i*t[0]+v.j*t[4]+v.k*t[8],
		 t[13]+v.i*t[1]+v.j*t[5]+v.k*t[9],
		 t[14]+v.i*t[2]+v.j*t[6]+v.k*t[10]);
}


int invert (float b[], float a[]);


#endif
