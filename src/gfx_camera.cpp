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
#include "gfx_camera.h"

//Remove GL specific stuff here
#include "vegastrike.h"

  const float fov = 78;
  const float aspect = 1.33F;
  const float znear = 1.00F;
  const float zfar = 100.00F;

//const float PI=3.1415926536;
Camera::Camera(ProjectionType proj) : myPhysics(0.1,0.075,&Coord,&P,&Q,&R), projectionType(proj)
{
	ResetVectors(P,Q,R);
	R = -R;
	Coord.i = 0;
	Coord.j = 0;
	Coord.k = -1;

	changed = TRUE;
	//SetPosition();
	//SetOrientation();
	Identity(view);
	Yaw(PI);
	x = y = 0;
	xsize = ysize = 1.0;
	zoom = 1.0;
}

void Camera::GetPQR (Vector &p1, Vector &q1, Vector &r1){p1.i = P.i;p1.j = P.j; p1.k = P.k;q1.i = Q.i;q1.j = Q.j; q1.k = Q.k;r1.i = R.i;r1.j = R.j; r1.k = R.k;}

void Camera::UpdateGFX(bool updateFrustum)
{
  float xmin, xmax, ymin, ymax;
	if(changed)
	{
		myPhysics.Update();
		GFXLoadIdentity(PROJECTION);
		GFXLoadIdentity(VIEW);
		switch(projectionType) {
		case Camera::PERSPECTIVE:
		  GFXPerspective (zoom*fov,aspect,znear,zfar); //set perspective to 78 degree FOV
		  break;
		case Camera::PARALLEL:
		  ymax = znear * tanf( zoom*fov * PI / ((float)360.0) ); //78.0 --> 4.7046
		  
		  ymin = -ymax; //-4.7046
		  
		  xmin = ymin * aspect;//-6.2571
		  xmax = ymax * aspect;//6.2571
		  
		  //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
		  GFXParallel(aspect*-zoom,aspect*zoom,-zoom,zoom,-zfar,zfar);
		  break;
		}
		GFXLookAt (Coord-R, Coord, Q);
		if (updateFrustum) GFXCalculateFrustum();
		GFXGetMatrix(VIEW,view);
		GFXSubwindow(x,y,xsize,ysize);
	}
}

void Camera::UpdateGLCenter()
{
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
		  GFXPerspective (zoom*fov,aspect,znear,zfar); //set perspective to 78 degree FOV
		  break;
		case Camera::PARALLEL:
		  ymax = znear * tanf( zoom*fov * PI / ((float)360.0) ); //78.0 --> 4.7046
		  
		  ymin = -ymax; //-4.7046
		  
		  xmin = ymin * aspect;//-6.2571
		  xmax = ymax * aspect;//6.2571
		  
		  //GFXParallel(xmin,xmax,ymin,ymax,-znear,zfar);
		  GFXParallel(aspect*-zoom,aspect*zoom,-zoom,zoom,-znear,zfar);
		  break;
		}

		GFXSubwindow(x,y,xsize,ysize);
		GFXLookAt (-R, Vector(0,0,0), Q);
		//changed = FALSE;
	}
	//glMultMatrixf(view);
}

void Camera::SetPosition(const Vector &origin)
{
	Coord = origin;
	changed= TRUE;
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
void Camera::GetView (Matrix x) {
  CopyMatrix (x,view);
}

void Camera::LookAt(const Vector &loc, const Vector &up) {
  P = (loc - Coord).Normalize();
  Q = up;
  Q.Normalize();
  CrossProduct(P,Q,R);
  changed = TRUE;
}

void Camera::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{
	P = p;
	Q = q;
	R = r;
	changed = TRUE;
}

void Camera::SetSubwindow(float x, float y, float xsize, float ysize) {
  this->x = x;
  this->y = y;
  this->xsize = xsize;
  this->ysize = ysize;
  changed = TRUE;
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
	changed= TRUE;
}
void Camera::Pitch(float rad)
{
	::Pitch(rad,P,Q,R);
	changed= TRUE;
}
void Camera::Roll(float rad)
{
	::Roll(rad,P,Q,R);
	changed= TRUE;
}
void Camera::XSlide(float factor)
{
	Coord += P * factor;
	changed = TRUE;
}
void Camera::YSlide(float factor)
{
	Coord += Q * factor;
	changed = TRUE;
}
void Camera::ZSlide(float factor)
{
	Coord += R * factor;
	changed = TRUE;
}
