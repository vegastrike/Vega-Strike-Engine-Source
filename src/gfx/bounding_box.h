#ifndef _GFX_BOUNDING_BOX_H_
#define _GFX_BOUNDING_BOX_H_
#include "matrix.h"
#include "quaternion.h"

class BoundingBox {
 private:
  QVector lx,ly,lz;
  QVector mx,my,mz;

 public:
  BoundingBox (const QVector &LX, const QVector &MX,const QVector &LY,const QVector &MY,const QVector &LZ,const QVector &MZ);
  void Transform (const Matrix &t);
  void Transform (const Transformation &transform);
  QVector Center (){
    return QVector (((mx+lx)+(my+ly)+(mz+lz))).Scale(.16666666666666666);
  }
  float ZCenter () {
    return .166666666666666666F*(mx.k+lx.k+my.k+ly.k+mz.k+lz.k);
  }
  bool Within (const QVector &query,float err);
  bool OpenWithin (const QVector &query,float err, int exclude);
  int Intersect (const QVector &eye, const QVector &pnt, float err);
};

#endif

