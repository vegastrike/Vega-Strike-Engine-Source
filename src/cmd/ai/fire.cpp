
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"

using Orders::FireAt;

FireAt::FireAt (float reaction_time, float aggressivitylevel): Order (WEAPON|TARGET),  rxntime (reaction_time), delay(0), agg (aggressivitylevel), distance(1){
  gunspeed=float(.0001);
  gunrange=float(.0001);
  missileprobability=.001;
  
}
//temporary way of choosing
void FireAt::ChooseTargets (int ) {
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList()->createIterator();
  Unit * un ;
  float relation=1;
  float worstrelation=1;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    relation = _Universe->GetRelation (parent->faction, un->faction);
    if (relation<worstrelation) {
      worstrelation = relation;
      parent->Target (un);
    }
    iter->advance();
  }
  delete iter;
  if (worstrelation>0) {
    parent->Target (NULL);
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
  float angle = parent->cosAngleTo (targ, dist,parent->GetComputerData().itts?gunspeed:FLT_MAX,gunrange);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<.8*agg&&angle>1/agg);
}


void FireAt::Execute () {
  bool tmp = done;
  Order::Execute();	
  if (gunspeed==float(.0001)) {
    parent->getAverageGunSpeed (gunspeed,gunrange);  
  }
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
  } else {

  }
  if ((targ = parent->Target())) {
    shouldfire |= ShouldFire (targ);
    if (targ->GetHullData()<0) {
      ChooseTargets(1);
    }
  } else {
    ChooseTargets(1);
  }
  if (shouldfire) {
    if ((float(rand())/RAND_MAX)<missileprobability*SIMULATION_ATOM) {
      parent->Fire(true);
      parent->ToggleWeapon(true);//change missiles to only fire 1
    }
    if (delay>rxntime) {
      parent->Fire(false);


    } else {
      delay +=SIMULATION_ATOM;
    }
  } else {
    delay =0;
    parent->UnFire();
  }

  
}

