#include "gfx_click_list.h"
#include "gfx_transform_vector.h"
#include "gfx_primitive.h"
#include "gfx_animation.h"
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
  
