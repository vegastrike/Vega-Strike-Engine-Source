/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#include "gfxlib.h"
#include "gfx_transform_vector.h"
//typedef float GLdouble;
#include <math.h>
#include <string.h>
const float PI=3.1415926536;
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


float Magnitude(Vector v)
{
	return sqrt(v.i*v.i + v.j*v.j + v.k*v.k);
}

Vector CrossProduct(const Vector& v1, const Vector& v2)
{
	Vector result;
    result.i = v1.j * v2.k - v1.k * v2.j;
    result.j = v1.k * v2.i - v1.i * v2.k;
    result.k = v1.i * v2.j  - v1.j * v2.i;     
	return result;
}
/*
Vector operator*(float left, const Vector &right)
{
	return Vector(right.i*left, right.j*left, right.k*left);
}*/

float DotProduct(Vector &a, Vector &b)
{
	return a.Dot(b);
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

static Matrix model, view, projection;

Vector eye, center, up;

BOOL /*GFXDRVAPI*/ GFXMultMatrix(MATRIXMODE mode, Matrix matrix)
{
	Matrix t;
	switch(mode)
	{
	case MODEL:
		MultMatrix(t, model, matrix);
		CopyMatrix(model, t);
		MultMatrix(t, view, model);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(t);
		break;
	case VIEW:
		MultMatrix(t, view, matrix);
		CopyMatrix(view, t);
		MultMatrix(t, view, model);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(t);
		break;
	case PROJECTION:
		MultMatrix(t, projection, matrix);
		CopyMatrix(projection, t);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projection);
		break;
	}
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXLoadMatrix(MATRIXMODE mode, Matrix matrix)
{
	Matrix t;
	switch(mode)
	{
	case MODEL:
		CopyMatrix(model, matrix);
		MultMatrix(t, view, model);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(t);
		break;
	case VIEW:
		CopyMatrix(view, matrix);
		MultMatrix(t, view, model);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(t);
		break;
	case PROJECTION:
		CopyMatrix(projection, matrix);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projection);
		break;
	}
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXLoadIdentity(MATRIXMODE mode)
{
	switch(mode)
	{
	case MODEL:
		Identity(model);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(view);
		break;
	case VIEW:
		Identity(view);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(model);
		break;
	case PROJECTION:
		Identity(projection);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		break;
	}
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXGetMatrix(MATRIXMODE mode, Matrix matrix)
{
	switch(mode)
	{
	case MODEL:
		CopyMatrix(matrix, model);
		break;
	case VIEW:
		CopyMatrix(matrix, view);
		break;
	case PROJECTION:
		CopyMatrix(matrix, projection);
		break;
	}
	return TRUE;
}

static void gl_Frustum( 
                 float left, float right,
	 	 float bottom, float top,
		 float nearval, float farval )
{
   GLfloat x, y, a, b, c, d;
   GLfloat *m = projection;

   x = (2.0*nearval) / (right-left);
   y = (2.0*nearval) / (top-bottom);
   a = (right+left) / (right-left);
   b = (top+bottom) / (top-bottom);
   c = -(farval+nearval) / ( farval-nearval);
   d = -(2.0*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
}

BOOL /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar)
{
   float xmin, xmax, ymin, ymax;

   ymax = znear * tan( fov * PI / 360.0 );
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   gl_Frustum( xmin, xmax, ymin, ymax, znear, zfar );
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(projection);
   //glMatrixMode(GL_PROJECTION);
   //glFrustum(xmin,xmax,ymin,ymax,znear,zfar);

   return TRUE;

/*	float fov_vert = fov*(PI/180);
	float fov_horiz = aspect*fov_vert; //I HOPE this is right
									//YO IF SOMETHING GOES WRONG WITH THIS CHECK THIS LINE FIRST!!!
	float near_plane = znear;
	float far_plane = zfar;

    float    h, w, Q;
 
    w = 1.00F/(float)(tan(fov_horiz*0.50)); // modified
    h = 1.00F/(float)(tan(fov_vert*0.50));

    Q = far_plane/(far_plane - near_plane);
 
    //D3DMATRIX ret = ZeroMatrix();
	ZeroMemory(projection, sizeof(projection));
#define M(row, col) projection[col*4+row]
    M(0, 0) = -w;
    M(1, 1) = h;
    M(2, 2) = Q;
    M(3, 2) = -Q*near_plane;
    M(2, 3) = 1;
#undef M
	glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(projection);
	return TRUE;
*/
/*	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, 1, -1, 0.10, 10);
	return TRUE;
*/
}

BOOL /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float znear, float zfar)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, znear, zfar);
	return TRUE;
}


static void LookAtHelper( float eyex, float eyey, float eyez,
                         float centerx, float centery, float centerz,
                         float upx, float upy, float upz )
{
   float m[16];
   float x[3], y[3], z[3];
   float mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
   if (mag) {  /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   //x[0] =  z[1]*y[2] - z[2]*y[1];
   //x[1] = -z[0]*y[2] + z[2]*y[0];
   //x[2] =  z[0]*y[1] - z[1]*y[0];
   x[0] =  y[1]*z[2] - y[2]*z[1];
   x[1] = -y[0]*z[2] + y[2]*z[0];
   x[2] =  y[0]*z[1] - y[1]*z[0];

   /* Recompute Y = Z cross X */
   //y[0] =  x[1]*z[2] - x[2]*z[1];
   //y[1] = -x[0]*z[2] + x[2]*z[0];
   //y[2] =  x[0]*z[1] - x[1]*z[0];
   y[0] =  z[1]*x[2] - z[2]*x[1];
   y[1] = -z[0]*x[2] + z[2]*x[0];
   y[2] =  z[0]*x[1] - z[1]*x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;

   float tm[16];
#ifdef WIN32	
   ZeroMemory(tm, sizeof(tm));
#else
   bzero (tm, sizeof (tm));
#endif

#undef M

#define M(row,col)  tm[col*4+row]
   M(0,0) = 1.0;
   M(0,3) = -eyex;
   M(1,1) = 1.0;
   M(1,3) = -eyey;
   M(2,2) = 1.0;
   M(2,3) = -eyez;
   M(3,3) = 1.0;
#undef M

   MultMatrix(view, m, tm);
 //  glMultMatrixd( m );

   /* Translate Eye to Origin */
  // glTranslated( -eyex, -eyey, -eyez );

}

BOOL /*GFXDRVAPI*/ GFXLookAt(Vector eye, Vector center, Vector up)
{
	LookAtHelper(eye.i, eye.j, eye.k, center.i, center.j, center.k, up.i, up.j, up.k);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(view);

	return TRUE;
}
