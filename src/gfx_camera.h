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
#include "gfxlib.h"
#include "physics.h"

#ifndef _CAMERA_H_
#define _CAMERA_H_
class Camera{
	Vector Coord;
	Matrix view;
	BOOL changed;
	
	float x, y, xsize, ysize;
	float zoom;

public:
	enum ProjectionType {
	  PARALLEL,
	  PERSPECTIVE
	};
	
private:
	ProjectionType projectionType;

public:
	Vector P,Q,R;
	PhysicsSystem myPhysics;

	Camera(ProjectionType proj = PERSPECTIVE);
        void GetView (Matrix);
	void GetPQR (Vector &p1, Vector &q1, Vector &r1);
	void UpdateGFX(bool updateFrustum=true);
	void UpdateGLCenter();

	void SetPosition(const Vector &origin);
	void GetPosition(Vector &vect);

	void GetOrientation(Vector &p, Vector &q, Vector &r);
	const Vector &GetPosition();

	void LookAt(const Vector &loc, const Vector &up);
	void SetOrientation(const Vector &p, const Vector &q, const Vector &r);
	void SetSubwindow(float x, float y, float xsize, float ysize);
	void SetProjectionType(ProjectionType t);
	void SetZoom(float z);
	float GetZoom();
	void Yaw(float rad);
	void Pitch(float rad);
	void Roll(float rad);
	void XSlide(float factor);
	void YSlide(float factor);
	void ZSlide(float factor);
};

#endif
