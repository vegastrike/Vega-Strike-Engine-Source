#ifndef _QUATERNION_H_
#define _QUATERNION_H_
#include "gfx_transform_vector.h"
#include "gfx_transform_matrix.h"

struct Quaternion {
  float s;
  Vector v;
  inline Quaternion() {
    s = 0;
    v = Vector(0,0,0);
  }
  inline Quaternion(float s, Vector v) {this->s = s; this->v = v;};

  inline Quaternion Conjugate() const {return Quaternion(s, Vector(-v.i, -v.j, -v.k));};

  inline float Magnitude() const {return sqrtf(s*s+v.i*v.i+v.j*v.j+v.k*v.k);};
  inline Quaternion operator* (const Quaternion &rval) const { 
	  return Quaternion(s*rval.s - DotProduct(v, rval.v), 
			    s*rval.v + rval.s*v + v.Cross(rval.v));};
  inline Quaternion operator*=(const Quaternion &rval) {
    return *this = *this * rval;
  }
  Quaternion Normalize() { v = v * (1.0/Magnitude()); s /= Magnitude(); return *this;}
  void to_matrix(Matrix mat) const;

  static Quaternion from_vectors(const  Vector &v1, const Vector &v2, const Vector &v3);
  static Quaternion from_axis_angle(const Vector &axis, float angle);
};

inline Quaternion operator-(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s - b.s, a.v-b.v);
}
inline Quaternion operator+(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s + b.s, a.v+b.v);
}

inline Quaternion operator*(const Quaternion &a, const double &b) {
  return Quaternion(a.s * b, a.v * b);
}

const Quaternion identity_quaternion(1, Vector(0,0,0));

struct Transformation {
  Quaternion orientation;
  Vector position;
  inline Transformation() { orientation = identity_quaternion; position = Vector(0,0,0); }
  inline Transformation(const Quaternion &orient, const Vector &pos) : orientation(orient), position(pos) { }
 
  inline void to_matrix(Matrix m) const {
    orientation.to_matrix(m);
    m[12] = position.i;
    m[13] = position.j;
    m[14] = position.k;
  }
  inline void Compose(const Transformation &b, const Matrix m) {
    orientation*=b.orientation;
    position = position.Transform(m);
  }
  inline void Invert() {
    orientation = orientation.Conjugate();
    position = -position;
  }
};

const Transformation identity_transformation(identity_quaternion,Vector(0,0,0));

#endif

