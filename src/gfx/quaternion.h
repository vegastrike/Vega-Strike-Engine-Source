#ifndef _QUATERNION_H_
#define _QUATERNION_H_
#include "vec.h"
#include "matrix.h"
//#define QUATERNION_DOUBLE


#ifdef QUATERNION_DOUBLE
typedef double QFLOAT;
#define QSQRT sqrt
#else
typedef float QFLOAT;
#define QSQRT sqrtf

#endif


class QVector {
 public:
  QFLOAT i;
  QFLOAT j;
  QFLOAT k;
  QVector () {}
  QVector (const Vector & a) {i=a.i;j=a.j;k=a.k;}
  QVector (QFLOAT a, QFLOAT b, QFLOAT c) {i=a;j=b;k=c;}
  QFLOAT Magnitude () {return QSQRT(i*i+j*j+k*k);}
  QVector Cross (const QVector &v) const {return    QVector ( this->j*v.k-this->k*v.j, 
							      this->k*v.i-this->i*v.k,
							      this->i*v.j-this->j*v.i);}
  QVector operator + (const QVector b) const {return QVector (i+b.i,j+b.j,k+b.k);}
  QVector operator * (const QFLOAT b) const {return QVector (i*b,j*b,k*b);}
  QVector operator - (const QVector b) const {return QVector (i-b.i,j-b.j,k-b.k);}
  QVector operator- () const {return QVector (-i, -j, -k);}
};

inline QFLOAT DotProduct(const QVector &a,const QVector &b)
{
	return (a.i*b.i+a.j*b.j+a.k*b.k);
}
inline QVector operator * (const QFLOAT a, const QVector &b) {
  return (QVector (a*b.i,a*b.j,a*b.k));
} 
struct Quaternion {
  QFLOAT s;
  QVector v;
  inline Quaternion(): s(0),v(0,0,0){}
  inline Quaternion(QFLOAT s, Vector v) {this->s = s; this->v = v;};
  inline Quaternion(QFLOAT s, QVector v) {this->s = s; this->v = v;};
  inline Quaternion Conjugate() const {return Quaternion(s, Vector(-v.i, -v.j, -v.k));};

  inline QFLOAT Magnitude() const {return QSQRT(s*s+v.i*v.i+v.j*v.j+v.k*v.k);};
  inline Quaternion operator* (const Quaternion &rval) const { 
	  return Quaternion(s*rval.s - DotProduct(v, rval.v), 
			    s*rval.v + rval.s*v + v.Cross(rval.v));};
  inline Quaternion operator*=(const Quaternion &rval) {
    return *this = *this * rval;
  }
  Quaternion Normalize() { v = v * (((QFLOAT)1.0)/Magnitude()); s /= Magnitude(); return *this;}
  void to_matrix(Matrix mat) const;

  static Quaternion from_vectors(const  Vector &v1, const Vector &v2, const Vector &v3);
  static Quaternion from_axis_angle(const Vector &axis, QFLOAT angle);
};

inline Quaternion operator-(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s - b.s, a.v-b.v);
}
inline Quaternion operator+(const Quaternion &a, const Quaternion &b) {
  return Quaternion(a.s + b.s, a.v+b.v);
}

inline Quaternion operator*(const Quaternion &a, const QFLOAT &b) {
  return Quaternion(a.s * b, a.v * b);
}

const Quaternion identity_quaternion(1, Vector(0,0,0));
inline QVector Transform (const Matrix t, const QVector & v) {
  //    Vector tLocation (t[12],t[13],t[14]);
//    Vector tP (t[0],t[1],t[2]);//the p vector of the plane being selected on
//    Vector tQ (t[4],t[5],t[6]);//the q vector of the plane being selected on
//    Vector tR (t[8],t[9],t[10]);//the q vector of the plane being selected on
  return QVector (t[12]+v.i*t[0]+v.j*t[4]+v.k*t[8],
		 t[13]+v.i*t[1]+v.j*t[5]+v.k*t[9],
		 t[14]+v.i*t[2]+v.j*t[6]+v.k*t[10]);
}
inline QVector InvTransformNormal (const Matrix t, const QVector & v) {

#define M(A,B) t[B*4+A]
  return QVector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}
inline QVector InvTransform (const Matrix t, const QVector & v) {
  return InvTransformNormal (t,  QVector (v.i-t[12], v.j-t[13], v.k-t[14]));
}


inline QVector TransformNormal (const Matrix t, const QVector & v) {
  return QVector (v.i*t[0]+v.j*t[4]+v.k*t[8],
		 v.i*t[1]+v.j*t[5]+v.k*t[9],
		 v.i*t[2]+v.j*t[6]+v.k*t[10]);
}

struct Transformation {
  Quaternion orientation;
  QVector position;
  inline Transformation():orientation(identity_quaternion), position(0,0,0) { }
  inline Transformation(const Quaternion &orient, const Vector &pos) : orientation(orient), position(pos) { }
 
  inline void to_matrix(Matrix m) const {
    orientation.to_matrix(m);
    m[12] = position.i;//FIXME???
    m[13] = position.j;
    m[14] = position.k;
  }
  inline void Compose(const Transformation &b, const Matrix m) {
    orientation*=b.orientation;
    position = Transform(m,position);
  }
  inline void InvertOrientationRevPos() {
    orientation = orientation.Conjugate();
    position = QVector (-position.i,-position.j,-position.k);
  }
  inline void InvertAndToMatrix (Matrix m) {
      InvertOrientationRevPos();
      to_matrix(m);
      position = TransformNormal (m,position);
      m[12]=position.i;
      m[13]=position.j;
      m[14]=position.k;
  }
  static Transformation from_matrix (Matrix m) {
    Vector p,q,r,c;
    MatrixToVectors (m,p,q,r,c);
    return Transformation(Quaternion::from_vectors (p,q,r),c);
  }
};

const Transformation identity_transformation(identity_quaternion,Vector(0,0,0));

#endif

