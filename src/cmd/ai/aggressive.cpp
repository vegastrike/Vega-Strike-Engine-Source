#include "aggressive.h"
#include "event_xml.h"
#include "script.h"
#include <list>
#include <vector>
#include "vs_globals.h"
#include "config_xml.h"
#include "xml_support.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "cmd/script/flightgroup.h"
#include "flybywire.h"
#include "hard_coded_scripts.h"
#include "cmd/script/mission.h"
#include "gfx/cockpit_generic.h"
#include "lin_time.h"
#include "faction_generic.h"
#include "cmd/role_bitmask.h"
using namespace Orders;
using std::map;
const EnumMap::Pair element_names[] = {
  EnumMap::Pair ("AggressiveAI" , AggressiveAI::AGGAI),
  EnumMap::Pair ("UNKNOWN", AggressiveAI::UNKNOWN),
  EnumMap::Pair ("Distance", AggressiveAI::DISTANCE),
  EnumMap::Pair ("MeterDistance", AggressiveAI::METERDISTANCE),
  EnumMap::Pair ("Threat", AggressiveAI::THREAT),
  EnumMap::Pair ("FShield", AggressiveAI::FSHIELD),
  EnumMap::Pair ("LShield",AggressiveAI:: LSHIELD),
  EnumMap::Pair ("RShield", AggressiveAI::RSHIELD),
  EnumMap::Pair ("BShield", AggressiveAI::BSHIELD),
  EnumMap::Pair ("Hull", AggressiveAI::HULL),
  EnumMap::Pair ("Facing", AggressiveAI::FACING),
  EnumMap::Pair ("Movement", AggressiveAI::MOVEMENT),
  EnumMap::Pair ("Rand", AggressiveAI::RANDOMIZ)
};
const EnumMap AggressiveAIel_map(element_names, 12);
using std::pair;
std::map<string,AIEvents::ElemAttrMap *> logic;
std::map<string,AIEvents::ElemAttrMap *> interrupts;
AIEvents::ElemAttrMap* getLogicOrInterrupt (string name,int faction, std::map<string,AIEvents::ElemAttrMap *> &mymap, bool inter) {
  map<string,AIEvents::ElemAttrMap *>::iterator i = mymap.find (name+string("%")+tostring(faction));
  if (i==mymap.end()) {
    AIEvents::ElemAttrMap * attr = new AIEvents::ElemAttrMap(AggressiveAIel_map);
    string filename (name+(inter?string(".int.xml"):string(".agg.xml")));
    AIEvents::LoadAI (filename.c_str(),*attr,FactionUtil::GetFaction(faction));
    mymap.insert (pair<string,AIEvents::ElemAttrMap *> (name+string("%")+tostring(faction),attr));
    return attr;
  }
  return i->second;
}
AIEvents::ElemAttrMap* getProperLogicOrInterruptScript (string name,int faction, bool interrupt) {
  return getLogicOrInterrupt (name,faction,interrupt?interrupts:logic,interrupt);
}
AIEvents::ElemAttrMap * getProperScript(Unit * me, Unit * targ, bool interrupt) {
  if (!me||!targ) {
    int fac=0;
    if (me)
      fac=me->faction;
    return getProperLogicOrInterruptScript("default",fac,interrupt);
  }
  return getProperLogicOrInterruptScript (ROLES::getRoleEvents(me->combatRole(),targ->combatRole()),me->faction,interrupt);
}

void DoSpeech (Unit * un, const string &speech) {
  string myname ("[Static]");
  if (un) {
    myname= un->name;
  }
  mission->msgcenter->add (myname,"all",speech);
}
void LeadMe (Unit * un, string directive, string speech) { 
  if (un!=NULL) {
    for (int i=0;i<_Universe->numPlayers();i++) {
      Unit * pun =_Universe->AccessCockpit(i)->GetParent();
      if (pun) {
	if (pun->getFlightgroup()==un->getFlightgroup()){
	  DoSpeech (un, speech);	
	}
      }
    }
    Flightgroup * fg = un->getFlightgroup();
    if (fg) {
      if (fg->leader.GetUnit()!=un) {
		  if ((!_Universe->isPlayerStarship(fg->leader.GetUnit()))||_Universe->isPlayerStarship(un)) {
			fg->leader.SetUnit (un);
		  }
      }
      fg->directive = directive;
    }
  }
}

static float aggressivity=2.01;
AggressiveAI::AggressiveAI (const char * filename, const char * interruptname, Unit * target):FireAt(), logic (getProperScript(NULL,NULL,false)), interrupts(getProperScript(NULL,NULL,true)) {
  curinter=INTNORMAL;
  last_time_insys=true;
  obedient = true;
  if (aggressivity==2.01) {
    float defagg = XMLSupport::parse_float (vs_config->getVariable ("unit","aggressivity","2"));
    aggressivity = defagg;
  }
  if (target !=NULL) {
    AttachOrder (target);
  }
  last_directive = filename+string("|")+interruptname;
  //  AIEvents::LoadAI (filename,logic,"neutral");
  //  AIEvents::LoadAI (interruptname,interrupt,"neutral");
}
void AggressiveAI::SetParent (Unit * parent1) {
  FireAt::SetParent(parent1);
  int which = last_directive.find("|");
  string filename (string("default.agg.xml"));
  string interruptname (string("default.int.xml"));
  if (which!=string::npos) {
    filename = last_directive.substr (0,which);
    interruptname = last_directive.substr(which+1);
  }
  last_directive="b";//prevent escort race condition
}
void AggressiveAI::SignalChosenTarget () {
  if (parent) {
    logic =getProperScript(parent,parent->Target(),false);
    interrupts=getProperScript(parent,parent->Target(),true);
  }
  FireAt::SignalChosenTarget();
}

bool AggressiveAI::ExecuteLogicItem (const AIEvents::AIEvresult &item) {
  
  if (item.script.length()!=0) {
    Order * tmp = new AIScript (item.script.c_str());	
    //    parent->EnqueueAI (tmp);
    EnqueueOrder (tmp);
    return true;
  }else {
    return false;
  }
}


bool AggressiveAI::ProcessLogicItem (const AIEvents::AIEvresult &item) {
  float value;

  static float game_speed = XMLSupport::parse_float (vs_config->getVariable ("physics","game_speed","1"));
  static float game_accel = XMLSupport::parse_float (vs_config->getVariable ("physics","game_accel","1"));
  switch (abs(item.type)) {
  case DISTANCE:
    value = distance;
    break;
  case METERDISTANCE:
    {
      Unit * targ = parent->Target();
      if (targ) {
	value = (parent->Position()-targ->Position()).Magnitude()-parent->rSize()-targ->rSize();
      }else {
	value = 10000; 
      }
      value/=game_speed*game_accel;
    }
    break;
  case THREAT:
    value = parent->GetComputerData().threatlevel;
    break;
  case FSHIELD:
    value = parent->FShieldData();
    break;
  case BSHIELD:
    value = parent->BShieldData();
    break;
  case HULL:
    value = parent->GetHull();
    break;
  case LSHIELD:
    value = parent->LShieldData();
    break;
  case RSHIELD:
    value = parent->RShieldData();
    break;
  case FACING:
    //    return parent->getAIState()->queryType (Order::FACING)==NULL;
    return queryType (Order::FACING)==NULL;
  case MOVEMENT:
    //    return parent->getAIState()->queryType (Order::MOVEMENT)==NULL;
    return queryType (Order::MOVEMENT)==NULL;
  case RANDOMIZ:
    value= ((float)rand())/RAND_MAX;
  default:
    return false;
  }
  return item.Eval(value);
}

bool AggressiveAI::ProcessLogic (AIEvents::ElemAttrMap & logi, bool inter) {
  //go through the logic. 
  bool retval=false;
  //  Unit * tmp = parent->Target();
  //  distance = tmp? (tmp->Position()-parent->Position()).Magnitude()-parent->rSize()-tmp->rSize() : FLT_MAX;
  std::vector <std::list <AIEvents::AIEvresult> >::iterator i = logi.result.begin();
  for (;i!=logi.result.end();i++) {
    std::list <AIEvents::AIEvresult>::iterator j;
    bool trueit=true;
    for (j= i->begin();j!=i->end();j++) {
      if (!ProcessLogicItem(*j)) {
	trueit=false;
	break;
      }
    }
    if (trueit&&j==i->end()) {
      //do it
      if (inter) {

	//parent->getAIState()->eraseType (Order::FACING);
	//parent->getAIState()->eraseType (Order::MOVEMENT);
	eraseType (Order::FACING);
	eraseType (Order::MOVEMENT);


      }
      j = i->begin();
      while (j!=i->end()) {
	if (ExecuteLogicItem (*j)) {
	  AIEvents::AIEvresult tmp = *j;
	  i->erase(j);
	  retval=true;
	  i->push_back (tmp);
	  break; 
	}else {
	  j++;
	}
      }

    }
  }
  return retval;
}

Unit * GetThreat (Unit * parent, Unit * leader) {
  Unit * th=NULL;
  Unit * un=NULL;
  bool targetted=false;
  float mindist= FLT_MAX;
	  for (un_iter ui = _Universe->activeStarSystem()->getUnitList().createIterator();
	       (un = *ui);
	       ++ui) {
	    if (parent->getRelation (un)<0) {
	      float d = (un->Position()-leader->Position()).Magnitude();
	      bool thistargetted = (un->Target()==leader);
	      if (!th||(thistargetted&&!targetted)||((thistargetted||(!targetted))&&d<mindist)) {
		th = un;
		targetted=thistargetted;
		mindist = d;
	      }
	    }
	  }
	  return th;
}
bool AggressiveAI::ProcessCurrentFgDirective(Flightgroup * fg) {
  bool retval=false;
  if (fg !=NULL) {
    Unit * leader = fg->leader.GetUnit();
    if (last_directive.empty()) {
      last_directive = fg->directive;
    }
    if (fg->directive!=last_directive) {
      if (float(rand())/RAND_MAX<(obedient?(1-logic->obedience):logic->obedience)) {
	obedient = !obedient;
      }
      if (obedient) {
	eraseType (Order::FACING);
	eraseType (Order::MOVEMENT);
	Unit * targ = parent->Target();
	if (targ) {
	  if (!isJumpablePlanet (targ)) {
	    parent->Target(NULL);
	  }
	}
      }else {
	    CommunicationMessage c(parent,leader,NULL,0);
	    c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	    leader->getAIState()->Communicate(c);
      }
    }
    if (obedient) {
      if (fg->directive==string("a")||fg->directive==string("A")) {
	Unit * targ = fg->leader.GetUnit();
	targ = targ!=NULL?targ->Target():NULL;
	if (targ) {
	  if (targ->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    CommunicationMessage c(parent,leader,NULL,0);
	    if (parent->InRange (targ,true,false)) {
	      parent->Target (targ);
	      parent->TargetTurret(targ);
	      c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	    }else {
	      c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	    }
	    if (fg->directive!=last_directive) {
	      leader->getAIState()->Communicate(c);
	    }
	  }
	}
      }else if (fg->directive==string("f")||fg->directive==string("F")) {
	if (leader!=NULL) {
	  if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    retval=true;
	    if (fg->directive!=last_directive||(!last_time_insys)) {
	      last_time_insys=true;
	      CommunicationMessage c(parent,leader,NULL,0);
	      c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	    //}else {
	      //	  c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	      //	}
	      leader->getAIState()->Communicate(c);
	      float left= parent->getFgSubnumber()%2?1:-1;
	      static float esc_percent= XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"EscortDistance",
											"4.0"));
	      static float turn_leader= XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"TurnLeaderDist",
											"5.0"));
	      
	      double dist=esc_percent*(1+parent->getFgSubnumber()/2)*left*(parent->rSize()+leader->rSize());
	      Order * ord = new Orders::FormUp(QVector(dist,0,-fabs(dist)));
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
	      ord = new Orders::FaceDirection(dist*turn_leader);
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
	    }
	  } else {
	    last_time_insys=false;
	  }
	  for (unsigned int i=0;i<suborders.size();i++) {
	    suborders[i]->AttachSelfOrder (leader);
	  }
	}
      }else if (fg->directive==string("h")||fg->directive==string("H")) {
	//	fprintf (stderr,"he wnats to help out");
	if (fg->directive!=last_directive&&leader) {
	  if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    //fprintf (stderr,"%s he wnats to help out and hasn't died\n", parent->name.c_str());
	    Unit * th=NULL;
	    if ((th=leader->Threat())) {
	      //fprintf (stderr,"he wnats to help out and he has a threat\n");

	      CommunicationMessage c(parent,leader,NULL,0);
	      if (parent->InRange(th,true,false)) {
		parent->Target(th);
		parent->TargetTurret(th);
		c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	      }else {
		c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	      }
	      leader->getAIState()->Communicate(c);
	    }else {
	      //bool targetted=false;
	      //float mindist;
	      //Unit * un=NULL;
	      th= GetThreat(parent,leader);
	      CommunicationMessage c(parent,leader,NULL,0);
	      //fprintf (stderr,"he wnats to help out against threat %d",th);
	      if (th) {
		if (parent->InRange (th,true,false)) {
		  c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
		  parent->Target (th);
		  parent->TargetTurret (th);
		}else {
		  c.SetCurrentState(c.fsm->GetNoNode(),NULL,0);
		}
		//fprintf (stderr,"Helping out kill: %s",th->name.c_str());
	      }else {
		c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	      }
	      leader->getAIState()->Communicate(c);
	    }
	  }
	}
      }
    } 
    last_directive=fg->directive;
  }
  return retval;
}
static bool overridable (const std::string &s) {
  if (s.empty())
    return true;
  return (*s.begin())!=toupper(*s.begin());
}
extern void LeadMe (Unit * un, string directive, string speech);
void AggressiveAI::ReCommandWing(Flightgroup * fg) {
  static float time_to_recommand_wing = XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"TargetCommandierTime",
											"100"));
  if (fg!=NULL) {
    Unit* lead;
    if (overridable (fg->directive)) {//computer won't override capital orders
      if (NULL!=(lead=fg->leader.GetUnit())) {
	  if (float(rand())/RAND_MAX<SIMULATION_ATOM/time_to_recommand_wing) {
	    if (parent->Threat()&&(parent->FShieldData()<.2||parent->RShieldData()<.2)){
	      fg->directive = string("h");
	      LeadMe (parent,"h","I need help here!");
	      fprintf (stderr,"he needs help %s",parent->name.c_str());
	    }else {
	      if (lead->getFgSubnumber()>=parent->getFgSubnumber()) {	
		fg->directive = string("b");
		LeadMe (parent,"b","I'm taking over this wing. Break and attack");
	      }
	    }
	  }
      }
    }
  }
}
void AggressiveAI::Execute () {  
  Flightgroup * fg=parent->getFlightgroup();
  //ReCommandWing(fg);
  FireAt::Execute();
  if (!ProcessCurrentFgDirective (fg)) {
  Unit * target = parent->Target();
  bool isjumpable = target?((!target->GetDestinations().empty())&&parent->GetJumpStatus().drive>=0):false;
  if (!isjumpable &&(
#if 1
      curinter==INTRECOVER||//this makes it so only interrupts may not be interrupted
#endif
      curinter==INTNORMAL)) {
    if ((curinter = (ProcessLogic (*interrupts, true)?INTERR:curinter))==INTERR) {
      logiccurtime=interrupts->maxtime;//set it to the time allotted
    }
  }
  //  if (parent->getAIState()->queryType (Order::FACING)==NULL&&parent->getAIState()->queryType (Order::MOVEMENT)==NULL) { 
  


  if (queryType (Order::FACING)==NULL&&queryType (Order::MOVEMENT)==NULL) { 
    if (isjumpable) {
      AfterburnTurnTowards(this,parent);
    }else {
      if (target) {
	ProcessLogic(*logic);
	curinter=(curinter==INTERR)?INTRECOVER:INTNORMAL;
      }else {
	FlyStraight(this,parent);
	/*
	Order * ord = new Orders::MatchLinearVelocity (parent->ClampVelocity(Vector (0,0,10000),false),true,true,false);
	ord->SetParent(parent);
	EnqueueOrder (ord);
	*/
      }
    }
  } else {
    if (target) {
    logiccurtime-=SIMULATION_ATOM;
    if (logiccurtime<=0) {
      curinter=(curinter==INTERR)?INTRECOVER:INTNORMAL;
      //parent->getAIState()->eraseType (Order::FACING);
      //parent->getAIState()->eraseType (Order::MOVEMENT);
      eraseType (Order::FACING);
      eraseType (Order::MOVEMENT);
      if (isjumpable ) {
	AfterburnTurnTowards(this,parent);
      }else {
	ProcessLogic (*logic);
      }
      logiccurtime = logic->maxtime;      
    }
    }
  }
  }
#ifdef AGGDEBUG
  fprintf (stderr,"endagg");
  fflush (stderr);
#endif    
  if (getTimeCompression()>3) {
	float mag = parent->GetVelocity().Magnitude();
	if (mag>.01)
		mag = 1/mag;
    parent->SetVelocity(parent->GetVelocity()*(mag*parent->GetComputerData().max_speed()/getTimeCompression()));
	parent->NetLocalForce=parent->NetForce=Vector(0,0,0);
  }
}  


