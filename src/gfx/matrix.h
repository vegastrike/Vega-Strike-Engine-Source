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

#include "vec.h"

typedef float Matrix[16];

const Matrix identity_matrix = {1,0,0,0,
				    0,1,0,0,
				    0,0,1,0,
				    0,0,0,1};

/** moves a vector struct to a matrix */

inline void ScaleMatrix(float matrix[], const Vector &scale) {
  matrix[0]*=scale.i;
  matrix[1]*=scale.i;
  matrix[2]*=scale.i;
  matrix[4]*=scale.j;
  matrix[5]*=scale.j;
  matrix[6]*=scale.j;
  matrix[8]*=scale.k;
  matrix[9]*=scale.k;
  matrix[10]*=scale.k;
}

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

inline void VectorAndPositionToMatrix(float matrix[], const Vector &v1, const Vector &v2, const Vector &v3, const Vector &pos) {

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
	
	matrix[12] = pos.i;
	matrix[13] = pos.j;
	matrix[14] = pos.k;
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

inline void RotateAxisAngle(float tmp[], const Vector &axis, const float angle) {
  float c = cosf (angle);
  float s = sinf (angle);
#define M(a,b) (tmp[b*4+a])
                M(0,0)=axis.i*axis.i*(1-c)+c;
                M(0,1)=axis.i*axis.j*(1-c)-axis.k*s;
                M(0,2)=axis.i*axis.k*(1-c)+axis.j*s;
                M(0,3)=0;
                M(1,0)=axis.j*axis.i*(1-c)+axis.k*s;
                M(1,1)=axis.j*axis.j*(1-c)+c;
                M(1,2)=axis.j*axis.k*(1-c)-axis.i*s;
                M(1,3)=0;
                M(2,0)=axis.i*axis.k*(1-c)-axis.j*s;
                M(2,1)=axis.j*axis.k*(1-c)+axis.i*s;
                M(2,2)=axis.k*axis.k*(1-c)+c;
                M(2,3)=0;
                M(3,0)=0;
                M(3,1)=0;
                M(3,2)=0;
                M(3,3)=1;
#undef M
}

inline void Translate(float matrix[], const Vector &v) {
  Translate(matrix, v.i, v.j, v.k);
}

/** Multiplies m1 and m2 and pops the result into dest;
 *  dest != m1, dest !=m2
 */
inline void MultMatrix(float dest[], const float m1[], const float m2[])
{
  dest[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
  dest[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
  dest[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
  dest[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

  dest[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
  dest[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
  dest[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
  dest[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

  dest[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
  dest[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
  dest[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
  dest[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

  dest[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
  dest[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
  dest[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
  dest[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];

  /*	for(int rowcount = 0; rowcount<4; rowcount++)
		for(int colcount = 0; colcount<4; colcount++)
			for(int mcount = 0; mcount <4; mcount ++)
			dest[colcount*4+rowcount] += m1[mcount*4+rowcount]*m2[colcount*4+mcount];
  */
}
/**
 * Copies Matrix source into the destination Matrix 
 */ 
inline void CopyMatrix(Matrix dest, const Matrix source)
{
	dest[0]=source[0];
	dest[1]=source[1];
		dest[2]=source[2];
			dest[3]=source[3];
	dest[4]=source[4];
	dest[5]=source[5];
	dest[6]=source[6];
	dest[7]=source[7];
	dest[8]=source[8];
	dest[9]=source[9];
	dest[10]=source[10];
	dest[11]=source[11];
	dest[12]=source[12];
	dest[13]=source[13];
	dest[14]=source[14];
	dest[15]=source[15];

  //memcpy(dest, source, sizeof(Matrix));
  /*
	for(int matindex = 0; matindex<16; matindex++)
		dest[matindex] = source[matindex];
  */
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
inline Vector InvTransformNormal (const Matrix t, const Vector & v) {

#define M(A,B) t[B*4+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}
inline Vector InvTransform (const Matrix t, const Vector & v) {
  return InvTransformNormal (t,  Vector (v.i-t[12], v.j-t[13], v.k-t[14]));
}

inline Vector Transform (const Matrix t, const float x, const float y, const float z) {
  //    Vector tLocation (t[12],t[13],t[14]);
//    Vector tP (t[0],t[1],t[2]);//the p vector of the plane being selected on
//    Vector tQ (t[4],t[5],t[6]);//the q vector of the plane being selected on
//    Vector tR (t[8],t[9],t[10]);//the q vector of the plane being selected on
  return Vector (t[12]+x*t[0]+y*t[4]+z*t[8],
		 t[13]+x*t[1]+y*t[5]+z*t[9],
		 t[14]+x*t[2]+y*t[6]+z*t[10]);
}

inline Vector TransformNormal (const Matrix t, const Vector & v) {
  return Vector (v.i*t[0]+v.j*t[4]+v.k*t[8],
		 v.i*t[1]+v.j*t[5]+v.k*t[9],
		 v.i*t[2]+v.j*t[6]+v.k*t[10]);
}
inline Vector TransformNormal (const Matrix t, const float x, const float y, const float z) {
  return Vector (x*t[0]+y*t[4]+z*t[8],
		 x*t[1]+y*t[5]+z*t[9],
		 x*t[2]+y*t[6]+z*t[10]);
}
int invert (float b[], float a[]);

inline void MatrixToVectors (const Matrix m,Vector &p,Vector&q,Vector&r, Vector &c) {
  p.Set (m[0],m[1],m[2]);
  q.Set (m[4],m[5],m[6]);
  r.Set (m[8],m[9],m[10]);
  c.Set (m[12],m[13],m[14]);
}

inline Vector InvScaleTransform (Matrix trans,  Vector pos) {
  pos = pos - Vector (trans[12],trans[13],trans[14]);
#define a (trans[0])
#define b (trans[4])
#define c (trans[8])
#define d (trans[1])
#define e (trans[5])
#define f (trans[9])
#define g (trans[2])
#define h (trans[6])
#define i (trans[10])
  float factor = 1.0F/(-c*e*g+ b*f*g + c*d*h - a*f*h - b*d*i + a*e*i);
  return (Vector(pos.Dot (Vector (e*i- f*h,c*h-b*i,b*f-c*e)),pos.Dot (Vector (f*g-d*i,a*i-c*g, c*d-a*f)),pos.Dot (Vector (d*h-e*g, b*g-a*h, a*e-b*d)))*factor);
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
}
inline void InvertMatrix (Matrix o, Matrix trans) {
#define a (trans[0])
#define b (trans[4])
#define c (trans[8])
#define d (trans[1])
#define e (trans[5])
#define f (trans[9])
#define g (trans[2])
#define h (trans[6])
#define i (trans[10])
  float factor = 1.0F/(-c*e*g+ b*f*g + c*d*h - a*f*h - b*d*i + a*e*i);
  o[0]=factor*(e*i- f*h);
  o[4]=factor*(c*h-b*i);
  o[8]=factor*(b*f-c*e);
  o[1]=factor*(f*g-d*i);
  o[5]=factor*(a*i-c*g);
  o[9]=factor*(c*d-a*f);
  o[2]=factor*(d*h-e*g);
  o[6]=factor*(b*g-a*h);
  o[10]=factor*(a*e-b*d);
  o[3]=0;
  o[7]=0;
  o[11]=0;
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i

  Vector pos (TransformNormal (o,Vector (-trans[12],-trans[13],-trans[14])));
  o[12]=pos.i;
  o[13]=pos.j;
  o[14]=pos.k;
  o[15]=1;

}

inline void Rotate (Matrix tmp, const Vector &axis, float angle) {
                double c = cos (angle);
                double s = sin (angle);
//Row, COl
#define M(a,b) (tmp[b*4+a])
                M(0,0)=axis.i*axis.i*(1-c)+c;
                M(0,1)=axis.i*axis.j*(1-c)-axis.k*s;
                M(0,2)=axis.i*axis.k*(1-c)+axis.j*s;
          M(0,3)=0;
                M(1,0)=axis.j*axis.i*(1-c)+axis.k*s;
                M(1,1)=axis.j*axis.j*(1-c)+c;
                M(1,2)=axis.j*axis.k*(1-c)-axis.i*s;
                M(1,3)=0;
                M(2,0)=axis.i*axis.k*(1-c)-axis.j*s;
                M(2,1)=axis.j*axis.k*(1-c)+axis.i*s;
                M(2,2)=axis.k*axis.k*(1-c)+c;
                M(2,3)=0;
                M(3,0)=0;
                M(3,1)=0;
                M(3,2)=0;
                M(3,3)=1;
#undef M
}

#endif
