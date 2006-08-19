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
#include "cmd/unit_find.h"
#include "faction_generic.h"
using namespace Orders;
using stdext::hash_map;
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
stdext::hash_map<string,AIEvents::ElemAttrMap *> logic;

extern bool CheckAccessory (Unit *tur);
//extern void TurretFAW(Unit *parent); /*
static void TurretFAW(Unit * parent) {
  UnitCollection::UnitIterator iter = parent->getSubUnits();
  Unit * un;
  while (NULL!=(un=iter.current())) {
    if (!CheckAccessory(un)) {
      un->EnqueueAIFirst (new Orders::FireAt(15.0f));
      un->EnqueueAIFirst (new Orders::FaceTarget (false,3));
    }
    TurretFAW(un);
    iter.advance();
  }
  
}

static stdext::hash_map<string,string> getAITypes() {
	stdext::hash_map<string,string> ret;
  VSFileSystem::VSFile f;
  VSError err = f.OpenReadOnly( "VegaPersonalities.csv", AiFile);
  if (err<=Ok) {
    CSVTable table(f,f.GetRoot());
	stdext::hash_map<std::string,int>::iterator browser=table.rows.begin();
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
static AIEvents::ElemAttrMap* getLogicOrInterrupt (string name,int faction, string unittype, stdext::hash_map<string,AIEvents::ElemAttrMap *> &mymap, int personalityseed) {
  string append="agg";
  static stdext::hash_map<string,string>myappend=getAITypes();
  stdext::hash_map<string,string>::iterator iter;
  string factionname= FactionUtil::GetFaction(faction);
  if ((iter=myappend.find(factionname+"%"+unittype))!=myappend.end()) {    
    append = select_from_space_list((*iter).second,personalityseed);
  }else if ((iter=myappend.find(factionname))!=myappend.end()) {
    append = select_from_space_list((*iter).second,personalityseed);
  }
  if (append.length()==0) append="agg";
  string hashname = name +"."+append;
  stdext::hash_map<string,AIEvents::ElemAttrMap *>::iterator i = mymap.find (hashname);
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
  return getProperLogicOrInterruptScript (ROLES::getRoleEvents(me->attackPreference(),targ->unitRole()),me->faction,me->name,interrupt,personalityseed);
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
    
    myname= un->isUnit()==PLANETPTR?un->name:un->getFullname();
	Flightgroup * fg=un->getFlightgroup();
		if (fg&&fg->name!="base"&&fg->name!="Base") {
			myname = fg->name+" "+XMLSupport::tostring(un->getFgSubnumber())+", "+un->getFullname();
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
void LeadMe (Unit * un, string directive, string speech, bool changetarget) { 

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
      if (changetarget)

		  fg->target.SetUnit (un->Target());

      if ((directive == ""))

		  fg->target.SetUnit (NULL);

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
    static float defagg = XMLSupport::parse_float (vs_config->getVariable ("unit","aggressivity","2"));
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
	float myvel = PosDifference.Dot(parent->GetVelocity()-targ->GetVelocity())/value; // /pdmag;
	
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

extern Cargo * GetMasterPartList (const char *);

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

		
          void * parentowner;
          void * leaderowner = leader;
          parentowner = parent->owner?parent->owner:parent;
          leaderowner = leader->owner?leader->owner:leader;

		
		if (fg->directive.find("k")!=string::npos||fg->directive.find("k")!=string::npos) {
    	Unit * targ = fg->target.GetUnit();
		bool callme = false;
	if (targ && (targ->faction != parent->faction)) {
	  if (targ->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    CommunicationMessage c(parent,leader,NULL,0);
            c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);
	    if (parent->InRange (targ,true,false)) {
                if (targ != parent->Target())
					callme = true;
                parent->Target(targ);
                parent->SetTurretAI();
                parent->TargetTurret(targ);
 
              // if I am the capship, go into defensive mode.
              if (parent == leaderowner) {
                // get in front of me
                parent->TurretFAW();

				c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
                //          MatchVelocity(parent->ClampVelocity(vec,true),Vector(0,0,0),true,true,false)
                //		  Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
                Order * ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,0,0),true),true,false,true);
                ord->SetParent (parent);
                ReplaceOrder (ord);
                // facing forward
                ord = new Orders::FaceTarget (false,3);
                ord->SetParent (parent);
                ReplaceOrder (ord);
              } else {
                Order * ord = new Orders::FaceTarget (false,3);
                ord->SetParent (parent);
                ReplaceOrder (ord);
                c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
              }
            }else {
              c.SetCurrentState (c.fsm->GetNoNode(),NULL,0);             
            }
            if (fg->directive!=last_directive) {
              Order * lo = leader->getAIState();
              if (lo || callme)
                lo->Communicate(c);
            }
	  }
	}   // a is now used for AI, for backward compatibility. do not use for player
      }else if (fg->directive.find("a")!=string::npos||fg->directive.find("A")!=string::npos) {
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
		  
	      double dist=esc_percent*(1+abs(fgnum-1)/2)*left*(parent->rSize()+leader->rSize());
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
      }else if (fg->directive.find("l")!=string::npos||fg->directive.find("L")!=string::npos) { 
	if (leader!=NULL) {
	  if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    retval=true;
	    if (fg->directive!=last_directive||(!last_time_insys)) {
	      last_time_insys=true;
	      CommunicationMessage c(parent,leader,NULL,0);
// this order is only valid for cargo wingmen, other wingmen will not comply
          c.SetCurrentState (c.fsm->GetYesNode(),NULL,0); 
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

/*
// this does the job for real! "parent" is executor, "leader" is commander

// moves where you want it to
// moves flat out in front of parent unit (to allow for tractoring)
		  Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
// faces same direction as leader
//		  ord = new Orders::FaceDirection(dist*turn_leader);
// faces opposite direction as leader, as in, stare at me in the face please
		  ord = new Orders::FaceDirection(-dist*turn_leader);
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
*/          

		  int alternate = fgnum%2?1:-1;
		  int psize = parent->radial_size;
		  int Ypos = 0;
		  int Xpos = 0;
		  int position = floor((fgnum%3) * 0.5 * alternate);
// nice square formation, how many of these are you going to have anyway? Max 9, then go back. Should be enough.
		  switch (fgnum%9) {
		  case 0: Xpos = 0; Ypos = 0; break;
		  case 1: Xpos = -1; Ypos = 0; break;
		  case 2: Xpos = 1; Ypos = 0; break;
		  case 3: Xpos = 0; Ypos = -1; break;
		  case 4: Xpos = -1; Ypos = -1; break;
		  case 5: Xpos = 1; Ypos = -1; break;
		  case 6: Xpos = 0; Ypos = 1; break;
		  case 7: Xpos = -1; Ypos = 1; break;
		  case 8: Xpos = 1; Ypos = 1; break;
          default: Xpos = 0; Ypos = 0;
		  }

		  float dist=(leader->radial_size+parent->radial_size);
	      float formdist=esc_percent*(1+fgnum*2)*alternate*(dist);


		  // if i am a cargo wingman, get into a dockable position
          if (parentowner == leader) {
            Order * ord = new Orders::FormUp(QVector(1.1*Xpos*psize,1.1*Ypos*psize,fabs(dist)));
            ord->SetParent (parent);
            ReplaceOrder (ord);
            // facing me
            ord = new Orders::FaceDirection(-dist*turn_leader);
            ord->SetParent (parent);
            ReplaceOrder (ord);
          }
          // if i am a cargo wingman and so is the player, get into a dockable position with the leader          
          else if (parentowner && leaderowner && (parentowner == leaderowner))
          {
//	      float left= fgnum%2?1:-1;	
            Unit * leaderownerun=(leaderowner==leader?leader:(leaderowner==parent?parent:findUnitInStarsystem(leaderowner)));
                  float qdist=(parent->rSize() + leaderownerun->rSize());
		  Order * ord = new Orders::MoveTo(leaderownerun->Position() + Vector(0.5*Xpos*psize,0.5*Ypos*psize,0.5*qdist), true, 4);
   		  ord->SetParent (parent);
                  ReplaceOrder (ord);
		  // facing it
		  ord = new Orders::FaceDirection(-qdist*turn_leader);
                  ord->SetParent (parent);
                  ReplaceOrder (ord);
		  }

          // if i am the capship, go into defensive mode
          else if (parent == leaderowner)
          {
//		  // parent->Target(parent);
		  parent->SetTurretAI();
		  TurretFAW(parent);
		  Order * ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,0,0),true),true,false,true);
   		  ord->SetParent (parent);
	      ReplaceOrder (ord);
		  if (parent->Target() != NULL)
    		  ord = new Orders::FaceTarget (false,3);
		  else
	    	  ord = new Orders::FaceDirection(-dist*turn_leader);
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
          }
          else
            // if i'm not a cargo wingman, just form up somewhat loosely.
          {
            parentowner = parent;
            Order * ord = new Orders::FormUp(QVector(5*Xpos*psize,5*Ypos*psize,-fabs(formdist)+Ypos*psize+Xpos*psize));
            ord->SetParent (parent);
            ReplaceOrder (ord);
            ord = new Orders::FaceDirection(dist*turn_leader);
            ord->SetParent (parent);
            ReplaceOrder (ord);
          }
          


            }
	  } else {
	    last_time_insys=false;
	  }
	  for (unsigned int i=0;i<suborders.size();i++) {
	    suborders[i]->AttachSelfOrder (leader);
	  }
	}
      }else if (fg->directive.find("e")!=string::npos||fg->directive.find("E")!=string::npos) { 
       static QVector LeaderPosition = QVector (0,0,0);

	if (LeaderPosition.Magnitude() > 0 || leader!=NULL) {
	  if (LeaderPosition.Magnitude() > 0 || leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    retval=true;
		if (LeaderPosition.Magnitude() == 0)   // only read the position the first time
			LeaderPosition = leader->Position();
	    if (fg->directive!=last_directive||(!last_time_insys)) {
	      last_time_insys=true;
	      CommunicationMessage c(parent,leader,NULL,0);
// this order is only valid for cargo wingmen, other wingmen will not comply
          c.SetCurrentState (c.fsm->GetYesNode(),NULL,0); 
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

/*
// this does the job for real! "parent" is executor, "leader" is commander

// moves where you want it to
// moves flat out in front of parent unit (to allow for tractoring)
		  Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
// faces same direction as leader
//		  ord = new Orders::FaceDirection(dist*turn_leader);
// faces opposite direction as leader, as in, stare at me in the face please
		  ord = new Orders::FaceDirection(-dist*turn_leader);
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
*/          

		  int alternate = fgnum%2?1:-1;
		  int psize = parent->radial_size;
		  int Ypos = 0;
		  int Xpos = 0;
		  int position = floor((fgnum%3) * 0.5 * alternate);
// nice square formation, how many of these are you going to have anyway? Max 9, then go back. Should be enough.
		  switch (fgnum%9) {
		  case 0: Xpos = 0; Ypos = 0; break;
		  case 1: Xpos = -1; Ypos = 0; break;
		  case 2: Xpos = 1; Ypos = 0; break;
		  case 3: Xpos = 0; Ypos = -1; break;
		  case 4: Xpos = -1; Ypos = -1; break;
		  case 5: Xpos = 1; Ypos = -1; break;
		  case 6: Xpos = 0; Ypos = 1; break;
		  case 7: Xpos = -1; Ypos = 1; break;
		  case 8: Xpos = 1; Ypos = 1; break;
          default: Xpos = 0; Ypos = 0;
		  }

		  float dist=(leader->radial_size+parent->radial_size);
	      float formdist=esc_percent*(1+fgnum*2)*alternate*(dist);

          // if i am a cargo wingman go close for pickup          
          // if i am the capship, go close for pickup
		  if ((parent->owner == leader->owner) || parent->owner == leader)
		  {
//	      float left= fgnum%2?1:-1;		  
	      float qdist=(1.5*parent->rSize()+1.5*leader->rSize());
		  Order * ord = new Orders::MoveTo(LeaderPosition+Vector(0.5*Xpos*psize,0.5*Ypos*psize,0.5*qdist), true, 4);
   		  ord->SetParent (parent);
	      ReplaceOrder (ord);
		  // facing it
		  ord = new Orders::FaceDirection(-qdist*turn_leader);
	      ord->SetParent (parent);
	      ReplaceOrder (ord);
		  }

          // if i'm not a cargo wingman, IT'S NOT MY PROBLEM.          
		  else {
          parent->owner = parent;
		  c.SetCurrentState(c.fsm->GetNoNode(),NULL,0);
          Flightgroup* leave=new Flightgroup();
		  leave->directive="b";
          parent->SetFg (leave, 1);
		  }			  
		  Order * o = leader->getAIState();
		  if (o)
			  o->Communicate(c);

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

// if I am the capship, go into defensive mode.
		  if (parent == leader->owner)
		  {
		  // parent->Target(parent);
		  parent->SetTurretAI();
		  TurretFAW(parent);
//          MatchVelocity(parent->ClampVelocity(vec,true),Vector(0,0,0),true,true,false)
//		  Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
		  Order * ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,0,0),true),true,false,true);
   		  ord->SetParent (parent);
	      ReplaceOrder (ord);
		  if (parent->Target() != NULL)
			{
     		 ord = new Orders::FaceTarget (false,3);
	         ord->SetParent (parent);
	         ReplaceOrder (ord);
			}
		  }



		}else {

		
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
      }else if (fg->directive.find("p")!=string::npos||fg->directive.find("P")!=string::npos) {
	//	VSFileSystem::vs_fprintf (stderr,"he wnats to help out");
	bool callme = false;

	if (fg->directive!=last_directive&&leader) {
	  if (leader->InCorrectStarSystem(_Universe->activeStarSystem())) {
	    //VSFileSystem::vs_fprintf (stderr,"%s he wnats to help out and hasn't died\n", parent->name.c_str());
	    Unit * th=NULL;
    	Unit * targ = fg->target.GetUnit();

	    if (targ && (th=targ->Threat())) {
	      //VSFileSystem::vs_fprintf (stderr,"he wnats to help out and he has a threat\n");

	      CommunicationMessage c(parent,leader,NULL,0);
	      if (parent->InRange(th,true,false)) {
		parent->Target(th);
		parent->TargetTurret(th);
		c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
        fg->directive = "";
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
	      th= GetThreat(parent,targ);
	      CommunicationMessage c(parent,leader,NULL,0);
	      //VSFileSystem::vs_fprintf (stderr,"he wnats to help out against threat %d",th);
	      if (th) {
		if (parent->InRange (th,true,false)) {
		  c.SetCurrentState (c.fsm->GetYesNode(),NULL,0);
          if (th!= parent->Target())
				callme = true;

		  parent->Target (th);
		  parent->TargetTurret (th);
// if I am the capship, go into defensive mode.
		  if (parent == leaderowner)
		  {
		  // parent->Target(parent);
		  parent->SetTurretAI();
		  parent->TurretFAW();
//          MatchVelocity(parent->ClampVelocity(vec,true),Vector(0,0,0),true,true,false)
//		  Order * ord = new Orders::FormUp(QVector(position*parent->radial_size,0,fabs(dist)));
		  Order * ord = new Orders::MatchLinearVelocity(parent->ClampVelocity(Vector(0,0,0),true),true,false,true);
   		  ord->SetParent (parent);
	      ReplaceOrder (ord);
		  if (parent->Target() != NULL)
			{
			ord = new Orders::FaceTarget (false,3);
			ord->SetParent (parent);
			ReplaceOrder (ord);
			}
		  }

		
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
extern void LeadMe (Unit * un, string directive, string speech, bool changetarget);
void AggressiveAI::ReCommandWing(Flightgroup * fg) {
  static float time_to_recommand_wing = XMLSupport::parse_float(vs_config->getVariable ("AI",
											"Targetting",
											"TargetCommandierTime",
											"100"));
  static bool verbose_debug = XMLSupport::parse_bool(vs_config->getVariable("data","verbose_debug","false"));
  if (fg!=NULL) {
    Unit* lead;
    if (overridable (fg->directive)) {//computer won't override capital orders
      if (NULL!=(lead=fg->leader.GetUnit())) {
	  if (float(rand())/RAND_MAX<SIMULATION_ATOM/time_to_recommand_wing) {
	    if (parent->Threat()&&(parent->FShieldData()<.2||parent->RShieldData()<.2)){
	      fg->directive = string("h");
	      LeadMe (parent,"h","I need help here!", false);
		  if(verbose_debug){
			  VSFileSystem::vs_fprintf (stderr,"he needs help %s",parent->name.c_str());
		  }
	    }else {
	      if (lead->getFgSubnumber()>=parent->getFgSubnumber()) {	
		fg->directive = string("b");
		LeadMe (parent,"b","I'm taking over this wing. Break and attack", false);
	      }
	    }
	  }
      }
    }
  }
}
static Unit * GetRandomNav(vector<UnitContainer> navs[3], unsigned int randnum) {
  size_t total_size = navs[0].size()+navs[1].size()+navs[2].size();
  if (total_size==0) return NULL;
  randnum%=total_size;
  if (randnum>=navs[0].size()) {
    randnum-=navs[0].size();
    if (randnum>=navs[1].size()) {    
      randnum-=navs[1].size();
      return navs[2][randnum].GetUnit();
    }
    return navs[1][randnum].GetUnit();
  }
  return navs[0][randnum].GetUnit();
}
static std::string insysString("Insys");
static std::string arrowString("->");
static Unit * ChooseNavPoint(Unit * parent, Unit **otherdest, float *lurk_on_arrival) {
  static string script=vs_config->getVariable("AI","ChooseDestinationScript","");
  *lurk_on_arrival=0;
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
  StarSystem * ss=_Universe->activeStarSystem();
  StarSystem::Statistics *stats=&ss->stats;
  

  float sysrel=FactionUtil::GetIntRelation(stats->system_faction,parent->faction);          
  static float lurk_time = XMLSupport::parse_float(vs_config->getVariable("AI","lurk_time","600"));
  static float select_time = XMLSupport::parse_float(vs_config->getVariable("AI","fg_nav_select_time","120"));
  static float hostile_select_time = XMLSupport::parse_float(vs_config->getVariable("AI","pirate_nav_select_time","400"));
  static int num_ships_per_roid = XMLSupport::parse_float(vs_config->getVariable("AI","num_pirates_per_asteroid_field","12"));
  bool civilian=FactionUtil::isCitizenInt(parent->faction);

  bool hostile=sysrel<0;
  bool anarchy=stats->enemycount>stats->friendlycount;
  float timehash=select_time;
  if (hostile&&!anarchy)
    timehash=hostile_select_time;
  unsigned int firstRand,thirdRand;
  float secondRand;
  if (civilian) {
    firstRand=vsrandom.genrand_int31();
    secondRand=vsrandom.uniformExc(0,1);
    thirdRand=vsrandom.genrand_int31();
  }else{
    int k = (int)(getNewTime()/timehash);// two minutes
    string key = UnitUtil::getFlightgroupName(parent);
    std::string::const_iterator start = key.begin();
    for(;start!=key.end(); start++) {
      k += (k * 128) + *start;
    }
    VSRandom choosePlace(k);
    firstRand=choosePlace.genrand_int31();
    secondRand=choosePlace.uniformExc(0,1);
    thirdRand=choosePlace.genrand_int31();
  }
  
  bool asteroidhide = (secondRand < stats->enemycount/(float)stats->friendlycount)&&(secondRand<num_ships_per_roid*stats->navs[2].size()/(float)stats->enemycount);
  bool siege=stats->enemycount>2*stats->friendlycount;//rough approx
  int whichlist=1;//friendly
  std::string fgname=UnitUtil::getFlightgroupName(parent);

  bool insys=(parent->GetJumpStatus().drive==-2)||fgname.find(insysString)!=std::string::npos;
  std::string::size_type whereconvoy=fgname.find(arrowString);
  bool convoy=(whereconvoy!=std::string::npos);
  size_t total_size=stats->navs[0].size()+stats->navs[whichlist].size();//friendly and neutral
  if (hostile) {
    if (anarchy&&!siege) {
      whichlist=2;
      total_size=stats->navs[0].size()+stats->navs[whichlist].size();//asteroids and neutrals
    }else {
      whichlist=2;
      total_size=stats->navs[whichlist].size();//just asteroids
    }
  }else if (civilian) {
    if (anarchy||siege) {
      whichlist=0;
      total_size=stats->navs[0].size();
    }else if (insys||convoy) {
      whichlist=1;
      total_size=stats->navs[1].size();//don't go to jump point
    }
  }
  
  if (hostile&&((anarchy==false&&asteroidhide==false)||total_size==0)&&civilian==false) {
    //hit and run
    Unit * a=GetRandomNav(stats->navs,firstRand);
    Unit * b=GetRandomNav(stats->navs,thirdRand);
    if (a==b) {
      b=GetRandomNav(stats->navs,thirdRand+1);
    }
    if (a!=b) {
      *otherdest=b;
      *lurk_on_arrival=lurk_time;
    }
    return a;
  }else {
    if(convoy) {
      std::string srcdst[2]={fgname.substr(0,whereconvoy),fgname.substr(whereconvoy+2)};
      if (srcdst[0]==ss->getFileName())
        srcdst[0]=srcdst[1];
      if (srcdst[1]==ss->getFileName())
        srcdst[1]=srcdst[0];      
      size_t rand8=thirdRand%8;
      if (thirdRand<2) {
        stdext::hash_map <std::string, UnitContainer>::iterator i=stats->jumpPoints.find(srcdst[thirdRand]);
        if (i!=stats->jumpPoints.end()) {
          Unit * un=i->second.GetUnit();
          if (un) {
            return un;
          }
        }else{
          total_size=stats->navs[whichlist].size()+stats->navs[0].size();//no such jump point--have to random-walk it
          //maybe one day we can incorporate some sort of route planning
        }
      }
    }
    if (total_size>0) {
      firstRand%=total_size;
      if (firstRand>=stats->navs[whichlist].size()) {
        firstRand-=stats->navs[whichlist].size();
        whichlist=0;//allows you to look for both neutral and ally lists  
      }
      return stats->navs[whichlist][firstRand].GetUnit();
    }
  }
  return NULL;
}

static Unit * ChooseNearNavPoint(Unit * parent,QVector location, float locradius) {
  Unit * candidate=NULL;
  float dist = FLT_MAX;
  Unit * un;
  NearestNavLocator nnl;
  findObjects(_Universe->activeStarSystem()->collidemap[Unit::UNIT_ONLY],
              parent->location[Unit::UNIT_ONLY],
              &nnl);
  return nnl.retval.unit;
  //DEAD CODE
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
  //END DEAD CODE
}

class FlyTo:public Orders::MoveTo {
  float creationtime;
public:
  FlyTo(const QVector &target, bool aft, bool terminating=true, float creationtime=0,int leniency=6) : MoveTo(target,aft,leniency,terminating) {this->creationtime=creationtime;}

  virtual void Execute() {
    MoveTo::Execute();
    if (done) {
      printf ("Flyto done\n");
    }
    Unit * un=NULL;
    static float mintime=XMLSupport::parse_float(vs_config->getVariable("AI","min_time_to_auto","25"));
    if (getNewTime()-creationtime>mintime) {
      if (_Universe->AccessCockpit()->autoInProgress()&&(!_Universe->AccessCockpit()->unitInAutoRegion(parent))&&(un =ChooseNearNavPoint(parent,targetlocation,0))!=NULL) {
        WarpToP(parent,un,true);
      }else {
        Unit* playa=_Universe->AccessCockpit()->GetParent();
        if (playa==NULL||playa->Target()!=parent||1) {
          WarpToP(parent,targetlocation,0,true);
        }
      }
    }
  }
};
static Vector randVector() {
  return Vector((rand()/(float)RAND_MAX)*2-1,(rand()/(float)RAND_MAX)*2-1,(rand()/(float)RAND_MAX)*2-1);
}
static void GoTo(AggressiveAI * ai, Unit * parent, const QVector &nav, float creationtime, bool boonies=false) {
  static bool can_afterburn = XMLSupport::parse_bool(vs_config->getVariable("AI","afterburn_to_no_enemies","true")); 
  Order * mt=new FlyTo(nav,can_afterburn,true,creationtime,boonies?16:6);
  Order * ch=new Orders::ChangeHeading(nav,32,.25f,true);
  ai->eraseType(Order::FACING);
  ai->eraseType(Order::MOVEMENT);
  mt->SetParent(parent);
  ch->SetParent(parent);
  ai->ReplaceOrder(mt);
  ai->EnqueueOrder(ch);
}
void AggressiveAI::ExecuteNoEnemies() {
  static float safetyspacing=XMLSupport::parse_float(vs_config->getVariable("AI","safetyspacing","25000"));	
  static float randspacingfactor=XMLSupport::parse_float(vs_config->getVariable("AI","randomspacingfactor","4"));	
  if (nav.i==0&&nav.j==0&&nav.k==0) {
    Unit * otherdest=NULL;
    Unit * dest=ChooseNavPoint (parent,&otherdest,&this->lurk_on_arrival);
   
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
      Vector unitdir=dir.Normalize();
      if (!otherdest) {
        dir=unitdir*(dest->rSize()+parent->rSize());
        if (dest->isUnit()==PLANETPTR) {
          float planetpct=UniverseUtil::getPlanetRadiusPercent();
          dir *=(planetpct+1.0f);
		  dir+=randVector()*parent->rSize()*2*randspacingfactor;
		}else {
		 dir*=2;
		 dir+=(unitdir*safetyspacing);
		 dir+=((randVector()*randspacingfactor/4)+(unitdir*randspacingfactor))*((parent->rSize() > (safetyspacing/5))?(safetyspacing/5):(parent->rSize()));
		}
      }
      
      nav=dest->Position()+dir;
      if (otherdest) {
        nav+=otherdest->Position();
        nav=nav*.5;
      }
      std::string fgname=UnitUtil::getFlightgroupName(parent);
      std::string pfullname=parent->getFullname();
      std::string dfullname=dest->getFullname();
      printf ("%s:%s %s going to %s:%s",parent->name.c_str(),pfullname.c_str(),fgname.c_str(),dest->name.c_str(),dfullname.c_str());
      if (otherdest) {
      std::string ofullname=otherdest->getFullname();
        printf (" between %s:%s\n",otherdest->name.c_str(),ofullname.c_str());
       
      }else printf("\n");

      GoTo(this,parent,nav,creationtime,otherdest!=NULL);
    }
  }else {          
    if ((nav-parent->Position()).MagnitudeSquared()<4*parent->rSize()*parent->rSize()&&lurk_on_arrival==0) {
      std::string fgname=UnitUtil::getFlightgroupName(parent);

      nav=QVector(0,0,0);
      Unit * dest =ChooseNearNavPoint(parent,parent->Position(),parent->rSize());
      if (dest) {
        if (fgname.find(insysString)==string::npos&&dest->GetDestinations().size()>0&&UniverseUtil::systemInMemory(dest->GetDestinations()[0])){
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
    }else if (lurk_on_arrival>0) {
      lurk_on_arrival-=SIMULATION_ATOM;
      //slowdown
      parent->Thrust(-parent->GetMass()*parent->UpCoordinateLevel(parent->GetVelocity())/SIMULATION_ATOM,false);
      parent->graphicOptions.InWarp=0;
      if (lurk_on_arrival<=0) {
        nav=QVector(0,0,0);
        ExecuteNoEnemies();//select new place to go
      }
      // have to do something while here.
      
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
  extern double aggfire;
  jump_time_check++;//just so we get a nicely often wrapping var;
  jump_time_check%=5;
  Flightgroup * fg=parent->getFlightgroup();
  //ReCommandWing(fg);
  double firetime=queryTime();
  FireAt::Execute();
  aggfire+=queryTime()-firetime;
  static bool resistance_to_side_movement=XMLSupport::parse_bool(vs_config->getVariable("AI","resistance_to_side_movement","false"));
  if(resistance_to_side_movement) {
    Vector p,q,r;
    parent->GetOrientation(p,q,r);
    float forwardness=parent->Velocity.Dot(r);
    Vector countervelocity=-parent->Velocity;
    Vector counterforce=-parent->NetForce;
    float forceforwardness=parent->NetForce.Dot(r);    
    if (forceforwardness>0){
      counterforce=forceforwardness*r-parent->NetForce;
    }
    if (forwardness>0) {
      countervelocity=forwardness*r-parent->Velocity;
    }
    static float resistance_percent=XMLSupport::parse_float(vs_config->getVariable("AI","resistance_to_side_movement_percent",".01"));
    static float force_resistance_percent=XMLSupport::parse_float(vs_config->getVariable("AI","resistance_to_side_force_percent","1"));
    parent->Velocity+=countervelocity*resistance_percent;
    parent->NetForce+=counterforce*force_resistance_percent;
    counterforce=-parent->NetLocalForce;
    counterforce.k=0;
    parent->NetLocalForce+=counterforce*force_resistance_percent;
    
  }
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
      //eraseType (Order::FACING);
      //eraseType (Order::MOVEMENT);      
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


