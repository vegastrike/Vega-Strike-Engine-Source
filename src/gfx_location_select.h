#include "gfx_transform_vector.h"
#include "gfx_primitive.h"
#include "in_mouse.h"
class LocationSelect:public Primitive {
protected:
  static const float CrosshairSize=2;
  //  System *parentSystem;
  Vector p,q,r;
  Vector LocalPosition;
  void UpdateMatrix();

public:

  LocationSelect (Vector,Vector,Vector/*, Scene*   */);
  LocationSelect (Vector,Vector,Vector,Vector/*, Scene*   */);
  ~LocationSelect();
  void MoveLocation (Vector pos,Vector p ,Vector q);
  void MoveLocation (Vector pos, Vector p, Vector q, Vector r); //requires normalized Vector triplet pqr
  void Draw();
};
