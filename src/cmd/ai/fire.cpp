
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet.h"
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
  float range=0;
  float worstrelation=0;
  while ((un = iter->current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    relation = _Universe->GetRelation (parent->faction, un->faction);
    Vector t;
    bool tmp = parent->InRange (un,t);
    if (tmp&&((relation<worstrelation||(relation==worstrelation&&t.Dot(t)<range)))) {
      worstrelation = relation;
      range = t.Dot(t);
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
  return (dist<agg&&angle>1/agg);
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
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
      istargetjumpableplanet=((Planet*)targ)->GetDestinations().size()&&parent->GetJumpStatus().drive>=0;
    }
    
    if (targ->CloakVisible()>.8) {
      if (!istargetjumpableplanet)
	shouldfire |= ShouldFire (targ);
      if (targ->GetHull()<0) {
	ChooseTargets(1);
      }
    }else {
      ChooseTargets(1);
    }
    if ((!istargetjumpableplanet)&&(float(rand())/RAND_MAX)<.5*missileprobability*SIMULATION_ATOM) {
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

