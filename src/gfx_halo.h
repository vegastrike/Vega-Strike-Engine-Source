#include "gfx_transform_vector.h"
#include "quaternion.h"


class Halo {
  Transformation local_transformation;  
  //enum BLENDFUNC blendSrc;
  //enum BLENDFUNC blendDst;
 public:
  Halo();
  ~Halo();
  void Draw (const Transformation &quat = identity_transformation, const Matrix m = identity_matrix);
  void SetDimensions(float wid, float hei);
  Vector &Position();
  void SetPosition (float, float, float);
  void SetPosition (const Vector &);
  void SetOrientation(const Vector &p, const Vector &q, const Vector &r);

}
