#include "gfx_transform_vector.h"
#include "gfx_primitive.h"
#include "gfx_animation.h"
#include "in_mouse.h"
class LocationSelect:public Primitive {
protected:
  float CrosshairSize;
  //  System *parentSystem;
  Animation LocSelAni;
  Animation LocSelUpAni;
  Vector p,q,r;
  Vector LocalPosition;

public:
  static void MouseMoveHandle (KBSTATE,int,int,int,int,int);
  LocationSelect (Vector,Vector,Vector/*, Scene*   */);
  LocationSelect (Vector,Vector,Vector,Vector/*, Scene*   */);
  ~LocationSelect();
  inline Vector GetVector (){return Transform (cumulative_transformation_matrix,LocalPosition);}
  void MoveLocation (Vector pos,Vector p ,Vector q);
  void MoveLocation (Vector pos, Vector p, Vector q, Vector r); //requires normalized Vector triplet pqr
  void Draw();
};
