
#include "firekeyboard.h"
#include "flybywire.h"
#include "navigation.h"



FireKeyboard::FireKeyboard (): Order (WEAPON){
  BindKey(' ',FireKeyboard::FireKey);
  BindKey('t',FireKeyboard::TargetKey);

}
static KBSTATE firekey=UP;
static KBSTATE targetkey=UP;
void FireKeyboard::FireKey(int, KBSTATE k) {
  firekey = k;
}
void FireKeyboard::TargetKey(int, KBSTATE k) {
  targetkey = k;
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
  float angle = parent->cosAngleTo (targ, dist,itts?0.001:FLT_MAX);
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
  if (firekey==DOWN) 
    parent->Fire();
  else if (firekey==UP) {
    parent->UnFire();
  }
  if (targetkey==PRESS) {
    ChooseTargets();
  }
}

