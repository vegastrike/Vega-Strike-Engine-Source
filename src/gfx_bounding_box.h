#ifndef _GFX_BOUNDING_BOX_H_
#define _GFX_BOUNDING_BOX_H_
#include "gfx_transform_matrix.h"
#include "quaternion.h"

class BoundingBox {
 private:
  Vector lx,ly,lz;
  Vector mx,my,mz;

 public:
  BoundingBox (Vector LX, Vector MX,Vector LY,Vector MY,Vector LZ,Vector MZ);
  void Transform (Matrix t);
  void Transform (const Transformation &transform);
  Vector Center (){
    return Vector (.16666666666666666F*((mx+lx)+(my+ly)+(mz+lz)));
  }
  float ZCenter () {
    return .166666666666666666F*(mx.k+lx.k+my.k+ly.k+mz.k+lz.k);
  }
  bool Within (const Vector &query,float err);
  bool OpenWithin (const Vector &query,float err, int exclude);
  int Intersect (const Vector &eye, const Vector &pnt, float err);
};

#endif
