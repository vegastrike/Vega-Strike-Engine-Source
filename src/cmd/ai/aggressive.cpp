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
#include "cmd/unit_util.h"
#include "warpto.h"
#include "cmd/csv.h"
#include "universe_util.h"
#include "vs_random.h"
#include "python/python_compile.h"
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
  EnumMap::Pair ("FShield_Heal_Rate", AggressiveAI::FSHIELD_HEAL_RATE),
  EnumMap::Pair ("BShield_Heal_Rate", AggressiveAI::BSHIELD_HEAL_RATE),
  EnumMap::Pair ("LShield_Heal_Rate", AggressiveAI::LSHIELD_HEAL_RATE),
  EnumMap::Pair ("RShield_Heal_Rate", AggressiveAI::RSHIELD_HEAL_RATE),
  EnumMap::Pair ("FArmor_Heal_Rate", AggressiveAI::FARMOR_HEAL_RATE),
  EnumMap::Pair ("BArmor_Heal_Rate", AggressiveAI::BARMOR_HEAL_RATE),
  EnumMap::Pair ("LArmor_Heal_Rate", AggressiveAI::LARMOR_HEAL_RATE),
  EnumMap::Pair ("RArmor_Heal_Rate", AggressiveAI::RARMOR_HEAL_RATE),
  EnumMap::Pair ("Hull_Heal_Rate", AggressiveAI::HULL_HEAL_RATE),
  EnumMap::Pair ("Target_Faces_You", AggressiveAI::TARGET_FACES_YOU),
  EnumMap::Pair ("Target_In_Front_Of_You", AggressiveAI::TARGET_IN_FRONT_OF_YOU),
  EnumMap::Pair ("Rand", AggressiveAI::RANDOMIZ),
  EnumMap::Pair ("Target_Going_Your_Direction", AggressiveAI::TARGET_GOING_YOUR_DIRECTION)
};
const EnumMap AggressiveAIel_map(element_names, 25);
using std::pair;
std::map<string,AIEvents::ElemAttrMap *> logic;

static map<string,string> getAITypes() {
  map <string,string> ret;
  VSFileSystem::VSFile f;
  VSError err = f.OpenReadOnly( "VegaPersonalities.csv", AiFile);
  if (err<=Ok) {
    CSVTable table(f,f.GetRoot());
    map<std::string,int>::iterator browser=table.rows.begin();
    for (;browser!=table.rows.end();++browser) {
      string rowname = (*browser).first;
      CSVRow row(&table,rowname);
      for (unsigned int i=1;i<table.key.size();++i) {
        string hasher = rowname;
        if (i!=1)
          hasher=rowname+"%"+table.key[i];
        string rawrow=row[i];
        if (rawrow.length()>0) {
          ret[hasher]=rawrow;;
        }
      }
    }
    f.Close();
  }
  return ret;
}
static string select_from_space_list(string inp,unsigned int seed) {
  if (inp.length()==0)
    return "";
  int count=1;
  string::size_type len = inp.length();
  for (unsigned int i=0;i<len;++i) {
    if (inp[i]==' ') {
      count++;
    }
  }
  count = seed%count;
  int ncount=0;
  unsigned int j;
  for (j=0;j<len;++j) {
    if (inp[j]==' ')
      ncount++;
    if (ncount>=count) 
      break;    
  }
  if (inp[j]==' ')
    j++;
  inp=inp.substr(j);
  if ((len=inp.find(" "))!=string::npos) {
    inp = inp.substr(0,len);
  }
  return inp;
}
static AIEvents::ElemAttrMap* getLogicOrInterrupt (string name,int faction, string unittype, std::map<string,AIEvents::ElemAttrMap *> &mymap, int personalityseed) {
  string append="agg";
  static map<string,string>myappend=getAITypes();
  map<string,string>::iterator iter;
  string factionname= FactionUtil::GetFaction(faction);
  if ((iter=myappend.find(factionname+"%"+unittype))!=myappend.end()) {    
    append = select_from_space_list((*iter).second,personalityseed);
  }else if ((iter=myappend.find(factionname))!=myappend.end()) {
    append = select_from_space_list((*iter).second,personalityseed);
  }
  if (append.length()==0) append="agg";
  string hashname = name +"."+append;
  map<string,AIEvents::ElemAttrMap *>::iterator i = mymap.find (hashname);
  if (i==mymap.end()) {
    AIEvents::ElemAttrMap * attr = new AIEvents::ElemAttrMap(AggressiveAIel_map);
    string filename (name+"."+append+".xml");
    AIEvents::LoadAI (filename.c_str(),*attr,FactionUtil::GetFaction(faction));
    mymap.insert (pair<string,AIEvents::ElemAttrMap *> (hashname,attr));
    return attr;
  }
  return i->second;
}
static AIEvents::ElemAttrMap* getProperLogicOrInterruptScript (string name,int faction, string unittype, bool interrupt, int personalityseed) {
  return getLogicOrInterrupt (name,faction,unittype,logic,personalityseed);
}
static AIEvents::ElemAttrMap * getProperScript(Unit * me, Unit * targ, bool interrupt, int personalityseed) {
  if (!me||!targ) {
    string nam="eject";
    int fac=0;
    if (me) {
      fac=me->faction;
      nam=me->name;
    }
    return getProperLogicOrInterruptScript("default",fac,nam,interrupt,personalityseed);
  }
  return getProperLogicOrInterruptScript (ROLES::getRoleEvents(me->combatRole(),targ->combatRole()),me->faction,me->name,interrupt,personalityseed);
}

inline std::string GetRelationshipColor (float rel) {
  if (rel>=1)
    return "#00FF00";
  if (rel<=-1)
    return "#FF0000";
  rel +=1.;
  rel/=2.;
  char str[20]; //Just in case all 8 digits of both #s end up inside the string for some reason.
  sprintf(str,"#%2X%2X00",(int)((1-rel)*256),(int)(rel*256));
  return str;
}

unsigned int DoSpeech (Unit * un, Unit *player_un, const FSM::Node &node) {
  unsigned int dummy=0;
  string speech=node.GetMessage(dummy);
  string myname ("[Static]");
  if (un!=NULL) {
    myname= un->getFullname();
	Flightgroup * fg=un->getFlightgroup();
		if (fg&&fg->name!="base"&&fg->name!="Base") {
			myname = fg->name+" "+XMLSupport::tostring(un->getFgSubnumber())+", "+un->name;
		}else if (myname.length()==0)
			myname = un->name;
	if (player_un!=NULL) {
		if (player_un==un) {
			myname=std::string("#0033FF")+myname+"#000000";
		} else {
			float rel=un->getRelation(player_un);
			myname=GetRelationshipColor(rel)+myname+"#000000";
		}
	}
  }
  mission->msgcenter->add (myname,"all",GetRelationshipColor(node.messagedelta*10)+speech+"#000000"); //multiply by 2 so colors are easier to tell
  return dummy;
}
void LeadMe (Unit * un, string directive, string speech) { 
  if (un!=NULL) {
    for (int i=0;i<_Universe->numPlayers();i++) {
      Unit * pun =_Universe->AccessCockpit(i)->GetParent();
      if (pun) {
	if (pun->getFlightgroup()==un->getFlightgroup()){
		DoSpeech (un, pun, FSM::Node::MakeNode(speech,.1));	
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
static int randomtemp;
AggressiveAI::AggressiveAI (const char * filename, Unit * target):FireAt(), logic (getProperScript(NULL,NULL,"default",randomtemp=rand())) {
  currentpriority=0;
  last_jump_time=0;
  nav=QVector(0,0,0);
  personalityseed=randomtemp;
  last_jump_distance=FLT_MAX;
  interruptcurtime=0;
  creationtime=getNewTime();
  jump_time_check=1;
  last_time_insys=true;
  logiccurtime=logic->maxtime;//set it to the time allotted
  obedient = true;
  if (aggressivity==2.01) {
    float defagg = XMLSupport::parse_float (vs_config->getVariable ("unit","aggressivity","2"));
    aggressivity = defagg;
  }
  if (target !=NULL) {
    AttachOrder (target);
  }
  last_directive = filename;
  //  AIEvents::LoadAI (filename,logic,"neutral");
  //  AIEvents::LoadAI (interruptname,interrupt,"neutral");
}
void AggressiveAI::SetParent (Unit * parent1) {
  FireAt::SetParent(parent1);
  string::size_type which = last_directive.find("|");
  string filename (string("default.agg.xml"));
  string interruptname (string("default.int.xml"));
  if (which!=string::npos) {
    filename = last_directive.substr (0,which);
    interruptname = last_directive.substr(which+1);
  }
  last_directive="b";//prevent escort race condition

  //INIT stored stuff
  Fshield_prev = parent->graphicOptions.InWarp?1:parent->FShieldData();
  Fshield_rate_old = 0.0;
  Fshield_prev_time = UniverseUtil::GetGameTime();
  Bshield_prev = parent->graphicOptions.InWarp?1:parent->BShieldData();
  Bshield_rate_old = 0.0;
  Bshield_prev_time = UniverseUtil::GetGameTime();
  Lshield_prev = parent->graphicOptions.InWarp?1:parent->LShieldData();
  Lshield_rate_old = 0.0;
  Lshield_prev_time = UniverseUtil::GetGameTime();
  Rshield_prev = parent->graphicOptions.InWarp?1:parent->RShieldData();
  Rshield_rate_old = 0.0;
  Rshield_prev_time = UniverseUtil::GetGameTime();
  Farmour_prev = 1.0;
  Farmour_rate_old = 0.0;
  Farmour_prev_time = UniverseUtil::GetGameTime();
  Barmour_prev = 1.0;
  Barmour_rate_old = 0.0;
  Barmour_prev_time = UniverseUtil::GetGameTime();
  Larmour_prev = 1.0;
  Larmour_rate_old = 0.0;
  Larmour_prev_time = UniverseUtil::GetGameTime();
  Rarmour_prev = 1.0;
  Rarmour_rate_old = 0.0;
  Rarmour_prev_time = UniverseUtil::GetGameTime();
  Hull_prev = parent->GetHullPercent();
  Hull_rate_old = 0.0;
  Hull_prev_time = UniverseUtil::GetGameTime();
}
void AggressiveAI::SignalChosenTarget () {
  if (parent) {
    logic =getProperScript(parent,parent->Target(),false,personalityseed);
  }
  FireAt::SignalChosenTarget();
}

bool AggressiveAI::ExecuteLogicItem (const AIEvents::AIEvresult &item) {
  
  if (item.script.length()!=0) {
    Order * tmp = new ExecuteFor(new AIScript (item.script.c_str()),item.timetofinish);	
    //    parent->EnqueueAI (tmp);
    EnqueueOrder (tmp);
    return true;
  }else {
    return false;
  }
}


bool AggressiveAI::ProcessLogicItem (const AIEvents::AIEvresult &item) {
  float value = 0.0;

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
	Vector PosDifference=targ->Position().Cast()-parent->Position().Cast();
	float pdmag = PosDifference.Magnitude();
	value = (pdmag-parent->rSize()-targ->rSize());
	float myvel = PosDifference.Dot(parent->GetVelocity()-targ->GetVelocity())/pdmag;
	
	if (myvel>0)
	  value-=myvel*myvel/(2*(parent->Limits().retro/parent->GetMass()));
      }else {
	value = 10000; 
      }
      value/=game_speed;/*game_accel*/;
    }
    break;
  case THREAT:
    value = parent->GetComputerData().threatlevel;
    break;
  case FSHIELD:
    value = parent->graphicOptions.InWarp?1:parent->FShieldData();
    break;
  case BSHIELD:
    value = parent->graphicOptions.InWarp?1:parent->BShieldData();
    break;
  case HULL:{
    value = parent->GetHullPercent();
    break;}
  case LSHIELD:
    value = parent->graphicOptions.InWarp?1:parent->LShieldData();
    break;
  case RSHIELD:
    value = parent->graphicOptions.InWarp?1:parent->RShieldData();
    break;
  case FSHIELD_HEAL_RATE:{
	double delta_t = UniverseUtil::GetGameTime() - Fshield_prev_time;
    if(delta_t>0.5)	//	0.5 = reaction time limit for hit rate
	{
		double delta_v = parent->graphicOptions.InWarp?1:parent->FShieldData() - Fshield_prev;
		value = delta_v / delta_t;
		Fshield_rate_old = value;
		Fshield_prev_time = UniverseUtil::GetGameTime();
		Fshield_prev = parent->graphicOptions.InWarp?1:parent->FShieldData();
	}
	else {
		value = Fshield_rate_old;}


//		if(value != 0.0)
//		{
//		string mystr ("Fshield "+XMLSupport::tostring (value)); 
//		UniverseUtil::IOmessage (0,"game","all",mystr);
//		}
	break;}
  case BSHIELD_HEAL_RATE:{
	double delta_t = UniverseUtil::GetGameTime() - Bshield_prev_time;
    if(delta_t>0.5)	//	0.5 = reaction time limit for hit rate
	{
		double delta_v = parent->graphicOptions.InWarp?1:parent->BShieldData() - Bshield_prev;
		value = delta_v / delta_t;
		Bshield_rate_old = value;
		Bshield_prev_time = UniverseUtil::GetGameTime();
		Bshield_prev = parent->graphicOptions.InWarp?1:parent->BShieldData();
	}
	else {
		value = Bshield_rate_old;}

//		if(value != 0.0)
//		{
//		string mystr ("Fshield "+XMLSupport::tostring (value)); 
//		UniverseUtil::IOmessage (0,"game","all",mystr);
//		}
	break;}
  case LSHIELD_HEAL_RATE:{
	double delta_t = UniverseUtil::GetGameTime() - Lshield_prev_time;
    if(delta_t>0.5)	//	0.5 = reaction time limit for hit rate
	{
		double delta_v = parent->graphicOptions.InWarp?1:parent->LShieldData() - Lshield_prev;
		value = delta_v / delta_t;
		Lshield_rate_old = value;
		Lshield_prev_time = UniverseUtil::GetGameTime();
		Lshield_prev = parent->graphicOptions.InWarp?1:parent->LShieldData();
	}
	else {
		value = Lshield_rate_old;}

//		if(value != 0.0)
//		{
//		string mystr ("Fshield "+XMLSupport::tostring (value)); 
//		UniverseUtil::IOmessage (0,"game","all",mystr);
//		}
	break;}
  case RSHIELD_HEAL_RATE:{
	double delta_t = UniverseUtil::GetGameTime() - Rshield_prev_time;
    if(delta_t>0.5)	//	0.5 = reaction time limit for hit rate
	{
		double delta_v = parent->graphicOptions.InWarp?1:parent->RShieldData() - Rshield_prev;
		value = delta_v / delta_t;
		Rshield_rate_old = value;
		Rshield_prev_time = UniverseUtil::GetGameTime();
		Rshield_prev = parent->graphicOptions.InWarp?1:parent->RShieldData();
	}
	else {
		value = Rshield_rate_old;}

//		if(value != 0.0)
//		{
//		string mystr ("Fshield "+XMLSupport::tostring (value)); 
//		UniverseUtil::IOmessage (0,"game","all",mystr);
//		}
	break;}
  case FARMOR_HEAL_RATE:
	value = 0.0;
	break;
  case BARMOR_HEAL_RATE:
	value = 0.0;
	break;
  case LARMOR_HEAL_RATE:
	value = 0.0;
	break;
  case RARMOR_HEAL_RATE:
	value = 0.5;
	break;
  case HULL_HEAL_RATE:{
	double delta_t = UniverseUtil::GetGameTime() - Hull_prev_time;
    if(delta_t>0.5)	//	0.5 = reaction time limit for hit rate
	{
		double delta_v = parent->GetHullPercent() - Hull_prev;
		value = delta_v / delta_t;
		Hull_rate_old = value;
		Hull_prev_time = UniverseUtil::GetGameTime();
		Hull_prev = parent->GetHullPercent();
	}
	else {
		value = Hull_rate_old;}

//		if(value != 0.0)
//		{
//		string mystr ("Fshield "+XMLSupport::tostring (value)); 
//		UniverseUtil::IOmessage (0,"game","all",mystr);
//		}

	break;}
  case TARGET_FACES_YOU:
	{
	value = 0.0;
    Unit * targ = parent->Target();
    if (targ) {

	Vector Q;
	Vector P;
	Vector R;

	Vector PosDelta=(parent->Position()) - (targ->Position());
	PosDelta = PosDelta/PosDelta.Magnitude();
	targ->GetOrientation(Q, P, R);
	value = PosDelta.Dot(R);
	}
	}
	break;
  case TARGET_IN_FRONT_OF_YOU:
	{
	value = 0.0;
    Unit * targ = parent->Target();
    if (targ) {

	Vector Q;
	Vector P;
	Vector S;

	Vector PosDelta=(targ->Position()) - (parent->Position());
	PosDelta = PosDelta/PosDelta.Magnitude();
	parent->GetOrientation(Q, P, S);
	value = PosDelta.Dot(S);
	}
	}
	break;
  case TARGET_GOING_YOUR_DIRECTION:
	{
	value = 0.0;
    Unit * targ = parent->Target();
    if (targ) {
	Vector Q;
	Vector P;
	Vector R;
	Vector S;

	parent->GetOrientation(Q, P, S);
	targ->GetOrientation(Q, P, R);
	value = S.Dot(R);
	}
	}
	break;
  case FACING:
    //    return parent->getAIState()->queryType (Order::FACING)==NULL;
    return queryType (Order::FACING)==NULL;
  case MOVEMENT:
    //    return parent->getAIState()->queryType (Order::MOVEMENT)==NULL;
    return queryType (Order::MOVEMENT)==NULL;
  case RANDOMIZ:
    value= ((float)rand())/RAND_MAX;
    break;
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
      if (j != i->begin())
        j--;
      if (j!=i->end()) {
        float priority = (*j).priority;
        if (priority>this->currentpriority||!inter) {
          if (inter) {

            //parent->getAIState()->eraseType (Order::FACING);
            //parent->getAIState()->eraseType (Order::MOVEMENT);
            eraseType (Order::FACING);
            eraseType (Order::MOVEMENT);


          }
          j = i->begin();
          logiccurtime=0;
          interruptcurtime=0;
          if (j!=i->end()) {
            while (j!=i->end()) {
              if (ExecuteLogicItem (*j)) {
                this->currentpriority=priority;          
                logiccurtime += (*j).timetofinish;
                interruptcurtime += (*j).timetointerrupt;
                //AIEvents::AIEvresult tmp = *j;
                //i->erase(j);
                retval=true;
                //i->push_back (tmp);
              }
              j++;
            }
            if (retval) break;
          }
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
	  static bool forceobedient= XMLSupport::parse_bool (vs_config->getVariable("AI","always_obedient","true"));
	  if (forceobedient)
		  obedient=true;
	  else if (float(rand())/RAND_MAX<(obedient?(1-logic->obedience):logic->obedience)) {
		  obedient = !obedient;
      }
      if (obedient) {
	eraseType (Order::FACING);
	eraseType (Order::MOVEMENT);
	Unit * targ = parent->Target();
	if (targ) {
          bool attacking=fg->directive.length()>0;
          if (attacking)
            attacking=tolower(fg->directive[0])=='a';
	  if ((!isJumpablePlanet (targ))&&attacking==false) {
	    parent->Target(NULL);
	  }
	}
      }else {
	    CommunicationMessage c(parent,leader,NULL,0);
	    c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
		Order * lo = leader->getAIState();
		if (lo)
			lo->Communicate(c);
      }
    }
    if (obedient) {
      if (fg->directive.find("a")!=string::npos||fg->directive.find("A")!=string::npos) {
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
			Order * lo = leader->getAIState();
			if (lo)
				lo->Communicate(c);
	    }
	  }
	}
      }else if (fg->directive.find("f")!=string::npos||fg->directive.find("F")!=string::npos) {
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
		  Order * o = leader->getAIState();
		  if (o)
			  o->Communicate(c);
	      static float esc_percent= XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"EscortDistance",
											"10.0"));
	      static float turn_leader= XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"TurnLeaderDist",
											"5.0"));
	      int fgnum = parent->getFgSubnumber();
		  if (parent->getFlightgroup()) {
			  int tempnum=0;
			  string nam = parent->getFlightgroup()->name;
			  int i=nam.length()-1;
			  for (;i>=0;--i) {
				  char digit = nam[i];
				  if (digit>='0'&&digit<='9') {
					  tempnum*=10;
					  tempnum+=digit-'0';
				  }else
					  break;				  
			  }
			  fgnum+=tempnum;
			  
		  }
	      float left= fgnum%2?1:-1;		  
	      double dist=esc_percent*(1+fgnum/2)*left*(parent->rSize()+leader->rSize());
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
      }else if (fg->directive.find("h")!=string::npos||fg->directive.find("H")!=string::npos) {
	//	VSFileSystem::vs_fprintf (stderr,"he wnats to help out");
	if (fg->directive!=last_directive&&leader) {
	  if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    //VSFileSystem::vs_fprintf (stderr,"%s he wnats to help out and hasn't died\n", parent->name.c_str());
	    Unit * th=NULL;
	    if ((th=leader->Threat())) {
	      //VSFileSystem::vs_fprintf (stderr,"he wnats to help out and he has a threat\n");

	      CommunicationMessage c(parent,leader,NULL,0);
	      if (parent->InRange(th,true,false)) {
		parent->Target(th);
		parent->TargetTurret(th);
		c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
	      }else {
		c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	      }
		  Order * oo = leader->getAIState();
		  if (oo) 
			  oo->Communicate(c);
	    }else {
	      //bool targetted=false;
	      //float mindist;
	      //Unit * un=NULL;
	      th= GetThreat(parent,leader);
	      CommunicationMessage c(parent,leader,NULL,0);
	      //VSFileSystem::vs_fprintf (stderr,"he wnats to help out against threat %d",th);
	      if (th) {
		if (parent->InRange (th,true,false)) {
		  c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
		  parent->Target (th);
		  parent->TargetTurret (th);
		}else {
		  c.SetCurrentState(c.fsm->GetNoNode(),NULL,0);
		}
		//VSFileSystem::vs_fprintf (stderr,"Helping out kill: %s",th->name.c_str());
	      }else {
		c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	      }
		  Order * loo = leader->getAIState();
		  if (loo)
			  loo->Communicate(c);
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
	      VSFileSystem::vs_fprintf (stderr,"he needs help %s",parent->name.c_str());
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
static Unit * ChooseNavPoint(Unit * parent) {
  static string script=vs_config->getVariable("AI","ChooseDestinationScript","");

  if (script.length()>0) {
    Unit * ret=NULL;
    UniverseUtil::setScratchUnit(parent);
    CompileRunPython(script);
    ret=UniverseUtil::getScratchUnit();
    UniverseUtil::setScratchUnit(NULL);
    if (ret!=NULL&&ret!=parent) {
      return ret;
    }
  }
  Unit* un;
  vector <Unit*> navs;
  for (un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
       (un=*i)!=NULL;
       ++i) {
    if (UnitUtil::isSignificant(un)) {
      if (parent->getRelation(un)>=-.05) {
        navs.push_back(un);
      }
    }
  } 
  if (navs.size()>0) {
    int k = (int)(getNewTime()/120);// two minutes
    string key = UnitUtil::getFlightgroupName(parent);
    std::string::const_iterator start = key.begin();
    for(;start!=key.end(); start++) {
      k += (k * 128) + *start;
    }
    VSRandom choosePlace(k);
    return navs[choosePlace.genrand_int32()%navs.size()];
  }
  return NULL;
}

static Unit * ChooseNearNavPoint(Unit * parent,QVector location, float locradius) {
  Unit * candidate=NULL;
  float dist = FLT_MAX;
  Unit * un;
  for (un_iter i= _Universe->activeStarSystem()->getUnitList().createIterator();
       (un=*i)!=NULL;
       ++i) {
    if (UnitUtil::isSignificant(un)&&un!=parent) {
      float newdist = (location-un->Position()).Magnitude()-un->rSize()-locradius;
      if (candidate==NULL||newdist<=dist) {
        candidate=un;
        dist=newdist;
      }              
    }
  }
  return candidate;
}

class FlyTo:public Orders::MoveTo {
  float creationtime;
public:
  FlyTo(const QVector &target, bool aft, bool terminating=true, float creationtime=0) : MoveTo(target,aft,4,terminating) {this->creationtime=creationtime;}

  virtual void Execute() {
    MoveTo::Execute();
    Unit * un=NULL;
    static float mintime=XMLSupport::parse_float(vs_config->getVariable("AI","min_time_to_auto","25"));
    if (getNewTime()-creationtime>mintime) {
      if (_Universe->AccessCockpit()->autoInProgress()&&(!_Universe->AccessCockpit()->unitInAutoRegion(parent))&&(un =ChooseNearNavPoint(parent,targetlocation,0))!=NULL) {
        WarpToP(parent,un,true);
      }else {
        Unit* playa=_Universe->AccessCockpit()->GetParent();
        if (playa==NULL||playa->Target()!=parent) {
          WarpToP(parent,targetlocation,0,true);
        }
      }
    }
  }
};
static Vector randVector() {
  return Vector((rand()/(float)RAND_MAX)*2-1,(rand()/(float)RAND_MAX)*2-1,(rand()/(float)RAND_MAX)*2-1);
}
static void GoTo(AggressiveAI * ai, Unit * parent, const QVector &nav, float creationtime) {
  static bool can_afterburn = XMLSupport::parse_bool(vs_config->getVariable("AI","afterburn_to_no_enemies","true")); 
  Order * mt=new FlyTo(nav,can_afterburn,true,creationtime);
  Order * ch=new Orders::ChangeHeading(nav,32,.25f,true);
  mt->SetParent(parent);
  ch->SetParent(parent);
  ai->ReplaceOrder(mt);
  ai->EnqueueOrder(ch);
}
void AggressiveAI::ExecuteNoEnemies() {
  if (nav.i==0&&nav.j==0&&nav.k==0) {
    Unit * dest=ChooseNavPoint (parent);
    if (dest) {
      static bool can_warp_to=XMLSupport::parse_bool(vs_config->getVariable("AI","warp_to_no_enemies","true"));      
      static float mintime=XMLSupport::parse_float(vs_config->getVariable("AI","min_time_to_auto","25"));
      if (getNewTime()-creationtime>mintime) {
        if (can_warp_to) {
          WarpToP(parent,dest,true);
        }else if (_Universe->AccessCockpit()->autoInProgress()&&!_Universe->AccessCockpit()->unitInAutoRegion(parent)) {
          
          WarpToP(parent,dest,true);
        }
      }
      Vector dir = parent->Position()-dest->Position();
      dir.Normalize();
      dir*=dest->rSize()+parent->rSize();
      dir+=randVector()*parent->rSize()*4;
      if (dest->isUnit()==PLANETPTR) {
        float planetpct=UniverseUtil::getPlanetRadiusPercent();
        dir *=planetpct+1.0f;
      }
      nav=dest->Position()+dir;      
      GoTo(this,parent,nav,creationtime);
    }
  }else {          
    if ((nav-parent->Position()).MagnitudeSquared()<4*parent->rSize()*parent->rSize()) {
      nav=QVector(0,0,0);
      Unit * dest =ChooseNearNavPoint(parent,parent->Position(),parent->rSize());
      if (dest) {
        if (dest->GetDestinations().size()>0&&UniverseUtil::systemInMemory(dest->GetDestinations()[0])) {
          parent->ActivateJumpDrive(0);
          parent->Target(dest);// fly there, baby!          
        }else if (dest->GetDestinations().size()==0&&UnitUtil::isDockableUnit(dest)){          
          UnitUtil::performDockingOperations(parent,dest,0);//dock there, baby
        }else {
          ExecuteNoEnemies();//find a new place to go
        }
      }else {
        ExecuteNoEnemies();//no suitable docking point found, recursive call which will take door1
      }
      // go dock to the nav point
    }else {
      GoTo(this,parent,nav,creationtime);
    }
  }
  /*
    Order * ord = new Orders::MatchLinearVelocity (parent->ClampVelocity(Vector (0,0,10000),false),true,true,false);
    ord->SetParent(parent);
    EnqueueOrder (ord);
  */
}


void AggressiveAI::AfterburnerJumpTurnTowards (Unit * target) {
  AfterburnTurnTowards(this,parent);
  static float jump_time_limit=XMLSupport::parse_float (vs_config->getVariable ("AI","force_jump_after_time","120"));
  if (jump_time_check==0) {
    float dist = (target->Position()- parent->Position()).MagnitudeSquared();
    if (last_jump_distance<dist||last_jump_time>jump_time_limit) {
      //force jump
      last_jump_time=0;
      if (target->GetDestinations().size()) {
	string dest= target->GetDestinations()[0];
	UnitUtil::JumpTo(parent,dest);
      }
    }else {
      last_jump_distance = dist;
    }
  }
  
}
void AggressiveAI::Execute () {  
  jump_time_check++;//just so we get a nicely often wrapping var;
  jump_time_check%=5;
  Flightgroup * fg=parent->getFlightgroup();
  //ReCommandWing(fg);
  FireAt::Execute();
  Unit * target = parent->Target();

  bool isjumpable = target?(!target->GetDestinations().empty()):false;
  
  if (!ProcessCurrentFgDirective (fg)) {
  if (isjumpable) {
  if (parent->GetJumpStatus().drive<0) {
    parent->ActivateJumpDrive(0);
    if (parent->GetJumpStatus().drive==-2) {
      static bool AIjumpCheat=XMLSupport::parse_bool (vs_config->getVariable ("AI","always_have_jumpdrive_cheat","false"));
      if (AIjumpCheat) {
		  static int i=0;
		  if (!i) {
			  VSFileSystem::vs_fprintf (stderr,"FIXME: warning ship not equipped to jump");
			  i=1;
		  }
		  parent->jump.drive=-1;
      }else {
	//	VSFileSystem::vs_fprintf (stderr,"warning ship not equipped to jump");
	parent->Target(NULL);
      }
    }else if (parent->GetJumpStatus().drive<0){
      static bool AIjumpCheat=XMLSupport::parse_bool (vs_config->getVariable ("AI","jump_cheat","true"));
      if (AIjumpCheat) {
	parent->jump.drive=0;
      }
    }
  }
  last_jump_time+=SIMULATION_ATOM;
  }else {
    last_jump_time=0;
  }
  if ((!isjumpable) &&interruptcurtime<=0&&target) {
//	  fprintf (stderr,"i");
	  ProcessLogic (*logic, true);
  }
  if (!target) {
    logiccurtime-=SIMULATION_ATOM;
    if (logiccurtime<0) {
      logiccurtime=20;
      currentpriority=-FLT_MAX;
      eraseType (Order::FACING);
      eraseType (Order::MOVEMENT);      
    }
  }

  //  if (parent->getAIState()->queryType (Order::FACING)==NULL&&parent->getAIState()->queryType (Order::MOVEMENT)==NULL) { 
  

  if (queryAny (Order::FACING|Order::MOVEMENT)==NULL) { 
    if (isjumpable) {
      AfterburnerJumpTurnTowards (target);
    }else {
      last_jump_distance=FLT_MAX;
      if (target) {
		  ProcessLogic(*logic,false);
      }else {
        ExecuteNoEnemies();
      }
    }
  } else {
    if (target) {
    static bool can_warp_to=XMLSupport::parse_bool(vs_config->getVariable("AI","warp_to_enemies","true"));      
    if (can_warp_to||_Universe->AccessCockpit()->autoInProgress()){
      WarpToP(parent,target,false);
    }
    logiccurtime-=SIMULATION_ATOM;
    interruptcurtime-=SIMULATION_ATOM;	
    if (logiccurtime<=0) {
      //parent->getAIState()->eraseType (Order::FACING);
      //parent->getAIState()->eraseType (Order::MOVEMENT);
      eraseType (Order::FACING);
      eraseType (Order::MOVEMENT);
      if (isjumpable ) {
		  AfterburnerJumpTurnTowards (target);
	      logiccurtime = logic->maxtime;      
      }else {
		  last_jump_distance=FLT_MAX;
		  ProcessLogic (*logic,false);
      }

    }
    }
  }
  }
#ifdef AGGDEBUG
  VSFileSystem::vs_fprintf (stderr,"endagg");
  fflush (stderr);
#endif    
  if (getTimeCompression()>3) {
	float mag = parent->GetVelocity().Magnitude();
	if (mag>.01)
		mag = 1/mag;
    parent->SetVelocity(parent->GetVelocity()*(mag*parent->GetComputerData().max_speed()/getTimeCompression()));
	parent->NetLocalForce=parent->NetForce=Vector(0,0,0);
  }
  target = parent->Target();

  isjumpable = target?(!target->GetDestinations().empty()):false;

  if (!isjumpable) {
     if (parent->GetJumpStatus().drive>=0) {
        parent->ActivateJumpDrive(-1);
     }     
  }
}  


