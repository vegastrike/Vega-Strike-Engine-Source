#include "gfx_transform_vector.h"

const float oocc = (float).0000000000000000111265005605; //   1/c^2
const float c = (float)299792458.0;
const float co10 = (float)29979245.8;

struct Quaternion {
	float s;
	Vector v;
	Quaternion(float s, Vector v) {this->s = s; this->v = v;};

	Quaternion Conjugate() {return Quaternion(s, Vector(-v.i, -v.j, -v.k));};
	float Magnitude() {return sqrtf(s*s+v.i*v.i+v.j*v.j+v.k*v.k);};

	Quaternion operator* (const Quaternion &rval) { 
	  return Quaternion(s*rval.s - DotProduct(v, rval.v), 
			    s*rval.v + rval.s*v + v.Cross(rval.v));};
};

