#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet_generic.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/unit_util.h"
#include "cmd/script/flightgroup.h"
#include "cmd/role_bitmask.h"
using Orders::FireAt;
void FireAt::ReInit (float reaction_time, float aggressivitylevel) {
  static float missileprob = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","MissileProbability",".01"));
  missileprobability = missileprob;  
  gunspeed=float(.0001);
  gunrange=float(.0001);
  delay=0;
  agg = aggressivitylevel;
  rxntime = reaction_time;
  distance=1;
}
FireAt::FireAt (float reaction_time, float aggressivitylevel): CommunicatingAI (WEAPON,STARGET){
  ReInit (reaction_time,aggressivitylevel);
}
FireAt::FireAt (): CommunicatingAI (WEAPON,STARGET) {
  static float reaction = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","ReactionTime",".2"));
  static float aggr = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","Aggressivity","15"));
  ReInit (reaction,aggr);
}
//temporary way of choosing
void FireAt::ChooseTargets (int numtargs, bool force) {
  parent->getAverageGunSpeed (gunspeed,gunrange);  
  Unit * inRangeThreat =NULL;
  Unit * outRangeThreat=NULL;
  Unit * inRangeHate =NULL;
  Unit * outRangeHate=NULL;
  float irt=FLT_MAX;
  float ort=FLT_MAX;
  float irrelation=0;
  float irh=FLT_MAX;
  float orh=FLT_MAX;
  float relation=1;
  float worstrelation=0;
  un_iter subun = parent->getSubUnits();
  Unit * su=NULL;
  double dist;
  UnitCollection::UnitIterator iter (_Universe->activeStarSystem()->getUnitList().createIterator());
  Unit * un=NULL;
  while ((un = iter.current())) {
    float range=UnitUtil::getDistance(parent,un);
    relation = GetEffectiveRelationship (un);
    bool tmp = parent->InRange (un,dist,false,false,true);
    if (tmp && relation<0) {
      if (((relation<worstrelation||(relation==worstrelation&&range<orh)))) {
	worstrelation = relation;
	orh = range;
	outRangeHate=un;
      }

      bool threat = (un->Target()==parent);
      bool aturret=false;
      if (threat) {
	if (range<ort) {
	  ort = range;
	  outRangeThreat = un;
	}
      }
      if (range < gunrange) {
	aturret=true;
	if (threat) {
	  if (range < irt) {
	    irt = range;
	    inRangeThreat = un;
	  }
	}else {
	  if (((relation<irrelation||(relation==irrelation&&range<irh)))) {
	    irh = range;
	    irrelation=relation;
	    inRangeHate = un;
	  }
	}
      }
      su = *subun;
      if (su&&aturret) {
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
  Unit * mytarg = inRangeThreat;
  if (!mytarg) {
    mytarg = inRangeHate;
    if (!mytarg) {
      mytarg = outRangeThreat;
      if (!mytarg) {
	mytarg = outRangeHate;
      }
    }
  }
  parent->Target (mytarg);
  for (;(su=*subun)!=NULL;++subun) {
    su->Target (mytarg);
    su->TargetTurret(mytarg);
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
    if (!targ) {
      return false;
        static int test=0;
        if (test++%1000==1)
            fprintf (stderr,"lost target");
    }
  float angle = parent->cosAngleTo (targ, dist,parent->GetComputerData().itts?gunspeed:FLT_MAX,gunrange);
  missilelock=false;
  targ->Threaten (parent,angle/(dist<.8?.8:dist));
  if (targ==parent->Target()) {
    distance = dist;
  }
  static float firewhen = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","InWeaponRange","1.2"));
  return (dist<agg&&angle>1/agg)||(parent->TrackingGuns(missilelock)&&dist<firewhen&&angle>0);
}

FireAt::~FireAt() {
#ifdef ORDERDEBUG
  fprintf (stderr,"fire%x\n",this);
  fflush (stderr);
#endif

}

void FireAt::FireWeapons(bool shouldfire, bool lockmissile) {
    if (shouldfire&&delay<rxntime) {
        delay+=SIMULATION_ATOM;
        return;
    }else if (!shouldfire) {
        delay=0;
    }
    unsigned int firebitm = ROLES::EVERYTHING_ELSE;
    Unit * un=parent->Target();
    if (un) {
        firebitm = ((1 << parent->combatRole()) |
                    ROLES::FIRE_GUNS|
                    (shouldfire?0:ROLES::FIRE_ONLY_AUTOTRACKERS)|
                    (((float)rand())/
                     ((float)RAND_MAX)<missileprobability)?ROLES::FIRE_MISSILES:0);
    }
    parent->Fire(firebitm);
}

bool FireAt::isJumpablePlanet(Unit * targ) {
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
      istargetjumpableplanet=(!((Planet*)targ)->GetDestinations().empty())&&(parent->GetJumpStatus().drive>=0);
      if (!istargetjumpableplanet) {
	ChooseTarget();
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
      ChooseTarget();
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
	ChooseTarget();
      }
    }
  } else {
    ChooseTarget();
  }
  if (parent->GetNumMounts ()>0) {
    FireWeapons (shouldfire,missilelock);
  }
}
