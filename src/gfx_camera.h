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
	void UpdateGFX();
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
