
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet.h"
#include "config_xml.h"
#include "vs_globals.h"
using Orders::FireAt;

FireAt::FireAt (float reaction_time, float aggressivitylevel): CommunicatingAI (WEAPON|TARGET),  rxntime (reaction_time), delay(0), agg (aggressivitylevel), distance(1){
  gunspeed=float(.0001);
  gunrange=float(.0001);
  static float missileprob = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","MissileProbability",".01"));
  missileprobability = missileprob;
  
}
//temporary way of choosing
void FireAt::ChooseTargets (int numtargs, bool force) {
  static float TargetSwitchProbability =XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","TargetSwitchReactionTime",".8"));
  if (!force) {
    if ((((float)rand())/RAND_MAX)<TargetSwitchProbability) {
      return;
    }
  }
  UnitCollection::UnitIterator iter (_Universe->activeStarSystem()->getUnitList().createIterator());
  Unit * un ;
  float relation=1;
  float range=0;
  float worstrelation=0;
  while ((un = iter.current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    relation = _Universe->GetRelation (parent->faction, un->faction);
    Vector t;
    bool tmp = parent->InRange (un,t);
    if (tmp&&((relation<worstrelation||(relation==worstrelation&&t.Dot(t)<range)))) {
      worstrelation = relation;
      range = t.Dot(t);
      parent->Target (un);
    }
    iter.advance();
  }
  if (worstrelation>0) {
    parent->Target (NULL);
  }
}
/* Proper choosing of targets
void FireAt::ChooseTargets (int num) {
  UnitCollection tmp;
  UnitCollection::UnitIterator *iter = _Universe->activeStarSystem()->getUnitList().createIterator();
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

FireAt::~FireAt() {
#ifdef ORDERDEBUG
  fprintf (stderr,"fire%x\n",this);
  fflush (stderr);
#endif

}
bool FireAt::DealWithMultipleTargets () {
  UnitCollection::UnitIterator iter (targets->createIterator());    
  if (!iter.current()) {
    ChooseTargets(2);
    iter = targets->createIterator();
  }
  Unit * targ;
  while ((targ = iter.current())!=NULL) {
    if (ShouldFire(targ))
      return true;
    iter.advance();
  }
  return false;
}
void FireAt::FireWeapons(bool shouldfire) {
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
void FireAt::Execute () {
  bool tmp = done;
  Order::Execute();	
  if (gunspeed==float(.0001)) {
    parent->getAverageGunSpeed (gunspeed,gunrange);  
  }
  done = tmp;
  Unit * targ;
  bool shouldfire=false;
  if (targets) 
    shouldfire |=DealWithMultipleTargets();

  if ((targ = parent->Target())) {
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
      istargetjumpableplanet=(!((Planet*)targ)->GetDestinations().empty())&&(parent->GetJumpStatus().drive>=0);
      if (!istargetjumpableplanet) {
	ChooseTargets(1);
      }
    }
    if (targ->CloakVisible()>.8&&targ->GetHull()>=0) {
      if (!istargetjumpableplanet)
	shouldfire |= ShouldFire (targ);
    }else {
      ChooseTargets(1,true);
    }
    static float targetswitchprobability = XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","TargetSwitchProbability",".01"));
    if ((!istargetjumpableplanet)&&(float(rand())/RAND_MAX)<targetswitchprobability*SIMULATION_ATOM) {
      ChooseTargets(1,true);
    }
  } else {
    ChooseTargets(1,false);
  }
  FireWeapons (shouldfire);
}

