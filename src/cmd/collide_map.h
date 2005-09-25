#include "key_mutable_set.h"
#include "vegastrike.h"
#include "gfx/vec.h"
class Unit;
class Bolt;
class Collidable{
  QVector pos;
public:
  float radius;//radius == 0, bolt  radius <0 beam, radius >0 unit
  union CollideRef{
    Unit * unit;
    unsigned int bolt_index;
  }ref;
  QVector GetPosition() const{
    return pos;
  }
  void SetPosition(const QVector &bpos) {
    //in case we want to drop in an xtra radius parameter when we get performance testing    
    this->pos=bpos;
  }
  bool operator <(const Collidable &other) const {
    return pos.MagnitudeSquared()<other.pos.MagnitudeSquared();
  }
  Collidable(Unit * un);
  Collidable(unsigned int bolt_index, float speed, const QVector &p){
    ref.bolt_index=bolt_index;
    radius=speed*SIMULATION_ATOM;
    this->pos=p;
  }
};
class CollideMap:public KeyMutableSet<Collidable> {
//Check collisions takes an item to check collisions with, and returns whether that item collided with a Unit only
  bool CheckCollisions(Bolt * bol, const Collidable & updated);
  bool CheckCollisions(Unit * un, const Collidable & updated);//will be handed off to a templated function
}
extern CollideMap null_collide_map;
