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
#include "gfx/vec.h"
#include <stdio.h>
//typedef float GLdouble;
#include <math.h>
#include <string.h>
#include <assert.h>
#include "vegastrike.h"
#include "vs_globals.h"
#ifdef WIN32
#include <windows.h>
#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif
#endif
#include "gl_matrix.h"

#include "vs_globals.h"


//#include <GL/glu.h>


float Magnitude(Vector v)
{
	return sqrtf(v.i*v.i + v.j*v.j + v.k*v.k);
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


static float centerx,centery,centerz;

using namespace GFXMatrices;  //causes problems with g_game
void evaluateViews (const Vector & myvec) {
  centerx = myvec.i;
  centery = myvec.j;
  centerz = myvec.k;
  //Identity(transview);
  Identity (rotview);
#define M(row,col) rotview[col*4+row]
  rotview[0]=view[0];
  rotview[1]=view[1];
  rotview[2]=view[2];
  //  transview[3]=view[3];
  rotview[4]=view[4];
  rotview[5]=view[5];
  rotview[6]=view[6];
  //  transview[7]=view[7];
  rotview[8]=view[8];
  rotview[9]=view[9];
  rotview[10]=view[10];
  //    transview[11]=view[11];
  //      fprintf (stderr,"trans %f,%f,%f",transview[3],transview[7],transview[11]);
#undef M

}
Vector eye, center, up;
void getInverseProjection (float *& inv) {
  inv = invprojection;
}

float GFXGetXInvPerspective () {
  return /*invprojection[11]*  */invprojection[0];//invprojection[15];//should be??  c/d == invproj[15]
}

float GFXGetYInvPerspective() {
  return /*invprojection[11]*  */invprojection[5];//invprojection[15];//should be??  c/d == invproj[15]
}
inline void ViewToModel (bool tofrom) {
  static Vector t;
  if (tofrom) {
    t.Set (model[12],model[13],model[14]);
    //    fprintf (stderr,"oldpos <%f %f %f> newpos <%f %f %f>",_Universe->AccessCamera()->GetPosition().i,_Universe->AccessCamera()->GetPosition().j,_Universe->AccessCamera()->GetPosition().k,centerx,centery,centerz);
    model[12]-=centerx;//_Universe->AccessCamera()->GetPosition().i;
      model[13]-=centery;//_Universe->AccessCamera()->GetPosition().j;
      model[14]-=centerz;//_Universe->AccessCamera()->GetPosition().k;
  }else {
    model[12]=t.i;
    model[13]=t.j;
    model[14]=t.k;
  }
}
void GFXTranslateView (const Vector &a) {
	  view[12]+=a.i*view[0]+a.j*view[4]+a.k*view[8];
	  view[13]+=a.i*view[1]+a.j*view[5]+a.k*view[9];
	  view[14]+=a.i*view[2]+a.j*view[6]+a.k*view[10];
	  evaluateViews (Vector (centerx, centery, centerz)+a);
	  glMatrixMode(GL_MODELVIEW);
	  //	  glPopMatrix();
	  //	  glLoadIdentity();
	  //	  glTranslatef(-view[12],-view[13],-view[14]);
	  //	  glPushMatrix();
	  ViewToModel (true);
	  glLoadMatrixf(model);
	  ViewToModel (false);
}

void /*GFXDRVAPI*/ GFXTranslate(const MATRIXMODE mode, const Vector & a) 
{
	switch(mode)
	{
	case MODEL:
	  model[12]+=a.i*model[0]+a.j*model[4]+a.k*model[8];
	  model[13]+=a.i*model[1]+a.j*model[5]+a.k*model[9];
	  model[14]+=a.i*model[2]+a.j*model[6]+a.k*model[10];
	  glMatrixMode(GL_MODELVIEW);
	  glTranslatef(a.i,a.j,a.k);
	  break;
	case PROJECTION:
	  projection[12]+=a.i*projection[0]+a.j*projection[4]+a.k*projection[8];
	  projection[13]+=a.i*projection[1]+a.j*projection[5]+a.k*projection[9];
	  projection[14]+=a.i*projection[2]+a.j*projection[6]+a.k*projection[10];
	  {
	    Matrix t;
	    MultMatrix (t,projection,rotview);
	    glMatrixMode(GL_PROJECTION);
	    glLoadMatrixf(t);
	  }
	  break;
	}
}

#if 0
void GFXMultMatrixView (const Matrix) {
  const int MULTMATRIXVIEWNOTIMPLEMENTED=0;
  assert (MULTMATRIXVIEWNOTIMPLEMENTED);
  MultMatrix(t, view, matrix);
  CopyMatrix(view, t);
  evaluateViews();
  MultMatrix(t, projection,rotview);
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(t);
  glMatrixMode(GL_MODELVIEW);
  //glPopMatrix();
  //glLoadIdentity();
  //glTranslatef(-centerx,-centery,-centerz);
  //glPushMatrix();
  ViewToModel (true);
  glLoadMatrixf(model);
  ViewToModel (false);
}
#endif
void /*GFXDRVAPI*/ GFXMultMatrix(const MATRIXMODE mode, const Matrix matrix)
{
	Matrix t;
	switch(mode)
	{
	case MODEL:
	  MultMatrix(t, model, matrix);
	  CopyMatrix(model, t);
	  glMatrixMode(GL_MODELVIEW);
	  //glPopMatrix();
	  //glPushMatrix();
	  ViewToModel (true);
	  glLoadMatrixf(model);
	  ViewToModel (false);
	  break;
	case PROJECTION:
		MultMatrix(t, projection, matrix);
		CopyMatrix(projection, t);
		MultMatrix (t,projection,rotview);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(t);
		break;
	}
}
float *mm = model;
float *vv = view;
void GFXLoadMatrixView (const Matrix matrix, const Vector &camloc) {

		CopyMatrix(view, matrix);
		Matrix t;
		evaluateViews(camloc);
		glMatrixMode(GL_MODELVIEW);
		ViewToModel (true);
		glLoadMatrixf(model);
		ViewToModel (false);
		glMatrixMode(GL_PROJECTION);
		MultMatrix (t,projection,rotview);
		glLoadMatrixf(t);
		
}
void /*GFXDRVAPI*/ GFXLoadMatrix(const MATRIXMODE mode, const Matrix matrix)
{
	switch(mode)
	{
	case MODEL:
	  CopyMatrix(model, matrix);
		glMatrixMode(GL_MODELVIEW);
		//		glPopMatrix();
		//		glPushMatrix();
		ViewToModel (true);
		glLoadMatrixf(model);
		ViewToModel (false);
		break;
	case PROJECTION:
		CopyMatrix(projection, matrix);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projection);
		glMultMatrixf(rotview);
		break;
	}
}

void GFXViewPort (int minx, int miny, int maxx, int maxy) {
  glViewport (minx,miny,maxx,maxy);
}

void GFXCenterCamera (bool Enter) {
  static Vector tmp;
  if (Enter) {
    tmp.Set (centerx,centery,centerz);
    centerx=0;centery=0;centerz=0;
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
  }else {
    centerx = tmp.i;
    centery = tmp.j;
    centerz = tmp.k;
    GFXLoadIdentity (MODEL);
  }
}
void GFXRestoreHudMode () {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();  
}
void GFXHudMode (const bool Enter) {
  if (Enter) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
  }else {
    glMatrixMode (GL_PROJECTION);
    glPopMatrix();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix();
  }
}
void GFXLoadIdentityView () {
		Identity(view);
		Identity (rotview);
		evaluateViews (Vector (0,0,0));
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixf(model);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(projection);

}
void /*GFXDRVAPI*/ GFXLoadIdentity(const MATRIXMODE mode)
{
	switch(mode)
	{
	case MODEL:
		Identity(model);
		glMatrixMode(GL_MODELVIEW);
		//		glLoadMatrixf(transview);
		glLoadIdentity();
		glTranslatef (-centerx,-centery,-centerz);
		break;
	case PROJECTION:
		Identity(projection);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(rotview);
		break;
	}

}

void GFXGetMatrixView (Matrix matrix) {
  CopyMatrix (matrix,view);
}
void /*GFXDRVAPI*/ GFXGetMatrix(const MATRIXMODE mode, Matrix matrix)
{

	switch(mode)
	{
	case MODEL:
	  CopyMatrix (matrix,model);
	  break;
	case PROJECTION:
	  CopyMatrix(matrix, projection);
	  break;
	}
}

static void gl_Frustum (float left,float right, float bottom, float top, float nearval,float farval){
  GFXGetFrustumVars(false,&left,&right,&bottom,&top,&nearval,&farval);
  GFXFrustum (projection,invprojection,left,right,bottom,top,nearval,farval);

  
}


void GFXFrustum(float * m,float *i, 
                 float left, float right,
	 	 float bottom, float top,
		 float nearval, float farval )
{

   GLfloat x, y, a, b, c, d;
   x = (((float)2.0)*nearval) / (right-left);
   y = (((float)2.0)*nearval) / (top-bottom);
   a = (right+left) / (right-left);
   b = (top+bottom) / (top-bottom);
   c = -(farval+nearval) / ( farval-nearval);
   d = -(((float)2.0)*farval*nearval) / (farval-nearval);  /* error? */

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = a;      M(0,3) = 0.0F;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0F;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = c;      M(2,3) = d;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = -1.0F;  M(3,3) = 0.0F;
#undef M
#define M(row,col) i[col*4+row]
   M(0,0) = 1./x;  M(0,1) = 0.0F;  M(0,2) = 0.0F;   M(0,3) = a/x;
   M(1,0) = 0.0F;  M(1,1) = 1./y;  M(1,2) = 0.0F;   M(1,3) = b/y;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = 0.0F;   M(2,3) =-1.0F;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = 1.F/d;  M(3,3) = (float)c/d;
#undef M
}
void /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar, float cockpit_offset)
{

  //  gluPerspective (fov,aspect,znear,zfar);


   float xmin, xmax, ymin, ymax;



   ymax = znear * tanf( fov * M_PI / ((float)360.0) ); //78.0 --> 4.7046

   ymin = -ymax; //-4.7046

   xmin = (ymin-cockpit_offset/2) * aspect;//-6.2571
   xmax = (ymax+cockpit_offset/2) * aspect;//6.2571
   ymin-=cockpit_offset;
   gl_Frustum( xmin, xmax, ymin, ymax, znear, zfar );
   glMatrixMode(GL_PROJECTION);
   glLoadMatrixf(projection);
}

void /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float nearval, float farval)
{
  float *m = projection, x,y,z,tx,ty,tz;
   x = 2.0 / (right-left);
   y = 2.0 / (top-bottom);
   z = -2.0 / (farval-nearval);
   tx = -(right+left) / (right-left);
   ty = -(top+bottom) / (top-bottom);
   tz = -(farval+nearval) / (farval-nearval);

#define M(row,col)  m[col*4+row]
   M(0,0) = x;     M(0,1) = 0.0F;  M(0,2) = 0.0F;  M(0,3) = tx;
   M(1,0) = 0.0F;  M(1,1) = y;     M(1,2) = 0.0F;  M(1,3) = ty;
   M(2,0) = 0.0F;  M(2,1) = 0.0F;  M(2,2) = z;     M(2,3) = tz;
   M(3,0) = 0.0F;  M(3,1) = 0.0F;  M(3,2) = 0.0F;  M(3,3) = 1.0F;
#undef M
   GFXLoadMatrix(PROJECTION, projection);

  GFXGetFrustumVars(false,&left,&right,&bottom,&top,&nearval,&farval);

}


static void LookAtHelper( float eyex, float eyey, float eyez,
                         float centerx, float centery, float centerz,
                         float upx, float upy, float upz)
{
   float m[16];
   float x[3], y[3], z[3];
   float mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex;
   z[1] = eyey;
   z[2] = eyez;
   mag = sqrtf( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
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

   mag = sqrtf( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrtf( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
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

#if defined(WIN32)
   ZeroMemory(tm, sizeof(tm));

#elif defined(IRIX)	// ANSI standard function, available on Windows also ...
	(void) memset(tm, 0, sizeof tm);
#else
   bzero (tm, sizeof (tm));
#endif

#undef M

#define M(row,col)  tm[col*4+row]
   M(0,0) = 1.0;
   M(0,3) = -centerx-eyex;
   M(1,1) = 1.0;
   M(1,3) = -centery-eyey;
   M(2,2) = 1.0;
   M(2,3) = -centerz-eyez;
   M(3,3) = 1.0;
#undef M

   MultMatrix(view, m, tm);

/***
    float dis = sqrtf(upx*upx+upy*upy);
   Identity (tm);
   if (eyez-centerz > 0) {
     upx = -upx; 
   }
#define M(row,col)  tm[col*4+row]
   M(0,0) = upy/dis;
   M(0,1) = -upx/dis;
   M(1,1) = upy/dis;
   M(1,0) = upx/dis;
   M(2,2) = 1.0;
   M(3,3) = 1.0;
#undef M
***/    //old hack to twiddle the texture in the xy plane
	
#ifdef NV_CUBE_MAP
   //FIXME--ADD CAMERA MATRICES
   //the texture matrix must be used to rotate the texgen-computed
   //reflection or normal vector texture coordinates to match the orinetation 
   //of the cube map.  Teh rotation can be computed based on two vectors 
   //1) the direction vector from the cube map center to the eye position
   //and 2 the cube map orientation in world coordinates.
   //the axis is the cross product of these two vectors...teh angle is arcsin
   //of the dot of these two vectors
	GFXActiveTexture (1);
	glMatrixMode (GL_TEXTURE);	
	glLoadIdentity();
#error

   //   Vector (centerx,centery,centerz).Cross (Vector (1,0,0));  DID NOT TRANSFORM THE ORIENTATION VECTOR TO REVERSE CAMERASPACE
   Vector axis (centerx,centery,centerz);
   Vector cubemapincamspace(eyex+centerx,eyey+centery,eyez+centerz);
   cubemapincamspace.Normalize();
   axis.Normalize();
   //   float theta = arcsinf (Vector (centerx,centery,centerz).Normalize().Dot (Vector (1,0,0)));  DID NOT TRANSFORM THE ORIENTATION VECTOR TO REVERSE CAMERASPACE
   float theta =asinf (axis.Dot(cubemapincamspace));
   axis = axis.Cross (axis.Cross(cubemapincamspace));
   glRotatef (theta,axis.i,axis.j,axis.k);
   //ok do matrix math to rotate by theta on axis  those ..
   GFXActiveTexture (0);

#else
   /*	glTranslatef(.5f,.5f,.4994f);
	glMultMatrixf(tm);
	glTranslatef(-.5f,-.5f,-.4994f);
   */
#endif
   
}

void /*GFXDRVAPI*/ GFXLookAt(Vector eye, Vector center, Vector up )
{
	LookAtHelper(eye.i, eye.j, eye.k, center.i, center.j, center.k, up.i, up.j, up.k);


	//	Identity(transview);
	//	transview[3]=center.i;
	//	transview[7]=center.j;
	//	transview[11]=center.k;
	GFXLoadMatrixView(view, center);
}

