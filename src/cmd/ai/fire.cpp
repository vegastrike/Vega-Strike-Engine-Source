
#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/script/flightgroup.h"
using Orders::FireAt;

FireAt::FireAt (float reaction_time, float aggressivitylevel): CommunicatingAI (WEAPON,STARGET),  rxntime (reaction_time), delay(0), agg (aggressivitylevel), distance(1){
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
  Unit * un =NULL;
  float relation=1;
  float range=0;
  float worstrelation=0;
  un_iter subun = parent->getSubUnits();
  Unit * su=NULL;
  double dist;
  while ((un = iter.current())) {
    //how to choose a target?? "if looks particularly juicy... :-) tmp.prepend (un);
    relation = GetEffectiveRelationship (un);
    bool tmp = parent->InRange (un,dist,false,false);
    if (tmp&&((relation<worstrelation||(relation==worstrelation&&dist<range)))) {
      worstrelation = relation;
      range = dist;
      parent->Target (un);
      su = *subun;
      if (su) {
	//	while (su->InRange (un,t,false,false)) {
	  su->Target (un);
	  su->TargetTurret(un);
	  ++subun;
	  //	  if (!(su=*subun))
	  //    break;
	  //}
      }
    }
    iter.advance();
  }
  if (worstrelation>0) {
    parent->Target (NULL);
  }else {
    for (;(su=*subun)!=NULL;++subun) {
      un=parent->Target();
      su->Target (un);
      su->TargetTurret(un);
      if (un) {
	//	fprintf (stderr,"turret %s targetting %s",parent->name.c_str(),un->name.c_str());
      }
    }
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
bool FireAt::ShouldFire(Unit * targ, bool &missilelock) {
  float dist;
  float angle = parent->cosAngleTo (targ, dist,parent->GetComputerData().itts?gunspeed:FLT_MAX,gunrange);
  missilelock=false;
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  return (dist<agg&&angle>1/agg)||(parent->TrackingGuns(missilelock)&&dist<agg&&angle>0);
}

FireAt::~FireAt() {
#ifdef ORDERDEBUG
  fprintf (stderr,"fire%x\n",this);
  fflush (stderr);
#endif

}
#if 0
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
#endif
void FireAt::FireWeapons(bool shouldfire, bool lockmissile) {
  if (lockmissile) {
    parent->Fire (true);
    parent->ToggleWeapon(true);
  }
  if (shouldfire) {
    if ((float(rand())/RAND_MAX)<missileprobability*SIMULATION_ATOM) {
      int locked = parent->LockMissile();
      if (locked==-1) {
	parent->Fire(true);
	parent->ToggleWeapon(true);//change missiles to only fire 1
      }
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

bool FireAt::isJumpablePlanet(Unit * targ) {
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
      istargetjumpableplanet=(!((Planet*)targ)->GetDestinations().empty())&&(parent->GetJumpStatus().drive>=0);
      if (!istargetjumpableplanet) {
	ChooseTargets(1);
      }
    }
    return istargetjumpableplanet;
}

void FireAt::Execute () {
  bool missilelock=false;
  bool tmp = done;
  Order::Execute();	
  if (gunspeed==float(.0001)) {
    parent->getAverageGunSpeed (gunspeed,gunrange);  
  }
  done = tmp;
  Unit * targ;
  if (parent->isUnit()==UNITPTR) {
    static float cont_update_time = XMLSupport::parse_float (vs_config->getVariable ("AI","AI","ContrabandUpdateTime","1"));
    if (rand()<RAND_MAX*SIMULATION_ATOM/cont_update_time) {
      UpdateContrabandSearch();
    }
    static float cont_initiate_time = XMLSupport::parse_float (vs_config->getVariable ("AI","AI","CommInitiateTime","300"));
    if ((float)rand()<((float)RAND_MAX*(SIMULATION_ATOM/cont_initiate_time))) {
      static float contraband_initiate_time = XMLSupport::parse_float (vs_config->getVariable ("AI","AI","ContrabandInitiateTime","3000"));
      unsigned int modulo = ((unsigned int)(contraband_initiate_time/cont_initiate_time));
      if (modulo<1)
	modulo=1;
      if (rand()%modulo) {
	RandomInitiateCommunication(.5,.25);
      }else {
	InitiateContrabandSearch (.98,.02);
      }
    }
  }

  bool shouldfire=false;
  //  if (targets) 
  //    shouldfire |=DealWithMultipleTargets();

  if ((targ = parent->Target())) {
    bool istargetjumpableplanet = isJumpablePlanet (targ);
    if (targ->CloakVisible()>.8&&targ->GetHull()>=0) {
      if (parent->GetNumMounts()>0) {
	if (!istargetjumpableplanet)
	  shouldfire |= ShouldFire (targ,missilelock);
      }
    }else {
      ChooseTargets(1,true);
    }
    static float targetswitchprobability = XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","TargetSwitchProbability",".01"));
    if ((!istargetjumpableplanet)&&(float(rand())/RAND_MAX)<targetswitchprobability*SIMULATION_ATOM) {
      bool switcht=true;
      Flightgroup * fg = parent->getFlightgroup();;
      if (fg) {
	if (!fg->directive.empty()) {
	  if ((*fg->directive.begin())==toupper (*fg->directive.begin())) {
	    switcht=false;
	  }
	}
      }
      if (switcht) {
	ChooseTargets(1,true);
      }
    }
  } else {
    ChooseTargets(1,false);
  }
  if (parent->GetNumMounts ()>0) {
    FireWeapons (shouldfire,missilelock);
  }
}

