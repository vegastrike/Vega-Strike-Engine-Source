#ifndef _CMD_BOLT_H_
#define _CMD_BOLT_H_
#include "weapon_xml.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"

class Animation;

class Bolt {
  Matrix curr_physical_state;
  Vector prev_position;//beams don't change heading.
  weapon_info::WEAPON_TYPE type;//beam or bolt;
  union {
    Animation * anim;//ball
    int ref;//which image it uses
  };
  float damage, curdist,longrange;
  float speed, range;
 public:
  Bolt(const Matrix orientationpos, float r, float g,float b, float a, float rad, float len);//makes a bolt
  Bolt(const Matrix orientationpos, const char * animation, float size);//makes a ball
  ~Bolt();
  void Draw();
  void UpdatePhysics();///www.cachunkcachunk.com
  void CollideAll();//does a point sampling of the collision table and finally intersect w/ a line
};

#endif
