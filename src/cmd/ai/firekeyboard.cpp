
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"
#include "in_joystick.h"


FireKeyboard::FireKeyboard (int whichjoystick, const char *): Order (WEAPON){
  gunspeed = gunrange = .0001;
  BindJoyKey (whichjoystick,0,FireKeyboard::JFireKey);
  BindJoyKey (whichjoystick,1,FireKeyboard::JMissileKey);
  BindJoyKey (whichjoystick,3,FireKeyboard::JTargetKey);

  BindKey(' ',FireKeyboard::FireKey);
  BindKey(13,FireKeyboard::MissileKey);
  BindKey('t',FireKeyboard::TargetKey);
  
}
static KBSTATE firekey=UP;
static KBSTATE targetkey=UP;
static KBSTATE missilekey = UP;
static KBSTATE jfirekey=UP;
static KBSTATE jtargetkey=UP;
static KBSTATE jmissilekey = UP;

void FireKeyboard::FireKey(int, KBSTATE k) {
  if (k==UP&&firekey==RELEASE) {

  } else {
    firekey = k;
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
}
void FireKeyboard::JTargetKey(KBSTATE k, float, float,int i) {
  if (jtargetkey!=PRESS)
    jtargetkey = k;
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
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    if (un==parent->Target()) {
      iter->advance();
      break;
    }
    iter->advance();
  }
  if ((un = iter->current())) {
    parent->Target (un);
  }
  delete iter;
  if (!un) {
    UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
    parent->Target (iter->current());//cycle through for now;
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
  if (targetkey==PRESS||jtargetkey==PRESS) {
    targetkey=DOWN;
    jtargetkey=DOWN;
    ChooseTargets();
  }
}

