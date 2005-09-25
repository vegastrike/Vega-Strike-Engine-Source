#include "collide_map.h"
#include "unit_generic.h"
CollideMap null_collide_map;

Collidable::Collidable(Unit *un):radius(un->rSize()),pos(un->LocalPosition()){
  ref.unit=un;  
}

template <class T, bool isBolt> class CollideChecker
{static bool CheckCollisions(CollideMap* cm, T* un, const Collidable& collider){
  CollideMap::iterator tless,tmore;
  cm->changeKey(t->location,collider,tless,tmore);
  if (tless!=location) {
    
  }
  for(;tmore!=cm->end();++tmore){
    
  }
  
  return false;
}};


bool CollideMap::CheckCollisions (Bolt * bol, const Collidable &updated) {
  return CollideChecker<Bolt,true>::CheckCollisions(this,bol, updated);
}
bool CollideMap::CheckCollisions (Unit * un, const Collidable &updated) {
  return CollideChecker<Unit,true>::CheckCollisions(this,un, updated);
}
