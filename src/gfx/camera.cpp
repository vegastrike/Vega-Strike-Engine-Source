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
#include "camera.h"
#include "matrix.h"

//Remove GL specific stuff here

#include "vs_globals.h"
#include "audiolib.h"
#include "lin_time.h"

#include <assert.h>	// needed for assert() calls
#include "planetary_transform.h"
//const float PI=3.1415926536;
Camera::Camera(ProjectionType proj) : projectionType(proj), myPhysics(0.1,0.075,&Coord,&P,&Q,&R)
{
	ResetVectors(P,Q,R);
	R = -R;
	Coord.i = 0;
	Coord.j = 0;
	Coord.k = -1;

	changed = GFXTRUE;
	//SetPosition();
	//SetOrientation();
	Yaw(PI);
	x = y = 0;
	xsize = ysize = 1.0;
	zoom = 1.0;
}

void Camera::GetPQR (Vector &p1, Vector &q1, Vector &r1){p1.i = P.i;p1.j = P.j; p1.k = P.k;q1.i = Q.i;q1.j = Q.j; q1.k = Q.k;r1.i = R.i;r1.j = R.j; r1.k = R.k;}



void Camera::UpdateGFX(GFXBOOL clip, GFXBOOL updateFrustum)
{
  const float ZFARCONST= 10000;
  float xmin, xmax, ymin, ymax;
	if(changed)
	{

		myPhysics.Update();
		GFXLoadIdentity(PROJECTION);
		//FIXMEGFXLoadIdentity(VIEW);
		switch(projectionType) {
		case Camera::PERSPECTIVE:
		  GFXPerspective (zoom*g_game.fov,g_game.aspect,g_game.znear,g_game.zfar*(clip?1:ZFARCONST),cockpit_offset); //set perspective to 78 degree FOV
		  break;
		case Camera::PARALLEL:
		  ymax = g_game.znear * tanf( zoom*g_game.fov * PI / ((float)360.0) ); 
		  
		  ymin = -ymax; //-4.7046
		  
		  xmin = ymin * g_game.aspect;//-6.2571
		  xmax = ymax * g_game.aspect;//6.2571
		  
		  //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
		  GFXParallel(g_game.aspect*-zoom,g_game.aspect*zoom,-zoom,zoom,-g_game.zfar*(clip?1:ZFARCONST),g_game.zfar*(clip?1:ZFARCONST));
		  break;
		}
		GFXLookAt (Coord-R, Coord, Q);
		if (updateFrustum) GFXCalculateFrustum();

#ifdef PERFRAMESOUND		
		Vector lastpos(view[12],view[13],view[14]);
		AUDListener (Coord, (Coord-lastpos)/GetElapsedTime());//this pos-last pos / elapsed time
#endif
		//		GFXGetMatrix(VIEW,view);
		GFXSubwindow(x,y,xsize,ysize);
#ifdef PERFRAMESOUND
		AUDListenerOrientation (P,Q,R);
#endif
	}
}
Vector Camera::GetVelocity() {
	return (Coord-lastpos)/SIMULATION_ATOM;
}
void Camera::UpdateCameraSounds() {
#ifndef PERFRAMESOUND
  AUDListener (Coord,GetVelocity());
  AUDListenerOrientation (P,Q,R);
#endif
}
#ifdef OLDUPDATEPLANEGFX
    /**
    Vector c (planet->InvTransform (Coord));
    Vector tt (planet->Transform (c));
    fprintf (stderr,"t <%f,%f,%f>\n",tt.i,tt.j,tt.k);
    fprintf (stderr,"Coord <%f,%f,%f>\n",Coord.i,Coord.j,Coord.k);
    Vector p (planet->InvTransform (Coord+P)-c);
    Vector q (planet->InvTransform (Coord+Q)-c);
    Vector r (planet->InvTransform (Coord+R)-c);
    r.Normalize();
    
    //    p= q.Cross (r);
//	  q = r.Cross (p);
    p.Normalize();
    q.Normalize();
    q = q-q.Dot (r)*r;
    q.Normalize();
    */

#endif

void Camera::GetView (Matrix vw) {
  GFXGetMatrix (VIEW,vw);
}
void Camera::UpdatePlanetGFX () {

  if (planet) {
    Matrix t;
    Matrix inv;
    planet->InvTransformBasis(planetview,P,Q,R,Coord);

    InvertMatrix (inv,planetview);
    VectorAndPositionToMatrix (t,P,Q,R,Coord);
    MultMatrix (planetview,t,inv);
  } else {
    Identity (planetview);
  }
}

void Camera::UpdateGLCenter()
{
#define ITISDEPRECATED 0
  assert (ITISDEPRECATED);
#undef ITISDEPRECATED
//	static float rotfactor = 0;
	//glMatrixMode(GL_PROJECTION);
  float xmin, xmax, ymin, ymax;

	if(changed)
	{
		GFXLoadIdentity(PROJECTION);
		GFXLoadIdentity(VIEW);
		//updating the center should always use a perspective
		switch(Camera::PERSPECTIVE) {
		case Camera::PERSPECTIVE:
		  GFXPerspective (zoom*g_game.fov,g_game.aspect,g_game.znear,g_game.zfar,cockpit_offset); //set perspective to 78 degree FOV
		  break;
		case Camera::PARALLEL:
		  ymax = g_game.znear * tanf( zoom*g_game.fov * PI / ((float)360.0) ); //78.0 --> 4.7046
		  
		  ymin = -ymax; //-4.7046
		  
		  xmin = ymin * g_game.aspect;//-6.2571
		  xmax = ymax * g_game.aspect;//6.2571
		  
		  //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
		  GFXParallel(g_game.aspect*-zoom,g_game.aspect*zoom,-zoom,zoom,-g_game.znear,g_game.zfar);
		  break;
		}

		GFXSubwindow(x,y,xsize,ysize);
		GFXLookAt (-R, Vector(0,0,0), Q);
		//changed = GFXFALSE;
	}
	//glMultMatrixf(view);
}

void Camera::SetPosition(const Vector &origin)
{
	Coord = origin;
	changed= GFXTRUE;
}

void Camera::GetPosition(Vector &vect)
{
	vect = Coord;
}
const Vector &Camera::GetPosition()
{
	return Coord;
}

void Camera::GetOrientation(Vector &p, Vector &q, Vector &r) {
  p = P;
  q = Q;
  r = R;
}

/** GetView (Matrix x)
 *  returns the view matrix (inverse matrix based on camera pqr)
 */
float *Camera::GetPlanetGFX () {
  return &planetview[0];
  //  CopyMatrix (x,view);
}

void Camera::LookAt(const Vector &loc, const Vector &up) {
  P = (loc - Coord).Normalize();
  Q = up;
  Q.Normalize();
  CrossProduct(P,Q,R);
  changed = GFXTRUE;
}

void Camera::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{
	P = p;
	Q = q;
	R = r;
	changed = GFXTRUE;
}

void Camera::SetSubwindow(float x, float y, float xsize, float ysize) {
  this->x = x;
  this->y = y;
  this->xsize = xsize;
  this->ysize = ysize;
  changed = GFXTRUE;
}

void Camera::SetProjectionType(ProjectionType t) {
  projectionType = t;
}

void Camera::SetZoom(float z) {
  zoom = z;
}

float Camera::GetZoom() {
  return zoom;
}

void Camera::Yaw(float rad)
{
	::Yaw(rad,P,Q,R);
	changed= GFXTRUE;
}
void Camera::Pitch(float rad)
{
	::Pitch(rad,P,Q,R);
	changed= GFXTRUE;
}
void Camera::Roll(float rad)
{
	::Roll(rad,P,Q,R);
	changed= GFXTRUE;
}
void Camera::XSlide(float factor)
{
	Coord += P * factor;
	changed = GFXTRUE;
}
void Camera::YSlide(float factor)
{
	Coord += Q * factor;
	changed = GFXTRUE;
}
void Camera::ZSlide(float factor)
{
	Coord += R * factor;
	changed = GFXTRUE;
}
