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
#include "gfx_transform.h"
//#include <gl/glu.h>
//#include <gl/gl.h>
#include "gfxlib.h"
#include "physics.h"

#ifndef _CAMERA_H_
#define _CAMERA_H_
class Camera{
	Vector Coord;

	Matrix translation;
	Matrix orientation;
	Matrix view;
	BOOL changed;

public:
	Vector P,Q,R;
	PhysicsSystem myPhysics;

	Camera();

	void GetPQR (Vector &p1, Vector &q1, Vector &r1);
	void UpdateGFX(bool updateFrustum=true);
	void UpdateGLCenter();

	void SetPosition(Vector &origin);
	void GetPosition(Vector &vect);

	Vector &GetPosition();
	void SetOrientation(Vector &p, Vector &q, Vector &r);
	void Yaw(float rad);
	void Pitch(float rad);
	void Roll(float rad);
	void XSlide(float factor);
	void YSlide(float factor);
	void ZSlide(float factor);
};

#endif
