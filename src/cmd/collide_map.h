#ifndef _COLLIDE_MAP_H_
#define _COLLIDE_MAP_H_
#include "key_mutable_set.h"
#include "vegastrike.h"
#include "gfx/vec.h"
#include <limits>
#include <vector>
/* Arbitrarily use Set for ALL PLATFORMS -hellcatv */
//#define VS_ENABLE_COLLIDE_KEY
class Unit;
class Bolt;
class Collidable{
  QVector position;
public:
  float radius;//radius == 0: to-be-deleted, radius <0 bolt (radius == speed in phys frame), radius >0 unit
  
  union CollideRef{
    Unit * unit;
    unsigned int bolt_index;
  }ref;
  QVector GetPosition() const{
    return position;
  }
  void SetPosition(const QVector &bpos) {
    //in case we want to drop in an xtra radius parameter when we get performance testing    
    this->position=bpos;
    /*
    key=bpos.MagnitudeSquared();
    */
    if (ISNAN(getKey())) {
      position=QVector(0,0,0);//hack for now      
    }
  }
  Collidable &operator* () {return *this;}
  Collidable *operator-> () {return this;}

  //float GetMagnitudeSquared()const {return key;}
  double getKey()const {return position.i;}
  bool operator <(const Collidable &other) const {
    return getKey()<other.getKey();
  }
  Collidable &get () {return *this;}
  Collidable() : radius(std::numeric_limits<float>::quiet_NaN()) {}
  Collidable(Unit * un);
  Collidable(unsigned int bolt_index, float speed, const QVector &p){
    ref.bolt_index=bolt_index;
    radius=-speed*SIMULATION_ATOM;
    if (ISNAN(radius)||radius>=-FLT_MIN) radius=-FLT_MIN*2;
    this->SetPosition(p);
  }
};


class CollideArray{
public:
  unsigned int location_index;//either UNIT_ONLY or UNIT_BOLT
  class  CollidableBackref:public Collidable{
  public:
    
    size_t toflattenhints_offset;
    CollidableBackref() : Collidable(){}
    CollidableBackref(Unit * un):Collidable(un){}
    CollidableBackref(unsigned int bolt_index, float speed, const QVector &p):Collidable(bolt_index,speed,p) {}
    CollidableBackref(const Collidable &b,size_t offset):Collidable(b){toflattenhints_offset=offset;}
    
  };
  void SetLocationIndex(unsigned int li) {location_index=li;}
  typedef Collidable *iterator;
  bool Iterable(iterator);
  typedef std::vector<Collidable> ResizableArray;
  ResizableArray sorted;
  ResizableArray unsorted;
  std::vector<std::list<CollidableBackref> >toflattenhints;
  int count;
  void UpdateBoltInfo(iterator iter,Collidable::CollideRef ref);
  void flatten();
  void flatten(CollideArray &example);//maybe it has some xtra bolts
  iterator insert (const Collidable & newKey,iterator hint);
  iterator insert (const Collidable & newKey);
  iterator changeKey (iterator iter, const Collidable & newKey);
  iterator changeKey (iterator iter, const Collidable & newKey, iterator tless, iterator tmore);
  iterator begin() {return &*sorted.begin();}
  iterator end() {return this->begin()+sorted.size();}
  iterator lower_bound(const Collidable&);
  void erase(iterator iter);
  void checkSet ();
  CollideArray(unsigned int location_index):toflattenhints(1),count(0) {
    this->location_index=location_index;
  }

};

#ifdef VS_ENABLE_COLLIDE_KEY
class CollideMap:public KeyMutableSet<Collidable> {
#else
#ifdef VS_ENABLE_COLLIDE_LIST
class CollideMap:public ListMutableSet<Collidable> {
#else
class CollideMap:public CollideArray {
#endif
#endif
public:
  CollideMap(unsigned int location_offset):CollideArray(location_offset){
  }
  
//Check collisions takes an item to check collisions with, and returns whether that item collided with a Unit only
  bool CheckCollisions(Bolt * bol, const Collidable & updated);
  bool CheckUnitCollisions(Bolt * bol, const Collidable & updated);//DANGER must be used on lists that are only populated with Units, not bolts
  bool CheckCollisions(Unit * un, const Collidable & updated);//will be handed off to a templated function
  bool CheckUnitCollisions(Unit * un, const Collidable & updated);//DANGER must be used on lists that are only populated with Units, not bolts
};

#if defined(VS_ENABLE_COLLIDE_LIST)||defined(VS_ENABLE_COLLIDE_KEY)
extern CollideMap null_collide_map;
extern CollideMap::iterator null_collide_iter;
extern bool null_collide_iter_initialized;

inline void init_null_collide_iter()
{
	if (!null_collide_iter_initialized) {
		null_collide_map.insert(Collidable());
		null_collide_iter = null_collide_map.begin();
		null_collide_iter_initialized = true;
	}
}

inline bool is_null(const CollideMap::iterator &it)
{
  return ISNAN((**it).radius);
}

inline void set_null(CollideMap::iterator &it)
{
	init_null_collide_iter();
	it = null_collide_iter;
}
#else
inline void init_null_collide_iter(){}
inline bool is_null(const CollideMap::iterator &it)
{
  return it==NULL;
}
inline void set_null(CollideMap::iterator &it)
{
	it = NULL;
}

#endif
#endif
