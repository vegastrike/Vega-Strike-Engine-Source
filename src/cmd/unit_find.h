#ifndef _UNIT_FIND_H_
#define _UNIT_FIND_H_

template <class Locator> void findObjects (StarSystem * ss,CollideMap::iterator location, Locator *check) {
    if (location!=null_collide_map.begin()) {
      QVector thispos = (**location).GetPosition();
      float thisrad=fabs((*location)->radius);
      CollideMap::iterator tless=location;
      CollideMap::iterator tmore=location;
      
      ++tmore;
      CollideMap *cm=ss->collidemap;             
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
          if (check->BoltsOrUnits()||(check->UnitsOnly()==(*tless)->radius>0)) {
            float trad = check->NeedDistance()?((*tless)->GetPosition()-thispos).Magnitude()-fabs((*tless)->radius)-thisrad:0;
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
          if (check->BoltsOrUnits()||(check->UnitsOnly()==(*tmore)->radius>0)) {
            float trad = check->NeedDistance()?((*tmore)->GetPosition()-thispos).Magnitude()-fabs((*tmore)->radius)-thisrad:0;
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
    return rad!=FLT_MAX&&(startkey-rad)*(startkey-rad)>(*tless)->GetMagnitudeSquared();
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
		return (startkey-radius-maxUnitRadius)*(startkey-radius-maxUnitRadius)>(*tless)->GetMagnitudeSquared();
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


#endif
