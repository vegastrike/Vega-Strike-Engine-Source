
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "in_joystick.h"


FireKeyboard::FireKeyboard (int whichjoystick, const char *): Order (WEAPON){
  gunspeed = gunrange = .0001;
#if 0
  BindJoyKey (whichjoystick,0,FireKeyboard::JFireKey);
  BindJoyKey (whichjoystick,1,FireKeyboard::JMissileKey);
  BindJoyKey (whichjoystick,3,FireKeyboard::JTargetKey);

  BindKey(' ',FireKeyboard::FireKey);
  BindKey(13,FireKeyboard::MissileKey);
  BindKey('t',FireKeyboard::TargetKey);
  BindKey('g',FireKeyboard::WeapSelKey);
  BindKey('m',FireKeyboard::MisSelKey);

#endif
}
static KBSTATE firekey=UP;
static KBSTATE targetkey=UP;
static KBSTATE missilekey = UP;
static KBSTATE jfirekey=UP;
static KBSTATE jtargetkey=UP;
static KBSTATE jmissilekey = UP;
static KBSTATE weapk=UP;
static KBSTATE misk=UP;
static KBSTATE cloakkey=UP;

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

void FireKeyboard::JFireKey(KBSTATE k, float, float,int i) {
  if (k==UP&&jfirekey==RELEASE) {

  } else {
    jfirekey = k;
  }
} 
void FireKeyboard::TargetKey(int, KBSTATE k) {
  if (targetkey!=PRESS)
    targetkey = k;
  if (k==RESET) {
    targetkey=PRESS;
  }
}
void FireKeyboard::JTargetKey(KBSTATE k, float, float,int i) {
  if (jtargetkey!=PRESS)
    jtargetkey = k;
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
  missilekey = k;
}
void FireKeyboard::JMissileKey(KBSTATE k, float, float,int i) {
  jmissilekey = k;
} 


void FireKeyboard::ChooseTargets () {
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  bool found=false;
  bool find=false;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    Vector t;

    if (un==parent->Target()) {
      iter->advance();
      found=true;
      continue;
    }
    if (!parent->InRange(un,t)) {
      iter->advance();
      continue;
    }
    iter->advance();
    if (found) {
      find=true;
      parent->Target (un);
      break;
    }
  }
  //  if ((un = iter->current())) {


    //  }
  delete iter;
  if (!find) {
    iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
    while ((un = iter->current())) {
      //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
      Vector t;
      if (un==parent->Target()){
	iter->advance();
	continue;
      }
      if (!parent->InRange(un,t)) {
	iter->advance();
	continue;
      }
      parent->Target(un);
      break;
    }
    delete iter;
  }
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
    ChooseTargets();
  }
  if (firekey==DOWN||jfirekey==DOWN) 
    parent->Fire(false);
  if (missilekey==DOWN||jmissilekey==DOWN)
    parent->Fire(true);
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
    ChooseTargets();
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

