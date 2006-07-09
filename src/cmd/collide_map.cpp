#include "collide_map.h"
#include "unit_generic.h"
#include "bolt.h"
CollideMap null_collide_map;
CollideMap::iterator null_collide_iter;
bool null_collide_iter_initialized = false;
#define collisionperf

#ifdef collisionperf
int boltcalls=0;
int boltchecks=0;
int boltonboltchecks=0;
int unitcalls=0;
int unitchecks=0;
#endif

Collidable::Collidable(Unit *un):radius(un->rSize()){
  assert(!un->isSubUnit());
  this->SetPosition(un->LocalPosition());
  ref.unit=un;  
}
extern size_t nondecal_index(Collidable::CollideRef b);
template <class T, bool canbebolt> class CollideChecker
{public:static bool CheckCollisions(CollideMap* cm, T* un, const Collidable& collider){
  CollideMap::iterator tless,tmore;
  float sortedloc=sqrt(collider.GetMagnitudeSquared());
  float rad=collider.radius;
  float maxlook=sortedloc+2.0625*fabs(rad);
  float minlook=sortedloc-2.0625*fabs(rad);
  float maxsqr=maxlook*maxlook;
  float minsqr=minlook*minlook;
  bool isnew =isNew(cm,un);
#ifdef collisionperf
  bool isbolt = rad<0;
  if(isbolt){
		boltcalls++;
  }else{
	  unitcalls++;
  }
#endif
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
	#ifdef collisionperf
	  if(isbolt){
	 	boltchecks++;
	  }else{
	    unitchecks++;
	  }
	#endif
      bool boltSpecimen=canbebolt&&((*tless)->radius<0);
#ifdef collisionperf
	  if(isbolt&&boltSpecimen){
		boltonboltchecks++;
	  }
#endif
      Collidable::CollideRef ref=(*tless)->ref;
      if (tless==cm->begin()) {
        if (canbebolt&&boltSpecimen) {
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
        if (canbebolt&&boltSpecimen) {
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
	#ifdef collisionperf
	  if(isbolt){
	 	boltchecks++;
	  }else{
	    unitchecks++;
	  }
	#endif
    bool boltSpecimen=canbebolt&&((*tmore)->radius<0);
	#ifdef collisionperf
	  if(isbolt&&boltSpecimen){
		boltonboltchecks++;
	  }
	#endif
    Collidable::CollideRef ref=(*tmore)->ref;
    if (canbebolt&&boltSpecimen) {
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
	return is_null(un->location);
  }
  static bool isNew(CollideMap * cm, Unit * b) {
    assert(!b->isSubUnit());
    if (is_null(b->location)) {
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
  return CollideChecker<Bolt,true>::CheckCollisions(this,bol, updated);
}

bool CollideMap::CheckUnitCollisions (Bolt * bol, const Collidable &updated) {
  return CollideChecker<Bolt,false>::CheckCollisions(this,bol, updated);
}
bool CollideMap::CheckCollisions (Unit * un, const Collidable &updated) {
  //need to check beams
  if (un->activeStarSystem==NULL) {
    un->activeStarSystem = _Universe->activeStarSystem();
  } else {
    assert (un->activeStarSystem==_Universe->activeStarSystem());
  }
  return CollideChecker<Unit,true>::CheckCollisions(this,un, updated);
}


bool CollideMap::CheckUnitCollisions (Unit * un, const Collidable &updated) {
  //need to check beams
  if (un->activeStarSystem==NULL) {
    un->activeStarSystem = _Universe->activeStarSystem();
  } else {
    assert (un->activeStarSystem==_Universe->activeStarSystem());
  }
  return CollideChecker<Unit,false>::CheckCollisions(this,un, updated);
}
