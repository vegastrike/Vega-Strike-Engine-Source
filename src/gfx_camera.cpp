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

//const float PI=3.1415926536;
Camera::Camera() : myPhysics(0.1,0.075,&Coord,&P,&Q,&R)
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
}
void Camera::GetPQR (Vector &p1, Vector &q1, Vector &r1){p1.i = P.i;p1.j = P.j; p1.k = P.k;q1.i = Q.i;q1.j = Q.j; q1.k = Q.k;r1.i = R.i;r1.j = R.j; r1.k = R.k;}
void Camera::UpdateGFX(bool updateFrustum)
{
//	static float rotfactor = 0;
	//glMatrixMode(GL_PROJECTION);

	if(changed)
	{
		myPhysics.Update();
		GFXLoadIdentity(PROJECTION);
		GFXLoadIdentity(VIEW);
		//glLoadIdentity();
		GFXPerspective (78,1.33F,1.00F,100.00F); //set perspective to 78 degree FOV
		GFXLookAt (Coord, Coord+R, Q);
		if (updateFrustum) GFXCalculateFrustum();
	}
	//glMultMatrixf(view);
}

void Camera::UpdateGLCenter()
{
//	static float rotfactor = 0;
	//glMatrixMode(GL_PROJECTION);

	if(changed)
	{
		GFXLoadIdentity(PROJECTION);
		GFXLoadIdentity(VIEW);
		GFXPerspective (78,1.33F,1.00F,100.00F); //set perspective to 78 degree FOV
		GFXLookAt (Vector(0,0,0), R, Q);
		//changed = FALSE;
	}
	//glMultMatrixf(view);
}

void Camera::SetPosition(Vector &origin)
{
	Coord = origin;
	changed= TRUE;
//	SetPosition();
}

/*void SetPosition()
{
	//Translate(translation, -Coord.i,-Coord.j,-Coord.k);
	gluLookAt (Coord.i,Coord.j,Coord.k, Coord.i+R.i, Coord.j+R.j, Coord.k+R.k, Q.i,Q.j,Q.k);
	changed = TRUE;
}*/
void Camera::GetPosition(Vector &vect)
{
	vect = Coord;
}
Vector &Camera::GetPosition()
{
	return Coord;
}

void Camera::SetOrientation(Vector &p, Vector &q, Vector &r)
{
	P = p;
	Q = q;
	R = r;

//	SetOrientation();
}

//void SetOrientation()
//{
	//VectorToMatrix(orientation, P,Q,R);
	//changed = TRUE;
//}
void Camera::Yaw(float rad)
{
	::Yaw(rad,P,Q,R);
	changed= TRUE;
	//SetOrientation();
}
void Camera::Pitch(float rad)
{
	::Pitch(rad,P,Q,R);
	changed= TRUE;
	//SetOrientation();
}
void Camera::Roll(float rad)
{
	::Roll(rad,P,Q,R);
	changed= TRUE;
	//SetOrientation();
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
