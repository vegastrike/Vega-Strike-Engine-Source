#ifndef _3DMANIP_H_
#define _3DMANIP_H_
#include "macosx_math.h"
#include <math.h>

#include "endianness.h"

#define QFLOAT float
#define NetSwap VSSwapHostFloatToLittle
#define XSQRT sqrtf
#define XVector Vector
#define YVector QVector
#include "xvector.h"
#undef NetSwap
#undef QFLOAT
#undef XVector
#undef YVector
#undef XSQRT

#define QFLOAT double
#define NetSwap VSSwapHostDoubleToLittle
#define XSQRT sqrt
#define XVector QVector
#define YVector Vector
#include "xvector.h"
#undef XSQRT
#undef NetSwap
#undef QFLOAT
#undef XVector
#undef YVector

inline Vector QVector::operator =(const Vector &a) {
  i=a.i;
  j=a.j;
  k=a.k;
  return a;
}
inline QVector::QVector (const Vector &a) {
  i=a.i;
  j=a.j;
  k=a.k;
}
inline QVector Vector::operator = (const QVector &a) {
  i=a.i;
  j=a.j;
  k=a.k;
  return a;
}
inline Vector::Vector (const QVector &a) {
  i=a.i;
  j=a.j;
  k=a.k;
}
inline QVector Vector::Cast() const{
  return QVector (i,j,k);
}
inline Vector QVector::Cast() const{
  return Vector (i,j,k);
}

#endif

