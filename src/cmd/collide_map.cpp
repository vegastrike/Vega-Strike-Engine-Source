#include <algorithm>
#include "collide_map.h"
#include "unit_generic.h"
#include "bolt.h"

void CollideArray::erase(iterator target) {
  count-=1;
  if (target>=this->begin()&&target<this->end()) {
    target->radius=0;
    target->ref.unit=NULL;
    iterator tmp = &*(this->unsorted.begin()+(target-this->begin()));
    tmp->radius=0;
    tmp->ref.unit=NULL;    
    return;
  }else if (target==NULL) {
    return;
  }else {
    CollidableBackref * targ=static_cast<CollidableBackref*>(&*target);
    std::list<CollidableBackref>* targlist=&toflattenhints[targ->toflattenhints_offset];
    std::list<CollidableBackref>::iterator endlist=targlist->end();
    for (std::list<CollidableBackref>::iterator i=targlist->begin();i!=endlist;++i){
      if (&*i==target) {
        targlist->erase(i);
        return;
      }
    }    
  }  
}
void CollideArray::UpdateBoltInfo (CollideArray::iterator iter, Collidable::CollideRef ref) {
  if (iter>=this->begin()&&iter<this->end()) {  
    (*(unsorted.begin()+(iter-this->begin()))).ref=ref;//update both unsorted and sorted
    (*iter).ref=ref;
  }else if (!is_null(iter)){
    (*iter).ref=ref;
  }
}
CollideArray::iterator CollideArray::changeKey(CollideArray::iterator iter, const Collidable &newKey) {
  if (iter>=this->begin()&&iter<this->end()) {  
    iterator tmp = &*(this->unsorted.begin()+(iter-this->begin()));
    *tmp=newKey;
  }else {
    *iter=newKey;
  }
  return iter;
}
CollideArray::iterator CollideArray::changeKey(CollideArray::iterator iter, const Collidable &newKey, CollideArray::iterator tless, CollideArray::iterator tmore) {
  
  return this->changeKey(iter,newKey);
}

class UpdateBackpointers{
public:
  void operator() (Collidable &collidable) {
    StarSystem * ss=_Universe->activeStarSystem();
    assert(collidable.radius!=0.0f);
    if (collidable.radius<0) {
      Bolt::BoltFromIndex(ss,collidable.ref)->location=&collidable;
    }else {
      collidable.ref.unit->location=&collidable;
    }
  }
};
void CollideArray::flatten () {
  sorted.resize(count);
  size_t len=unsorted.size();
  size_t index=0;
  for (size_t i=0;i<=len;++i) {
    Collidable * tmp;
    if (i<len&&(tmp=&unsorted[i])->radius!=0.0f) {
      sorted[index++]=*tmp;
    }
    std::list<CollidableBackref>::iterator listend=toflattenhints[i].end();
    for (std::list<CollidableBackref>::iterator j=toflattenhints[i].begin();
         j!=listend;
         ++j) {
      if (j->radius!=0){
        sorted[index++]=*j;
      }
    }
    toflattenhints[i].resize(0);
  }
  std::sort(sorted.begin(),sorted.end());
  unsorted=sorted;
 
  toflattenhints.resize(count+1);
  for_each(sorted.begin(),sorted.end(),UpdateBackpointers());
}



CollideArray::iterator CollideArray::insert(const Collidable &newKey, iterator hint) {
  if (this->begin()==this->end()){
    count+=1;
    this->unsorted.push_back(newKey);
    this->toflattenhints.resize(2);    
    this->sorted.push_back(newKey);    
    return &sorted.back();
  }else if (hint>=this->begin()&&hint<=this->end()) {
    count+=1;
    size_t len = hint-this->begin();
    std::list<CollidableBackref> *hintlist=&toflattenhints[len];
    return &*hintlist->insert(hintlist->end(),CollidableBackref(newKey,len));
  }else {
    return this->insert(newKey);//don't use hint;
  }
}



CollideArray::iterator CollideArray::lower_bound(const Collidable &newKey) {
  return ::std::lower_bound(this->begin(),this->end(),newKey);
}

CollideArray::iterator CollideArray::insert(const Collidable &newKey) {
  return this->insert(newKey,this->lower_bound(newKey));
}
void CollideArray::checkSet() {
 
  if (this->begin()!=this->end()) {
    iterator iter;
    for (iterator newiter=this->begin(), iter=newiter++;newiter!=this->end();iter=newiter++) {
      assert(*iter<*newiter);
    }
  }
}




CollideMap null_collide_map;
CollideMap::iterator null_collide_iter;
bool null_collide_iter_initialized = false;
//#define collisionperf

#ifdef collisionperf
int boltcalls=0;
int boltchecks=0;
int boltonboltchecks=0;
int unitcalls=0;
int unitchecks=0;
int boltruns=0;
bool seenunit=false;
#endif

Collidable::Collidable(Unit *un){
  radius=un->rSize();
  
  if (radius<=FLT_MIN||ISNAN(radius)) radius=2*FLT_MIN;
  assert(!un->isSubUnit());
  this->SetPosition(un->LocalPosition());
  ref.unit=un;  
}
bool CollideArray::Iterable(CollideArray::iterator a){ 
  return a>=begin()&&a<end();
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
  CollideMap::iterator cmbegin= cm->begin();
  CollideMap::iterator cmend= cm->end();
  if (cmbegin==cmend) return false;
  //CollideChecker<T,canbebolt>::isNew(cm,un); Now we do this in the caller
  if (!cm->Iterable(un->location)) {
    //fprintf (stderr,"ERROR: New collide map entry checked for collision\n Aborting collide\n");
    CollideArray::CollidableBackref * br=static_cast<CollideArray::CollidableBackref*>(un->location);    
    CollideMap::iterator tmploc=cmbegin+br->toflattenhints_offset;
    if (tmploc==cmend)
      tmploc--;
    tless=tmore=tmploc;//don't decrease tless
  }else {
    tless=tmore=un->location;
    if (tless!=cmbegin)
      --tless;

  }
  ++tmore;

#ifdef collisionperf
  bool isbolt = rad<0;
  if(isbolt){
		boltcalls++;
		seenunit=false;
  }else{
	  unitcalls++;
  }
#endif
  if (un->location!=cmbegin) {//if will happen in case of !Iterable
    while((*tless)->GetMagnitudeSquared()>=minsqr) {
	#ifdef collisionperf
	  if(isbolt){
	 	boltchecks++;
	  }else{
	    unitchecks++;
	  }
	#endif
          float rad=(*tless)->radius;
          bool boltSpecimen=canbebolt&&(rad<0);
          
#ifdef collisionperf
	  if(!seenunit&&!boltSpecimen){
		  seenunit=true;
	  }
	  if(isbolt&&boltSpecimen){
		boltonboltchecks++;
		if(!seenunit){
			boltruns++;
		}
	  }
#endif
      Collidable::CollideRef ref=(*tless)->ref;
      if (tless==cmbegin) {
        if (canbebolt&&boltSpecimen) {
          if (CheckCollision(un,collider,ref,**tless)){
            if (endAfterCollide(un)) {
              return true;       
            }else break;
          }else break;
          
        }else if (rad!=0){
          if (CheckCollision(un,collider,ref.unit,**tless)){
            if (endAfterCollide(un)){
              return true;
            }else break;
          }else break;
        }else break;        
      }else {
        if (canbebolt&&boltSpecimen) {
          if (CheckCollision(un,collider,ref,**tless--)) {
            if (endAfterCollide(un))
              return true;        
          }
        }else if (rad!=0){
          if (CheckCollision(un,collider,ref.unit,**tless--)){
            if (endAfterCollide(un)) 
              return true;      
          }
        }else {
          --tless;
        }
      }
    
     
    }
  }
  while (tmore!=cmend&&(*tmore)->GetMagnitudeSquared()<=maxsqr){
#ifdef collisionperf
    if(isbolt){
      boltchecks++;
    }else{
      unitchecks++;
    }
#endif
    float rad=(*tmore)->radius;
    bool boltSpecimen=canbebolt&&(rad<0);
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
    }else if (rad!=0){//not null unit
      if (CheckCollision(un,collider,ref.unit,**tmore++))
        if (endAfterCollide(un))
          return true;      
    }else ++tmore;
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
  /*
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
    }*/ // no longer necessary--done in caller
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
