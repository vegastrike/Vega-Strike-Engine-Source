#ifndef _CMD_BOLT_H_
#define _CMD_BOLT_H_
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include "weapon_xml.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"

class Animation;
class Unit;
class Bolt {
  GFXColor col;
  Matrix drawmat;
  Vector cur_position;
  Vector prev_position;//beams don't change heading.
  weapon_info::WEAPON_TYPE type;//beam or bolt;
  unsigned char percentphase;//0 is 0% is phased damage, 255 is 100%
  int decal;//which image it uses
  Unit *owner;
  float damage, curdist,longrange;
  float speed, range,radius;
  Vector ShipSpeed;
  bool Collide (Unit * target);
 public:
  Bolt(const weapon_info &type, const Matrix orientationpos, const Vector & ShipSpeed, Unit *owner);//makes a bolt
  ~Bolt();
  static void Draw();
  bool Update();///www.cachunkcachunk.com
  bool Collide();
};
class bolt_draw {
public:
  class DecalQueue *boltdecals;
  static GFXVertexList * boltmesh;
  vector <string> animationname;
  vector <Animation *> animations;
  vector <vector <Bolt *> > bolts;
  vector <vector <Bolt *> > balls;
  vector <int> cachedecals;
  bolt_draw();
  ~bolt_draw();
  void UpdatePhysics();
};

#endif
