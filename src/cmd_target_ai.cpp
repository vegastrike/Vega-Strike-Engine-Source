#include "physics.h"
#include "cmd_target_ai.h"
#include "cmd_flybywire.h"
#include "vegastrike.h"
#include "cmd_navigation_orders.h"
using Orders::FireAt;
using Orders::AggressiveAI;

FireAt::FireAt (float reaction_time, float aggressivitylevel, bool itts): Order (WEAPON|TARGET), itts(itts), rxntime (reaction_time), delay(0), agg (aggressivitylevel){
  
  
}

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
bool FireAt::ShouldFire(Unit * targ) {
  float dist;
  float angle = parent->cosAngleTo (targ, dist,itts?0.001:FLT_MAX);
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  
  return (dist<.8*agg&&angle>1/agg);
}


void FireAt::Execute () {
  bool tmp = done;
  Order::Execute();	
  
  done = tmp;
  Unit * targ;
  if (!targets) {
    if ((targ = parent->Target())) {
      UnitCollection tmp;
      tmp.prepend (targ);
      AttachOrder (&tmp);
    }
  }
  UnitCollection::UnitIterator *iter = targets->createIterator();    
  bool shouldfire=false;
  if (!iter->current()) {
    ChooseTargets(2);
    delete iter;
    iter = targets->createIterator();
  }
  while ((targ = iter->current())!=NULL) {
    shouldfire|=ShouldFire(targ);
    iter->advance();
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
  delete iter;  
  
}

const EnumMap::Pair element_names[] = {
  EnumMap::Pair ("UNKNOWN", UNKNOWN),
  EnumMap::Pair ("Distance", AggressiveAI::DISTANCE),
  EnumMap::Pair ("Threat", AggressiveAI::THREAT),
  EnumMap::Pair ("FShield", AggressiveAI::FSHIELD),
  EnumMap::Pair ("LShield",AggressiveAI:: LSHIELD),
  EnumMap::Pair ("RShield", AggressiveAI::RSHIELD),
  EnumMap::Pair ("BShield", AggressiveAI::BSHIELD),
  EnumMap::Pair ("FArmor", AggressiveAI::FARMOR),
  EnumMap::Pair ("BArmor", AggressiveAI::BARMOR),
  EnumMap::Pair ("LArmor", AggressiveAI::LARMOR),
  EnumMap::Pair ("RArmor", AggressiveAI::RARMOR),
  EnumMap::Pair ("Hull", AggressiveAI::HULL),
  EnumMap::Pair ("Facing", AggressiveAI::FACING),
  EnumMap::Pair ("Movement", AggressiveAI::MOVEMENT)
};
const EnumMap AggressiveAIel_map(element_names, 14);

AggressiveAI::AggressiveAI (Unit * target=NULL):FireAt(.2,6,false),threatened(false)  {
  if (target !=NULL) {
    UnitCollection tmp;
    tmp.prepend (target);
    AttachOrder (&tmp);
  }
}

void AggressiveAI::Execute () {  
  FireAt::Execute();
  if (parent->getAIState()->queryType (FACING)==NULL) {
    parent->EnqueueAI (new Orders::FaceTarget (false));
  }
  if (parent->getAIState()->queryType (MOVEMENT)==NULL) {
    parent->EnqueueAI (new Orders::MatchLinearVelocity (Vector (0,0,420),true,false));
  }

}
