#include "comm_ai.h"
#include "faction_generic.h"
#include "communication.h"
#include "cmd/collection.h"
#include "gfx/cockpit_generic.h"
#include "cmd/images.h"
#include "configxml.h"
#include "vs_globals.h"
#include "cmd/script/flightgroup.h"
CommunicatingAI::CommunicatingAI (int ttype, int stype,  float rank, float mood, float anger,float appeas,  float moodswingyness, float randomresp) :Order (ttype,stype),anger(anger), appease(appeas), moodswingyness(moodswingyness),randomresponse (randomresp),mood(mood),rank(rank) {
  comm_face=NULL;
  if (rank>665&&rank<667) {
    static float ran = XMLSupport::parse_float(vs_config->getVariable ("AI","DefaultRank",".01"));
    this->rank = ran;
  }
  if (appease>665&&appease<667) {
    static float appeas = XMLSupport::parse_float(vs_config->getVariable ("AI","EaseToAppease",".5"));
    this->appease = appeas;    
  }
  if ((anger>665&&anger<667)||(anger >-667&&anger <-665)) {
    static float ang = XMLSupport::parse_float(vs_config->getVariable ("AI","EaseToAnger","-.5"));
    this->anger = ang;
  }
  if (moodswingyness>665&&moodswingyness<667) {
    static float ang1 = XMLSupport::parse_float(vs_config->getVariable ("AI","MoodSwingLevel",".2"));
    this->moodswingyness = ang1;
  }
  if (randomresp>665&&moodswingyness<667) {
    static float ang2 = XMLSupport::parse_float(vs_config->getVariable ("AI","RandomResponseRange",".8"));
    this->randomresponse = ang2;
  }
}
vector <Animation *> *CommunicatingAI::getCommFaces(unsigned char &sex) {
  sex = this->sex;
  return comm_face;
}

void CommunicatingAI::SetParent (Unit * par) {
  Order::SetParent(par);
  comm_face = FactionUtil::GetRandAnimation(par->faction,sex);
}
int CommunicatingAI::selectCommunicationMessageMood (CommunicationMessage &c, float mood) {

  Unit * targ = c.sender.GetUnit();
  float relationship=0;

  if (targ) {
    relationship= GetEffectiveRelationship(targ);
    mood+=(1-randomresponse)*relationship;
  }
  //breaks stuff
  if (!(c.curstate<c.fsm->GetUnDockNode())) {
    c.curstate = c.fsm->getDefaultState(relationship);//hijack the current state
  }
  return c.fsm->getCommMessageMood (c.curstate,mood,randomresponse,relationship);

}
using std::pair;

float CommunicatingAI::getAnger(const Unit * target)const {
  relationmap::const_iterator i=effective_relationship.find(target);
  float rel=0;
  if (i!=effective_relationship.end()) {
    rel = (*i).second;
  }
  return rel;
}
float CommunicatingAI::GetEffectiveRelationship (const Unit * target)const {
  return Order::GetEffectiveRelationship (target)+getAnger(target);
}
static void AllUnitsCloseAndEngage(Unit * un, int faction) {
	Unit * ally;
	static float contraband_assist_range = XMLSupport::parse_float (vs_config->getVariable("physics","contraband_assist_range","50000"));
	for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
		(ally = *i)!=NULL;
		++i) {
		//Vector loc;

		if (ally->faction==faction) {
			//if (ally->InRange (un,loc,true)) {
				if ((ally->Position()-un->Position()).Magnitude()<contraband_assist_range) {
					Flightgroup * fg = ally->getFlightgroup();
					if (fg) {
						if (fg->directive.empty()?true:toupper(*fg->directive.begin())!=*fg->directive.begin()) {
							ally->Target (un);
							ally->TargetTurret (un);

							fg->directive=string("a");//attack my target (of leader)
						}else {
							ally->Target (un);
							ally->TargetTurret (un);

						}
					}
				}
			//}
		}
	}
}
void CommunicatingAI::TerminateContrabandSearch(bool contraband_detected) {
  //reports success or failure
  Unit * un;
  if ((un=contraband_searchee.GetUnit())) {
    CommunicationMessage c(parent,un,comm_face,sex);
    if (contraband_detected) {
      c.SetCurrentState(c.fsm->GetContrabandDetectedNode(),comm_face,sex);
      static int numHitsPerContrabandFail=3;
      GetMadAt (un,numHitsPerContrabandFail);
      AllUnitsCloseAndEngage(un,parent->faction);

    }else {
      c.SetCurrentState(c.fsm->GetContrabandUnDetectedNode(),comm_face,sex);      
    }
	Order * o = un->getAIState();
	if (o)
		o->Communicate(c);
  }
  contraband_searchee.SetUnit(NULL);

}
void CommunicatingAI::GetMadAt (Unit * un, int numHitsPerContrabandFail) {

      CommunicationMessage hit (un,parent,NULL,0);
      hit.SetCurrentState(hit.fsm->GetHitNode(),comm_face,sex);
      for (         int i=0;i<numHitsPerContrabandFail;i++) {
		  parent->getAIState()->Communicate(hit);
      }
}

static bool InList (std::string item, Unit * un) {
  if (un) {
  for (unsigned int i=0;i<un->numCargo();i++) {
    if (item==un->GetCargo(i).content) {
      if (un->GetCargo(i).quantity>0)
	return true;
    }
  }
  }
  return false;
}
void CommunicatingAI::UpdateContrabandSearch () {
	Unit * u = contraband_searchee.GetUnit();
	if (u) {
		if (which_cargo_item<(int)u->numCargo()) {
			if (u->GetCargo(which_cargo_item).quantity>0) {
				std::string item = u->GetManifest (which_cargo_item++,parent,SpeedAndCourse);
				
				static float speed_course_change = XMLSupport::parse_float (vs_config->getVariable ("AI","PercentageSpeedChangeToStopSearch","1"));
				if (u->CourseDeviation(SpeedAndCourse,u->GetVelocity())>speed_course_change) {
					CommunicationMessage c(parent,u,comm_face,sex);
					c.SetCurrentState(c.fsm->GetContrabandWobblyNode(),comm_face,sex);
					Order * o;
					if ((o=u->getAIState()))
						o->Communicate (c);
					GetMadAt(u,1);
					SpeedAndCourse=u->GetVelocity();
				}
				if (InList (item,FactionUtil::GetContraband(parent->faction))) {
					TerminateContrabandSearch(true);
				}
			}
		}else {
			TerminateContrabandSearch(false);
			
		}
	}
}
void CommunicatingAI::InitiateContrabandSearch (float playaprob, float targprob) {
  Unit *u= GetRandomUnit (playaprob,targprob);
  if (u) {
    Unit * un =FactionUtil::GetContraband (parent->faction);
    if (un) {
    if (un->numCargo()>0) {
    Unit * v;
    if ((v=contraband_searchee.GetUnit())) {
      if (v==u) {
	return;
      }
      TerminateContrabandSearch(false);
    }
    contraband_searchee.SetUnit (u);
    SpeedAndCourse = u->GetVelocity();
    CommunicationMessage c(parent,u,comm_face,sex);
    c.SetCurrentState(c.fsm->GetContrabandInitiateNode(),comm_face,sex);
	if (u->getAIState())
		u->getAIState()->Communicate (c);
    which_cargo_item = 0;
    }
    }
  }
}

void CommunicatingAI::AdjustRelationTo (Unit * un, float factor) {
  Order::AdjustRelationTo(un,factor);

  //now we do our magik  insert 0 if nothing's there... and add on our faction
  relationmap::iterator i = effective_relationship.insert (pair<const Unit*,float>(un,0)).first;
  bool abovezero=(*i).second+FactionUtil::GetIntRelation (parent->faction,un->faction)>=0;
  if (!abovezero) {
    static float slowrel=XMLSupport::parse_float (vs_config->getVariable ("AI","SlowDiplomacyForEnemies",".25"));
    factor *=slowrel;
  }
  FactionUtil::AdjustIntRelation (parent->faction,un->faction,factor,rank);  
  (*i).second+=factor;
  if ((*i).second<anger||(parent->Target()==NULL&&(*i).second+Order::GetEffectiveRelationship (un)<0)) {
	  if (parent->Target()==NULL||(parent->getFlightgroup()==NULL||parent->getFlightgroup()->directive.find(".")==string::npos)){
		  parent->Target(un);//he'll target you--even if he's friendly
		  parent->TargetTurret(un);//he'll target you--even if he's friendly
	  }
  } else if ((*i).second>appease) {
    if (parent->Target()==un) {
		if (parent->getFlightgroup()==NULL||parent->getFlightgroup()->directive.find(".")==string::npos) {
			parent->Target(NULL);
			parent->TargetTurret(NULL);//he'll target you--even if he's friendly
		}

    }
  }
  mood+=factor*moodswingyness;
}

Unit * CommunicatingAI::GetRandomUnit (float playaprob, float targprob) {
  float a =rand ();
  Unit * target=NULL;
  if (a<RAND_MAX*playaprob&&_Universe->AccessCockpit()->GetParent()!=parent) {
    target = _Universe->AccessCockpit()->GetParent();
  }
  if (a>RAND_MAX*(1-targprob)) {
    target = parent->Target();
  }
  if (target!=NULL) {
  if ((!parent->InRange (target))) {
    target=NULL;
  }
  }
  if (target==NULL) {
    for (un_iter ui=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (*ui)!=NULL; ++ui) {
      if (parent->InRange ((*ui))) {
	target = *ui;
      }
    }
  }
  return target;
}
void CommunicatingAI::RandomInitiateCommunication (float playaprob, float targprob) {
  Unit * target = GetRandomUnit(playaprob,targprob);
  if (target!=NULL) {
    for (std::list<CommunicationMessage *>::iterator i=messagequeue.begin();i!=messagequeue.end();i++) {   
      Unit * un=(*i)->sender.GetUnit();
      if (un==target) {
	return;
      }
    }
    //ok we're good to put a default msg in the queue as a fake message;
    messagequeue.push_back (new CommunicationMessage (target,this->parent,comm_face,sex));
  }
}

int CommunicatingAI::selectCommunicationMessage (CommunicationMessage &c,Unit * un) {
  if (0&&mood==0) {
    FSM::Node * n = c.getCurrentState ();  
    if (n)
      return rand()%n->edges.size();
    else
      return 0;
  }else {
    return selectCommunicationMessageMood (c,.5*mood+.5*getAnger (un));
  }
}

void CommunicatingAI::ProcessCommMessage (CommunicationMessage &c) {
  if (messagequeue.back()->curstate<messagequeue.back()->fsm->GetUnDockNode()) {

  Order::ProcessCommMessage(c);
  FSM *tmpfsm = c.fsm;
  Unit * targ = c.sender.GetUnit();
  if (targ) {
    c.fsm  =FactionUtil::GetConversation (parent->faction,targ->faction);
    FSM::Node * n = c.getCurrentState ();
    if (n) {
      if (n->edges.size()) {
        Unit * un = c.sender.GetUnit();
        if (un) {
          int b = selectCommunicationMessage (c,un);
          Order * o = un->getAIState();
          if (o)
            o->Communicate (CommunicationMessage (parent,un,c,b,comm_face,sex));
        }
      }
    }
    c.fsm=tmpfsm;    
  }
  }
}
CommunicatingAI::~CommunicatingAI() {

}
