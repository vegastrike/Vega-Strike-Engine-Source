#include "vec.h"
#include "animation.h"
#include "in_mouse.h"

class CoordinateSelect{
 protected:
  float CrosshairSize;
  Animation LocSelAni;
  QVector LocalPosition;
  void UpdateMouse();
 public:
  static void MouseMoveHandle (KBSTATE,int,int,int,int,int);
  CoordinateSelect (QVector);//wish I could warp mouse pointer there
  ~CoordinateSelect(){}
  inline QVector GetVector (){return LocalPosition;}
  void Draw();
};
  
