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
private:
  const weapon_info* type;//beam or bolt;
  Matrix drawmat;
  QVector cur_position;
  Vector ShipSpeed;
  QVector prev_position;//beams don't change heading.
  Unit *owner;
  float curdist;
  int decal;//which image it uses
  bool Collide (Unit * target);
 public:
  bool operator == (const Bolt & b) const{
    
    return owner==b.owner
      &&curdist==b.curdist
      &&cur_position==b.cur_position
      &&prev_position==b.prev_position;
  }
  Bolt(const weapon_info *type, const Matrix &orientationpos, const Vector & ShipSpeed, Unit *owner);//makes a bolt
  void Destroy(int index);
  static void Draw();
  bool Update(int index);
  bool Collide(int index);
  void noop()const{}
};
class bolt_draw {
public:
  class DecalQueue *boltdecals;
  static GFXVertexList * boltmesh;
  vector <string> animationname;
  vector <Animation *> animations;
  vector <vector <Bolt> > bolts;
  vector <vector <Bolt> > balls;
  vector <int> cachedecals;
  bolt_draw();
  ~bolt_draw();
  void UpdatePhysics();
};

#endif
