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


AggressiveAI::AggressiveAI (Unit * target=NULL):FireAt(.2,6,false)  {
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
    parent->EnqueueAI (new Orders::MatchLinearVelocity (Vector (0,0,100),true,false));
  }

}
