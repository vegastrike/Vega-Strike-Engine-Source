#include "collide_map.h"
#include "unit_generic.h"
#include "bolt.h"
CollideMap null_collide_map;

Collidable::Collidable(Unit *un):radius(un->rSize()){
  assert(!un->isSubUnit());
  this->SetPosition(un->LocalPosition());
  ref.unit=un;  
}
extern size_t nondecal_index(Collidable::CollideRef b);
template <class T> class CollideChecker
{public:static bool CheckCollisions(CollideMap* cm, T* un, const Collidable& collider){
  CollideMap::iterator tless,tmore;
  float sortedloc=sqrt(collider.GetMagnitudeSquared());
  float rad=collider.radius;
  float maxlook=sortedloc+2.0625*fabs(rad);
  float minlook=sortedloc-2.0625*fabs(rad);
  float maxsqr=maxlook*maxlook;
  float minsqr=minlook*minlook;
  bool isnew =isNew(cm,un);
  if (isnew==false && doUpdateKey(un)) {
    cm->changeKey(un->location,collider,tless,tmore);
  }else {
    tless=tmore=un->location;
    if (tless!=cm->begin())
      --tless;
    ++tmore;
  }
  if (un->location!=cm->begin()) {
    while((*tless)->GetMagnitudeSquared()>=minsqr) {
      bool boltSpecimen=(*tless)->radius<0;
      Collidable::CollideRef ref=(*tless)->ref;
      if (tless==cm->begin()) {
        if (boltSpecimen) {
          if (CheckCollision(un,collider,ref,**tless)){
            if (endAfterCollide(un)) {
              return true;       
            }else break;
          }else break;
          
        }else {
          if (CheckCollision(un,collider,ref.unit,**tless)){
            if (endAfterCollide(un)){
              return true;
            }else break;
          }else break;
        }        
      }else {
        if (boltSpecimen) {
          if (CheckCollision(un,collider,ref,**tless--)) {
            if (endAfterCollide(un))
              return true;        
          }
        }else {
          if (CheckCollision(un,collider,ref.unit,**tless--)){
            if (endAfterCollide(un)) 
              return true;      
          }
        }
      }
    
     
    }
  } 
  while (tmore!=cm->end()&&(*tmore)->GetMagnitudeSquared()<=maxsqr){
    bool boltSpecimen=(*tmore)->radius<0;
    Collidable::CollideRef ref=(*tmore)->ref;
    if (boltSpecimen) {
      if (CheckCollision(un,collider,ref,**tmore++))
        if (endAfterCollide(un))
          return true;      
    }else {
      if (CheckCollision(un,collider,ref.unit,**tmore++))
        if (endAfterCollide(un))
          return true;      
    }
  }
  
  return false;
}
  static bool doUpdateKey(Bolt * b) {
    return true;
  }
  static bool doUpdateKey (Unit *un) {
    return false;
  }
  static bool endAfterCollide(Bolt * b) {
    return true;
  }
  static bool endAfterCollide(Unit * un) {
    if (un->location==null_collide_map.begin())
      return true;
    return false;
  }
  static bool isNew(CollideMap * cm, Unit * b) {
    assert(!b->isSubUnit());
    if (b->location==null_collide_map.begin()) {
      b->location=cm->insert(Collidable(b));
      return true;
    }
    return false;
  }
  static bool isNew(CollideMap *cm, Bolt * b) {
    return false;
  }
  static bool Apart(const Collidable &a, const Collidable &b) {
    return (a.GetPosition()-b.GetPosition()).MagnitudeSquared()>a.radius*a.radius+fabs(a.radius*b.radius)*2+b.radius*b.radius;
  }
  static bool CheckCollision(Unit* a, const Collidable& aiter, Unit * b, const Collidable& biter) {
    if (!Apart(aiter,biter)) 
      return a->Collide(b);
    return false;
  }
  static bool CheckCollision(Bolt* a, const Collidable &aiter, Unit * b, const Collidable& biter) {
    if (!Apart(aiter,biter)) {
      if (a->Collide(b)) {
        a->Destroy(nondecal_index(aiter.ref));
        return true;
      }
    }
    return false;
  }
  static bool CheckCollision (Bolt * a, const Collidable &aiter, Collidable::CollideRef  b, const Collidable &biter) {
    return false;
  }
  static bool CheckCollision (Unit * un, const Collidable &aiter, Collidable::CollideRef b, const Collidable &biter) {
    if (!Apart(aiter,biter)) {
      return Bolt::CollideAnon(b,un);
    }
    return false;
  }
};


bool CollideMap::CheckCollisions (Bolt * bol, const Collidable &updated) {
  return CollideChecker<Bolt>::CheckCollisions(this,bol, updated);
}
bool CollideMap::CheckCollisions (Unit * un, const Collidable &updated) {
  //need to check beams
  if (un->activeStarSystem==NULL) {
    un->activeStarSystem = _Universe->activeStarSystem();
  } else {
    assert (un->activeStarSystem==_Universe->activeStarSystem());
  }
  return CollideChecker<Unit>::CheckCollisions(this,un, updated);
}
