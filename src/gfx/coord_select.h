#include "vec.h"
#include "primitive.h"
#include "animation.h"
#include "in_mouse.h"

class CoordinateSelect{
 protected:
  float CrosshairSize;
  Animation LocSelAni;
  Vector LocalPosition;
  void UpdateMouse();
 public:
  static void MouseMoveHandle (KBSTATE,int,int,int,int,int);
  CoordinateSelect (Vector);//wish I could warp mouse pointer there
  ~CoordinateSelect(){}
  inline Vector GetVector (){return LocalPosition;}
  void Draw();
};
  
