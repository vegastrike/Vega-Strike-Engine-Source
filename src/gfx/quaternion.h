#ifndef _QUATERNION_H_
#define _QUATERNION_H_
#include "vec.h"
#include "matrix.h"

struct Quaternion {
  float s;
  Vector v;
  inline Quaternion(): s(0),v(0,0,0){}
  inline Quaternion(float s, Vector v) {this->s = s; this->v = v;};
  //  inline Quaternion(float s, Vector v) {this->s = s; this->v = v;};
  inline Quaternion Conjugate() const {return Quaternion(s, Vector(-v.i, -v.j, -v.k));};

  inline void netswap()
  {
  	s = VSSwapHostFloatToLittle( s);
	v.netswap();
  }

  inline float Magnitude() const {return sqrtf(s*s+v.i*v.i+v.j*v.j+v.k*v.k);};
  inline Quaternion operator* (const Quaternion &rval) const { 
	  return Quaternion(s*rval.s - DotProduct(v, rval.v), 
			    s*rval.v + rval.s*v + v.Cross(rval.v));};
  inline Quaternion operator*=(const Quaternion &rval) {
    return *this = *this * rval;
  }
  inline bool operator == (const Quaternion &rval) const {
    return s==rval.s&&v.i==rval.v.i&&v.j==rval.v.j&&v.k==rval.v.k;
  }
  Quaternion Normalize() { float mag = Magnitude();v = v * (((float)1.0)/mag); s /= mag; return *this;}


  static Quaternion from_vectors(const  Vector &v1, const Vector &v2, const Vector &v3);
  static Quaternion from_axis_angle(const Vector &axis, float angle);
  void to_matrix(Matrix &mat) const {
    const  float GFXEPSILON =        ((float)10e-6);
    float W = v.i*v.i+v.j*v.j+v.k*v.k+s*s; //norm
    W = (W<0+GFXEPSILON&&W>0-GFXEPSILON)?0:2.0/W;

    float xw = v.i*W;    float yw = v.j*W;    float zw = v.k*W;
  
    float sx = s*xw;    float sy = s*yw;    float sz = s*zw;
 
    float xx = v.i*xw;    float xy = v.i*yw;    float xz = v.i*zw;
    
    float yy= v.j*yw;    float yz= v.j*zw;    float zz= v.k*zw;
  
#define M(B,A) mat.r[B*3+A]
  M(0,0)  = 1 - ( yy + zz );
  M(1,0)  =     ( xy + sz );
  M(2,0)  =     ( xz - sy );

  M(0,1)  =     ( xy - sz );
  M(1,1)  = 1 - ( xx + zz );
  M(2,1)  =     ( yz + sx );

  M(0,2)  =     ( xz + sy );
  M(1,2)  =     ( yz - sx );
  M(2,2) = 1 - ( xx + yy );
  //  M(3,0)  = M(3,1) = M(3,2) = M(0,3) = M(1,3) = M(2,3) = 0;
  //  M(3,3) = 1;

#undef M


  }


};

inline Quaternion operator-(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s - b.s, a.v-b.v);
}
inline Quaternion operator+(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s + b.s, a.v+b.v);
}

inline Quaternion operator*(const Quaternion &a, const float &b) {
  return Quaternion(a.s * b, a.v * b);
}

const Quaternion identity_quaternion(1, Vector(0,0,0));

struct Transformation {
  Quaternion orientation;
  QVector position;
  inline Transformation():orientation(identity_quaternion), position(0,0,0) { }
  inline Transformation(const Quaternion &orient, const QVector &pos) : orientation(orient), position(pos) { }
  //  inline Transformation(const Quaternion &orient, const QVector &pos) : orientation(orient), position(pos) { }

  inline void netswap()
  {
  	orientation.netswap();
	position.netswap();
  }
 
  inline void to_matrix(Matrix &m) const {
    orientation.to_matrix(m);
    m.p = position;
  }
  inline void Compose(const Transformation &b, const Matrix &m) {
    orientation*=b.orientation;
    position = Transform(m,position);
  }
  inline void InvertOrientationRevPos() {
    orientation = orientation.Conjugate();
    position = -position;
  }
  inline void InvertAndToMatrix (Matrix &m) {
      InvertOrientationRevPos();
      to_matrix(m);
      m.p= TransformNormal (m,position);
  }
  static Transformation from_matrix (Matrix &m) {
    Vector p,q,r;
    QVector c;
    MatrixToVectors (m,p,q,r,c);
    return Transformation(Quaternion::from_vectors (p,q,r),c);
  }
};

const Transformation identity_transformation(identity_quaternion,QVector(0,0,0));




#endif

