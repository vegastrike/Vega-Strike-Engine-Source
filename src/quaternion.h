#ifndef _QUATERNION_H_
#define _QUATERNION_H_
#include "gfx_transform_vector.h"
#include "gfx_transform_matrix.h"

struct Quaternion {
	float s;
	Vector v;
  Quaternion() {
    s = 0;
    v = Vector(0,0,0);
  }
	Quaternion(float s, Vector v) {this->s = s; this->v = v;};

	Quaternion Conjugate() const {return Quaternion(s, Vector(-v.i, -v.j, -v.k));};
	float Magnitude() const {return sqrtf(s*s+v.i*v.i+v.j*v.j+v.k*v.k);};
	Quaternion operator* (const Quaternion &rval) const { 
	  return Quaternion(s*rval.s - DotProduct(v, rval.v), 
			    s*rval.v + rval.s*v + v.Cross(rval.v));};
  Quaternion operator*=(const Quaternion &rval) {
    return *this = *this * rval;
  }
  Quaternion Normalize() { v = v * (1.0/Magnitude()); s /= Magnitude(); return *this;}
  void to_matrix(Matrix mat);

  static Quaternion from_vectors(Vector v1, Vector v2, Vector v3);
};

struct Transformation {
  Quaternion orientation;
  Vector position;
  Transformation() { }
  Transformation(const Quaternion &orient, const Vector &pos) : orientation(orient), position(pos) { }
 
  void to_matrix(Matrix m) {
    orientation.to_matrix(m);
    m[12] = position.i;
    m[13] = position.j;
    m[14] = position.k;
  }
  void Compose(const Transformation &b, const Matrix m) {
    orientation*=b.orientation;
    position = position.Transform(m) + b.position;
  }
  void Invert() {
    orientation = orientation.Conjugate();
    position = -position;
  }
};

#ifndef _QUATERNION_CPP_
extern const Quaternion identity_quaternion;
extern const Transformation identity_transformation;
#endif

#endif

