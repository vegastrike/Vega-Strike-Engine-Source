#include "fire.h"
#include "flybywire.h"
#include "navigation.h"
#include "cmd/planet_generic.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/unit_util.h"
#include "cmd/script/flightgroup.h"
#include "cmd/role_bitmask.h"
#include "cmd/ai/communication.h"
#include <algorithm>
static bool NoDockWithClear() {
	static bool nodockwithclear = XMLSupport::parse_bool (vs_config->getVariable ("physics","dock_with_clear_planets","true"));
	return nodockwithclear;
}
Unit * getAtmospheric (Unit * targ) {
  if (targ) {
    Unit * un;
    for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (un=*i)!=NULL;
	 ++i) {
      if (un->isUnit()==PLANETPTR) {
	if ((targ->Position()-un->Position()).Magnitude()<targ->rSize()*.5) {
	  if (!(((Planet *)un)->isAtmospheric())) {
	    return un;
	  }
	}
      }
    }
  }
  return NULL;
  
}

bool RequestClearence(Unit *parent, Unit *targ, unsigned char sex) {
	if (!targ->DockingPortLocations().size())
		return false;
	if (targ->isUnit()==PLANETPTR) {
		if (((Planet * )targ)->isAtmospheric()&&NoDockWithClear()) {
			targ = getAtmospheric (targ);
			if (!targ) {
				return false;
			}
			parent->Target(targ);
		}
	}
	CommunicationMessage c(parent,targ,NULL,sex);
	c.SetCurrentState(c.fsm->GetRequestLandNode(),NULL,sex);
	targ->getAIState()->Communicate (c);
	return true;
}

using Orders::FireAt;
bool FireAt::PursueTarget (Unit * un, bool leader) {
  if (leader)
    return true;
  if (un==parent->Target())
    return rand()<.9*RAND_MAX;
  if (parent->getRelation(un)<0)
    return rand()<.2*RAND_MAX;
  return false;
}
void FireAt::ReInit (float reaction_time, float aggressivitylevel) {
  static float missileprob = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","MissileProbability",".01"));
  missileprobability = missileprob;  
  gunspeed=float(.0001);
  gunrange=float(.0001);
  missilerange=float(.0001);
  delay=0;
  agg = aggressivitylevel;
  rxntime = reaction_time;
  distance=1;
  lastchangedtarg=-100000;
  had_target=false;
}
FireAt::FireAt (float reaction_time, float aggressivitylevel): CommunicatingAI (WEAPON,STARGET){
  ReInit (reaction_time,aggressivitylevel);
}
FireAt::FireAt (): CommunicatingAI (WEAPON,STARGET) {
  static float reaction = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","ReactionTime",".2"));
  static float aggr = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","Aggressivity","15"));
  ReInit (reaction,aggr);
}
float Priority (Unit * me, Unit * targ, float gunrange,float rangetotarget, float relationship) {
  if(relationship>=0)
    return -1;
  if (targ->GetHull()<0)
    return -1;
  char rolepriority = (31-ROLES::getPriority (me->combatRole())[targ->combatRole()]);//number btw 0 and 31 higher better
  if(rolepriority<=0)
    return -1;
  if(rangetotarget <1 && rangetotarget >-1000){
	rangetotarget=1;
  } else {
	  rangetotarget=fabs(rangetotarget);
  }
  if(rangetotarget<.5*gunrange)
    rangetotarget=.5*gunrange;
  if(gunrange <=0){
	gunrange= 50000;
  }//probably a mountless capship. 50000 is chosen arbitrarily
  float inertial_priority=0;
  {
    static float mass_inertial_priority_cutoff =XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","MassInertialPriorityCutoff","5000"));
    if (me->GetMass()>mass_inertial_priority_cutoff) {
      static float mass_inertial_priority_scale =XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","MassInertialPriorityScale",".0000001"));
      Vector normv (me->GetVelocity());
      float Speed = me->GetVelocity().Magnitude();
      normv*=1/Speed;
      Vector ourToThem = targ->Position()-me->Position();
      ourToThem.Normalize();
      inertial_priority = mass_inertial_priority_scale*(.5 + .5 * (normv.Dot(ourToThem)))*me->GetMass()*Speed;
    }
    
  }
  static float threat_weight = XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","ThreatWeight",".5"));
  float threat_priority = (me->Threat()==targ)?threat_weight:0;
  threat_priority+= (targ->Target()==me)?threat_weight:0;
  float role_priority01 = ((float)rolepriority)/31.;
  float range_priority01 =.5*gunrange/rangetotarget;//number between 0 and 1 for most ships 1 is best
  return range_priority01*role_priority01+inertial_priority+threat_priority;
}
void FireAt::SignalChosenTarget () {
}
//temporary way of choosing
struct TargetAndRange {
  Unit * t;
  float range;
  float relation;
  TargetAndRange (Unit * tt, float r,float rel) {
    t =tt;range=r;this->relation = rel;
  }
};
struct RangeSortedTurrets {
  Unit * tur;
  float gunrange;
  RangeSortedTurrets (Unit * t, float r) {tur = t; gunrange = r;}
  bool operator < (const RangeSortedTurrets &o) const{
    return gunrange<o.gunrange;
  }
};
struct TurretBin{
  float maxrange;
  vector <RangeSortedTurrets> turret;
  vector <TargetAndRange> listOfTargets[2];//we have the over (and eq) 16 crowd and the under 16  
  TurretBin () {
    maxrange =0;
  }
  bool operator < (const TurretBin & o) const{
    return (maxrange>o.maxrange);
  }
  void AssignTargets(const TargetAndRange &finalChoice) {
    //go through modularly assigning as you go;
    int count=0;
    const unsigned int lotsize[2]={listOfTargets[0].size(),listOfTargets[1].size()};
    for (vector<RangeSortedTurrets>::iterator uniter=turret.begin();uniter!=turret.end();++uniter) {
      bool foundfinal=false;
      uniter->tur->Target(NULL);
      uniter->tur->TargetTurret(NULL);
      if (finalChoice.t) {
		/*  FIX ME FIXME missiles not accounted for yet
		  if(uniter->gunrange<0){
		    uniter->gunrange=FLT_MAX; // IS MISSILE TURRET (we hope)
		  }
		  */
	    if (finalChoice.range<uniter->gunrange&&ROLES::getPriority (uniter->tur->combatRole())[finalChoice.t->combatRole()]<31) {
	      uniter->tur->Target(finalChoice.t);
	      uniter->tur->TargetTurret(finalChoice.t);
	      foundfinal=true;
		}
      }
      if (!foundfinal) {
	for (char f=0;f<2&&!foundfinal;f++) {
	for (unsigned int i=0;i<lotsize[f];i++) {
	  const int index =(count+i)%lotsize[f];
	  if (listOfTargets[f][index].range<uniter->gunrange) {
	    uniter->tur->Target(listOfTargets[f][index].t);
	    uniter->tur->TargetTurret(listOfTargets[f][index].t);
	    count++;
	    foundfinal=true;
	    break;
	  }
	}
	}
      }
    }
  }
};
void FireAt::getAverageGunSpeed (float & speed, float & range, float &mmrange) const{
  speed =gunspeed;
  range= gunrange;
  mmrange=missilerange;
}
void FireAt::ChooseTargets (int numtargs, bool force) {
  static float mintimetoswitch = XMLSupport::parse_float(vs_config->getVariable ("AI","Targetting","MinTimeToSwitchTargets","3"));
  if (lastchangedtarg+mintimetoswitch>0) 
    return;//don't switch if switching too soon
  Unit * curtarg=NULL;
  if ((curtarg=parent->Target())) 
    if (isJumpablePlanet (curtarg))
      return;
  Flightgroup * fg = parent->getFlightgroup();;
  if (fg) {
    if (!fg->directive.empty()) {
      if ((*fg->directive.begin())==toupper (*fg->directive.begin())) {
	return;//not allowed to switch targets
      }
    }
  }
  lastchangedtarg=0;
  parent->getAverageGunSpeed (gunspeed,gunrange,missilerange);  

  UnitCollection::UnitIterator iter (_Universe->activeStarSystem()->getUnitList().createIterator());
  Unit * un=NULL;
  vector <TurretBin> tbin;;
  
  float priority=0;
  Unit * mytarg=NULL;
  Unit * su=NULL;
  un_iter subun = parent->getSubUnits();
  for (;(su = *subun)!=NULL;++subun) {
    unsigned int bnum=0;
    for (;bnum<tbin.size();bnum++) {
      if (su->combatRole()==tbin[bnum].turret[0].tur->combatRole())
	break;
    }
    if (bnum>=tbin.size()) {
      tbin.push_back (TurretBin());
    }
    float gspeed, grange, mrange;
    grange=FLT_MAX;
	if (su->getAIState()) 
		su->getAIState()->getAverageGunSpeed (gspeed,grange,mrange);
    if (tbin [bnum].maxrange<grange) {
      tbin [bnum].maxrange=grange;
    }
    tbin[bnum].turret.push_back (RangeSortedTurrets (su,grange));
  }
  std::sort (tbin.begin(),tbin.end());
  while ((un = iter.current())) {
    if (un->CloakVisible()>.8) {
      float rangetotarget = UnitUtil::getDistance (parent,un);
      float relationship = GetEffectiveRelationship (un);
      float tmp=Priority (parent,un, gunrange,rangetotarget, relationship);
      if (tmp>priority) {
	mytarg = un;
      }
      for (vector <TurretBin>::iterator k=tbin.begin();k!=tbin.end();++k) {
	if (rangetotarget>k->maxrange) {
	  break;
	}
	const char tprior=ROLES::getPriority (k->turret[0].tur->combatRole())[un->combatRole()];
	if (relationship<0) {
	  if (tprior<16){
	    k->listOfTargets[0].push_back (TargetAndRange (un,rangetotarget,relationship));
	  }else if (tprior<31){
	    k->listOfTargets[1].push_back (TargetAndRange (un,rangetotarget,relationship));
	  }
	}
      }
    }
    iter.advance();
  }
  float efrel = 0;
  float mytargrange = FLT_MAX;
  if (mytarg) {
    efrel=GetEffectiveRelationship (mytarg);
    mytargrange = UnitUtil::getDistance (parent,mytarg);
  }
  TargetAndRange my_target (mytarg,mytargrange,efrel);
  for (vector <TurretBin>::iterator k =  tbin.begin();k!=tbin.end();++k) {
    k->AssignTargets(my_target);
  } 
  SignalChosenTarget();
  parent->Target (mytarg);
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
unsigned int FireBitmask (Unit * parent,bool shouldfire, float missileprobability) {
   unsigned int firebitm = ROLES::EVERYTHING_ELSE;
    Unit * un=parent->Target();
    if (un) {
      firebitm = (1 << parent->combatRole());
      firebitm |= ROLES::FIRE_GUNS;
      if (!shouldfire) 
	firebitm |= ROLES::FIRE_ONLY_AUTOTRACKERS;
      if (((float)rand())/((float)RAND_MAX)<missileprobability) 
	firebitm |=ROLES::FIRE_MISSILES;
    }
    return firebitm;
}
void FireAt::FireWeapons(bool shouldfire, bool lockmissile) {
    if (shouldfire&&delay<rxntime) {
        delay+=SIMULATION_ATOM;
        return;
    }else if (!shouldfire) {
        delay=0;
    }
     parent->Fire(FireBitmask(parent,shouldfire,  missileprobability),true);
}

bool FireAt::isJumpablePlanet(Unit * targ) {
    bool istargetjumpableplanet = targ->isUnit()==PLANETPTR;
    if (istargetjumpableplanet) {
      istargetjumpableplanet=(!((Planet*)targ)->GetDestinations().empty())&&(parent->GetJumpStatus().drive>=0);
      if (!istargetjumpableplanet) {
	//	ChooseTarget(); //WTF this will cause endless loopdiloop
      }
    }
    return istargetjumpableplanet;
}
using std::string;
void FireAt::PossiblySwitchTarget(bool unused) {
  static float targetswitchprobability = XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","TargetSwitchProbability",".01"));
  static float targettime = XMLSupport::parse_float (vs_config->getVariable ("AI","Targetting","TimeUntilSwitch","20"));
  if (lastchangedtarg+targettime<0) {
	  bool ct= true;
	  Flightgroup * fg;	  
	  if ((fg = parent->getFlightgroup())) {
		  if (fg->directive.find(".")!=string::npos) {
			  ct=(parent->Target()==NULL);
		  }
	  }
	  if (ct)
		  ChooseTarget();
  }
}
void FireAt::Execute () {
  lastchangedtarg-=SIMULATION_ATOM;
  
  bool missilelock=false;
  bool tmp = done;
  Order::Execute();	
  if (gunspeed==float(.0001)) {
    ChooseTarget();//starting condition
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
  bool istargetjumpableplanet=false;
  if ((targ = parent->Target())) {
    istargetjumpableplanet = isJumpablePlanet (targ);
    if (targ->CloakVisible()>.8&&targ->GetHull()>=0) {
      had_target=true;
      if (parent->GetNumMounts()>0) {
	if (!istargetjumpableplanet)
	  shouldfire |= ShouldFire (targ,missilelock);
      }
    }else {
      if (had_target) {
	had_target=false;
	lastchangedtarg=-100000;
      }
      ChooseTarget();
    }
  }else {
    if (had_target) {
      had_target=false;
      lastchangedtarg=-100000;
    }
  }
  PossiblySwitchTarget(istargetjumpableplanet);

  if (parent->GetNumMounts ()>0) {
    FireWeapons (shouldfire,missilelock);
  }
}
