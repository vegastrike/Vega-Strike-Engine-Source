
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "in_joystick.h"
#include "cmd/unit.h"

FireKeyboard::FireKeyboard (int whichjoystick, const char *): Order (WEAPON){
  gunspeed = gunrange = .0001;

}
static KBSTATE firekey=UP;

static KBSTATE missilekey = UP;
static KBSTATE jfirekey=UP;
static KBSTATE jtargetkey=UP;
static KBSTATE jmissilekey = UP;
static KBSTATE weapk=UP;
static KBSTATE misk=UP;
static KBSTATE cloakkey=UP;
static KBSTATE neartargetkey=UP;
static KBSTATE threattargetkey=UP;
static KBSTATE picktargetkey=UP;
static KBSTATE targetkey=UP;

static KBSTATE nearturrettargetkey=UP;
static KBSTATE threatturrettargetkey=UP;
static KBSTATE pickturrettargetkey=UP;
static KBSTATE turrettargetkey=UP;




void FireKeyboard::CloakKey(int, KBSTATE k) {

    if (k==PRESS) {
      cloakkey = k;      
    }
}
void FireKeyboard::FireKey(int key, KBSTATE k) {
  if(firekey==DOWN && k==UP){
    return;
  }
  if (k==UP&&firekey==RELEASE) {

  } else {

    firekey = k;
    //    printf("firekey %d %d\n",k,key);
  }
}

void FireKeyboard::TargetKey(int, KBSTATE k) {
  if (targetkey!=PRESS)
    targetkey = k;
  if (k==RESET) {
    targetkey=PRESS;
  }
}
void FireKeyboard::PickTargetKey(int, KBSTATE k) {
  if (picktargetkey!=PRESS)
    picktargetkey = k;
  if (k==RESET) {
    picktargetkey=PRESS;
  }
}

void FireKeyboard::NearestTargetKey(int, KBSTATE k) {
  if (neartargetkey!=PRESS)
    neartargetkey = k;

}
void FireKeyboard::ThreatTargetKey(int, KBSTATE k) {
  if (threattargetkey!=PRESS)
    threattargetkey = k;
}


void FireKeyboard::TargetTurretKey(int, KBSTATE k) {
  if (turrettargetkey!=PRESS)
    turrettargetkey = k;
  if (k==RESET) {
    turrettargetkey=PRESS;
  }
}
void FireKeyboard::PickTargetTurretKey(int, KBSTATE k) {
  if (pickturrettargetkey!=PRESS)
    pickturrettargetkey = k;
  if (k==RESET) {
    pickturrettargetkey=PRESS;
  }
}

void FireKeyboard::NearestTargetTurretKey(int, KBSTATE k) {
  if (nearturrettargetkey!=PRESS)
    nearturrettargetkey = k;

}
void FireKeyboard::ThreatTargetTurretKey(int, KBSTATE k) {
  if (threatturrettargetkey!=PRESS)
    threatturrettargetkey = k;
}



void FireKeyboard::WeapSelKey(int, KBSTATE k) {
  if (weapk!=PRESS)
    weapk = k;
}
void FireKeyboard::MisSelKey(int, KBSTATE k) {
  if (misk!=PRESS)
    misk = k;
} 

void FireKeyboard::MissileKey(int, KBSTATE k) {
  if (missilekey!=PRESS)
   missilekey = k;
}
void FireKeyboard::ChooseNearTargets(bool turret) {
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un;
  float range=FLT_MAX;
  while ((un = iter.current())) {
    Vector t;
    bool tmp = parent->InRange (un,t);
    if (tmp&&t.Dot(t)<range&&t.k>0&&_Universe->GetRelation(parent->faction,un->faction)<0) {
      range = t.Dot(t);
      if (turret)
	parent->TargetTurret(un);
      parent->Target (un);
    }
    iter.advance();
  }
#ifdef ORDERDEBUG
  fprintf (stderr,"i4%x",iter);
  fflush (stderr);
#endif

#ifdef ORDERDEBUG
  fprintf (stderr,"i4\n");
  fflush (stderr);
#endif

}
void FireKeyboard::ChooseThreatTargets(bool turret) {
  Unit * threat = parent->Threat();
  if (threat) 
    parent->Target(threat);
  if (turret&&threat)
    parent->TargetTurret(threat);
}

void FireKeyboard::PickTargets(bool Turrets){
  UnitCollection::UnitIterator uiter = _Universe->activeStarSystem()->getUnitList()->createIterator();

  float smallest_angle=PI;

  Unit *other=uiter.current();
  Unit *found_unit=NULL;

  while(other){
    Vector t;
    if(other!=parent && parent->InRange(other,t)){
      Vector p,q,r;
      Vector vectothem=Vector(other->Position() - parent->Position()).Normalize();
      parent->GetOrientation(p,q,r);
      float angle=acos( vectothem.Dot(r) );
      
      if(angle<smallest_angle){
	found_unit=other;
	smallest_angle=angle;
      }
    }
    uiter.advance();
    other=uiter.current();
  }
  if (Turrets)
    parent->TargetTurret(found_unit);
  parent->Target(found_unit);

}

void FireKeyboard::ChooseTargets (bool turret) {
  UnitCollection::UnitIterator iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  bool found=false;
  bool find=false;
  while ((un = iter.current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    Vector t;

    if (un==parent->Target()) {
      iter.advance();
      found=true;
      continue;
    }
    if (!parent->InRange(un,t)) {
      iter.advance();
      continue;
    }
    iter.advance();
    if (found) {
      find=true;
      if (turret)
	parent->TargetTurret (un);
      parent->Target (un);
      break;
    }
  }
  //  if ((un = iter->current())) {


    //  }
#ifdef ORDERDEBUG
  fprintf (stderr,"i5%x",iter);
  fflush (stderr);
#endif

  
#ifdef ORDERDEBUG
  fprintf (stderr,"i5\n");
  fflush (stderr);
#endif
  if (!find) {
    iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
    while ((un = iter.current())) {
      //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
      Vector t;
      if (un==parent->Target()){
	iter.advance();
	continue;
      }
      if (!parent->InRange(un,t)) {
	iter.advance();
	continue;
      }
      if (turret)
	parent->TargetTurret(un);
      parent->Target(un);
      break;
    }
  }
}
FireKeyboard::~FireKeyboard () {
#ifdef ORDERDEBUG
  fprintf (stderr,"fkb%x",this);
  fflush (stderr);
#endif

}
bool FireKeyboard::ShouldFire(Unit * targ) {
  float dist;
  if (gunspeed==.0001) {
    parent->getAverageGunSpeed (gunspeed,gunrange);
  }
  float angle = parent->cosAngleTo (targ, dist,gunspeed,gunrange);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<.8*agg&&angle>1/agg);
}


void FireKeyboard::Execute () {
  Unit * targ;
  if ((targ = parent->Target())) {
    ShouldFire (targ);
  } else {
    ChooseTargets(false);
  }
  if (firekey==PRESS||jfirekey==PRESS||firekey==DOWN||jfirekey==DOWN) 
    parent->Fire(false);
  if (missilekey==DOWN||missilekey==PRESS||jmissilekey==PRESS||jmissilekey==DOWN) {
    parent->Fire(true);
    if (missilekey==PRESS)
      missilekey = DOWN;
    if (jmissilekey==PRESS)
      jmissilekey=DOWN;
  }
  else if (firekey==RELEASE||jfirekey==RELEASE) {
    firekey=UP;
    jfirekey=UP;
    parent->UnFire();
  }
  if (cloakkey==PRESS) {
    static bool toggle=true;
    cloakkey=DOWN;
    parent->Cloak(toggle);
    toggle=!toggle;
  }
  if (targetkey==PRESS||jtargetkey==PRESS) {
    targetkey=DOWN;
    jtargetkey=DOWN;
    ChooseTargets(false);
  }
  if(picktargetkey==PRESS){
    picktargetkey=DOWN;
    PickTargets(false);
  }
  if (neartargetkey==PRESS) {
    ChooseNearTargets (false);
    neartargetkey=DOWN;
  }
  if (threattargetkey==PRESS) {
    ChooseThreatTargets (false);
    threattargetkey=DOWN;
  }



  if (turrettargetkey==PRESS) {
    turrettargetkey=DOWN;
    ChooseTargets(true);
  }
  if(pickturrettargetkey==PRESS){
    pickturrettargetkey=DOWN;
    PickTargets(true);
  }
  if (nearturrettargetkey==PRESS) {
    ChooseNearTargets (true);
    nearturrettargetkey=DOWN;
  }
  if (threatturrettargetkey==PRESS) {
    ChooseThreatTargets (true);
    threatturrettargetkey=DOWN;
  }



  if (weapk==PRESS) {
    weapk=DOWN;
    parent->ToggleWeapon (false);
  }
  if (misk==PRESS) {
    misk=DOWN;
    parent->ToggleWeapon(true);
  }
}

