#include "cmd_target_ai.h"
#include "cmd_flybywire.h"
#include "vegastrike.h"
#include "cmd_navigation_orders.h"
using Orders::FireAt;
using Orders::AggressiveAI;

FireAt::FireAt (float reaction_time, float aggressivitylevel): Order (WEAPON|TARGET), rxntime (reaction_time), delay(0), agg (aggressivitylevel){
  
  
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
  float angle = parent->cosAngleFromMountTo (targ, dist);
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
  while ((targ = iter->current())!=NULL) {
    shouldfire|=ShouldFire(targ);
    iter->advance();
  }
  if (shouldfire) {
    parent->Fire();
  } else {
    parent->UnFire();
  }
  delete iter;  
  
}


AggressiveAI::AggressiveAI (Unit * target=NULL):FireAt(.2,6)  {
  if (target !=NULL) {
    UnitCollection tmp;
    tmp.prepend (target);
    AttachOrder (&tmp);
  }
  facingtarg= false;
}

void AggressiveAI::Execute () {  
  FireAt::Execute();
  if (!facingtarg) {
    Order * tmp;
    parent->EnqueueAI (tmp = new Orders::FaceTarget (false));
    //tmp->SetParent(parent);
    parent->EnqueueAI (tmp = new Orders::MatchLinearVelocity (Vector (0,0,100),true,false));
    //tmp->SetParent (parent);
    facingtarg=true;
  }

}
