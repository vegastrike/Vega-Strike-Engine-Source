#ifndef _UNIT_FIND_H_
#define _UNIT_FIND_H_
#include "unit_util.h"
template <class Locator> void findObjects (StarSystem * ss,CollideMap::iterator location, Locator *check) {
    if (!is_null(location)) {
      QVector thispos = (**location).GetPosition();
      float thisrad=fabs((*location)->radius);
      CollideMap::iterator tless=location;
      CollideMap::iterator tmore=location;
      CollideMap *cm=ss->collidemap;             
      if (tmore!=cm->end())        
        ++tmore;
      check->init(cm,location);
      bool workA=true;
      bool workB=true;
      if (tless!=cm->begin())
        --tless;
      else 
        workA=false;      
      while(workA||workB){
        if (workA
            &&!check->cullless(tless)) {
          float rad=(*tless)->radius;
          if (rad!=0.0f&&(check->BoltsOrUnits()||(check->UnitsOnly()==(rad>0)))) {
            float trad = check->NeedDistance()?((*tless)->GetPosition()-thispos).Magnitude()-fabs(rad)-thisrad:0;
            if (!check->acquire(trad,tless)) {
              workA=false;
            }
          }
          if (tless!=cm->begin()) 
            tless--;
          else
            workA=false;
        }else workA=false;
        if (workB
            &&tmore!=cm->end()
            &&!check->cullmore(tmore)) {
          float rad = (*tmore)->radius;
          if (rad!=0.0f&&(check->BoltsOrUnits()||(check->UnitsOnly()==rad>0))) {
            float trad = check->NeedDistance()?((*tmore)->GetPosition()-thispos).Magnitude()-fabs(rad)-thisrad:0;
            if (!check->acquire(trad,tmore)) {
              workB=false;
            }
          }
          tmore++;
        }else workB=false;
      }      
    }
}

class NearestUnitLocator {
  CollideMap::iterator location;
  float startkey;
  float rad;
public:
  Collidable::CollideRef retval;
  bool BoltsOrUnits() {
    return false;
  }
  bool UnitsOnly() {
    return true;
  }
  bool NeedDistance() {
    return true;
  }
  NearestUnitLocator() {
    retval.unit=NULL;
  }
  void init (CollideMap * cm, CollideMap::iterator parent) {
    this->location=parent;
    startkey = sqrt((*location)->GetMagnitudeSquared());
    rad=FLT_MAX;
    retval.unit=NULL;
  }
  bool cullless (CollideMap::iterator tless) {
    return rad!=FLT_MAX&&(startkey-rad>0)&&(startkey-rad)*(startkey-rad)>(*tless)->GetMagnitudeSquared();
  }
  bool cullmore (CollideMap::iterator tmore) {
    return rad!=FLT_MAX&&(startkey+rad)*(startkey+rad)<(*tmore)->GetMagnitudeSquared();
  }
  bool acquire(float distance, CollideMap::iterator i) {
    if (distance<rad) {
      rad=distance;
      retval=(*i)->ref;
    }
    return true;
  }
};
class NearestBoltLocator :public NearestUnitLocator{
public:
  bool UnitsOnly() {
    return false;
  }
};
class NearestObjectLocator:public NearestUnitLocator{
public:
  bool isUnit;
  bool UnitsOnly() {return false;}
  bool BoltsOrUnits() {return true;}
  bool acquire(float distance, CollideMap::iterator i) {  
    Collidable::CollideRef lastret=retval;
    bool retval=NearestUnitLocator::acquire(distance,i);
    if (memcmp((void*)&retval,(void*)&lastret,sizeof(Collidable::CollideRef))){
      isUnit=(*i)->radius>0;
    }
    return retval;
  }
};
class NearestNavLocator:public NearestUnitLocator{
public:
  bool BoltsOrUnits() {
    return false;
  }
  bool UnitsOnly() {
    return true;
  }
  bool acquire(float distance, CollideMap::iterator i) {
    if (UnitUtil::isSignificant((*i)->ref.unit))
      return NearestUnitLocator::acquire(distance,i); 
    return true;
  }
};
template <class T>
class UnitWithinRangeLocator {
public:
	T action;
	float startkey;
	float radius;
	float maxUnitRadius;
	UnitWithinRangeLocator( float radius, float maxUnitRadius)
			: startkey(0), radius(radius), maxUnitRadius(maxUnitRadius) {
	}

	bool UnitsOnly() {return true;}
	bool BoltsOrUnits() {return false;}
	bool NeedDistance() {return true;}
	
	void init (CollideMap * cm, CollideMap::iterator parent) {
		startkey=sqrt((*parent)->GetMagnitudeSquared());
	}
	bool cullless (CollideMap::iterator tless) {
		float tmp=startkey-radius-maxUnitRadius;
		return tmp>0&&(tmp*tmp>(*tless)->GetMagnitudeSquared());
	}
	bool cullmore (CollideMap::iterator tmore) {
		return (startkey+radius+maxUnitRadius)*(startkey+radius+maxUnitRadius)<(*tmore)->GetMagnitudeSquared();
	}

	bool acquire(float dist, CollideMap::iterator i) {
		if (dist<radius) {
			// Inside radius...
			return action.acquire((*i)->ref.unit, dist);
		}
		return true;
	}
};
class UnitPtrLocator {
  const void * unit;
public:
  bool retval;
  bool BoltsOrUnits() { return false; }
  bool UnitsOnly() { return true; }
  bool NeedDistance() { return false; }
  UnitPtrLocator(const void * unit) { retval = false; this->unit=unit; }
  bool cullless (CollideMap::iterator tless) { return retval; }
  bool cullmore (CollideMap::iterator tmore) { return retval; }
  bool acquire(float distance, CollideMap::iterator i) { return (retval = (((const void*)((*i)->ref.unit)) == unit)); }
  void init (CollideMap * cm, CollideMap::iterator parent) { }
};


#endif
