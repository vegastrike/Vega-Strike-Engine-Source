
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"

using Orders::FireAt;

FireAt::FireAt (float reaction_time, float aggressivitylevel, bool itts): Order (WEAPON|TARGET), itts(itts), rxntime (reaction_time), delay(0), agg (aggressivitylevel), distance(1){
  
  
}
//temporary way of choosing
void FireAt::ChooseTargets (int ) {
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
/* Proper choosing of targets
void FireAt::ChooseTargets (int num) {
  UnitCollection tmp;
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    iter->advance();
  }
  delete iter;
  AttachOrder (&tmp);
}

*/
bool FireAt::ShouldFire(Unit * targ) {
  float dist;
  float angle = parent->cosAngleTo (targ, dist,itts?0.001:FLT_MAX);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<.8*agg&&angle>1/agg);
}


void FireAt::Execute () {
  bool tmp = done;
  Order::Execute();	
  
  done = tmp;
  Unit * targ;
  bool shouldfire=false;
  if (targets) {
    UnitCollection::UnitIterator *iter = targets->createIterator();    
  
    if (!iter->current()) {
      ChooseTargets(2);
      delete iter;
      iter = targets->createIterator();
    }
    while ((targ = iter->current())!=NULL) {
      shouldfire|=ShouldFire(targ);
      iter->advance();
    }
    delete iter;  
  }
  if ((targ = parent->Target())) {
    shouldfire |= ShouldFire (targ);
  }
  if (shouldfire) {
    if (delay>rxntime)
      parent->Fire();
    else
      delay +=SIMULATION_ATOM;
  } else {
    delay =0;
    parent->UnFire();
  }

  
}

