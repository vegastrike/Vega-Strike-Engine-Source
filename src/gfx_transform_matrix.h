#ifndef _MATRIX_H
#define _MATRIX_H

#include "gfx_transform_vector.h"

typedef float Matrix[16];

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

inline void MultMatrix(float dest[], float m1[], float m2[])
{
	Zero(dest);
	for(int rowcount = 0; rowcount<4; rowcount++)
		for(int colcount = 0; colcount<4; colcount++)
			for(int mcount = 0; mcount <4; mcount ++)
			dest[colcount*4+rowcount] += m1[mcount*4+rowcount]*m2[colcount*4+mcount];
}

inline void CopyMatrix(Matrix dest, const Matrix source)
{
	for(int matindex = 0; matindex<16; matindex++)
		dest[matindex] = source[matindex];
}

#endif