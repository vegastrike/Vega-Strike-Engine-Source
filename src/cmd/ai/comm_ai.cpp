#include "comm_ai.h"
#include "communication.h"
#include "cmd/collection.h"
#include "gfx/cockpit.h"
#include "cmd/images.h"
#include "config_xml.h"
#include "vs_globals.h"
CommunicatingAI::CommunicatingAI (int ttype, float anger, float moodswingyness, float randomresp, float mood) :Order (ttype),anger(anger),moodswingyness(moodswingyness),randomresponse (randomresp),mood(mood) {
  comm_face=NULL;
}
void CommunicatingAI::SetParent (Unit * par) {
  Order::SetParent(par);
  comm_face = _Universe->GetRandAnimation(par->faction);
}
static float sq (float i) {return i*i;}
bool nonneg (float i) {return i>=0;}
int CommunicatingAI::selectCommunicationMessageMood (CommunicationMessage &c, float mood) {
  FSM::Node * n = c.getCurrentState ();  
  mood+=-randomresponse+2*randomresponse*((float)rand())/RAND_MAX;
  Unit * targ = c.sender.GetUnit();
  if (targ) {
    mood+=(1-randomresponse)*_Universe->GetRelation(parent->faction,targ->faction);
  }
  
  int choice=0;
  float bestchoice=4;
  bool fitmood=false;
  for (unsigned i=0;i<n->edges.size();i++) {
    float md = c.fsm->nodes[n->edges[i]].messagedelta;
    bool newfitmood=nonneg(mood)==nonneg(md);
    if ((!fitmood)||newfitmood) {
      float newbestchoice=sq(md-mood);
      if ((newbestchoice<bestchoice)||(fitmood==false&&newfitmood==true)) {
	fitmood=newfitmood;
	choice =i;
	bestchoice = newbestchoice;
      }
    }
  }
  return choice;
}
using std::pair;
float CommunicatingAI::GetEffectiveRelationship (const Unit * target)const {
  relationmap::const_iterator i=effective_relationship.find(target);
  float rel=0;
  if (i!=effective_relationship.end()) {
    rel = (*i).second;
  }
  return _Universe->GetRelation (parent->faction,target->faction)+rel;
}

void CommunicatingAI::TerminateContrabandSearch(bool contraband_detected) {
  //reports success or failure
  Unit * un;
  if ((un=contraband_searchee.GetUnit())) {
    CommunicationMessage c(parent,un,comm_face);
    if (contraband_detected) {
      c.SetCurrentState(c.fsm->GetContrabandDetectedNode());
      static int numHitsPerContrabandFail=3;
      GetMadAt (un,numHitsPerContrabandFail);
    }else {
      c.SetCurrentState(c.fsm->GetContrabandUnDetectedNode());      
    }
    un->getAIState()->Communicate(c);
  }
  contraband_searchee.SetUnit(NULL);

}
void CommunicatingAI::GetMadAt (Unit * un, int numHitsPerContrabandFail) {

      CommunicationMessage hit (un,parent,NULL);
      hit.SetCurrentState(hit.fsm->GetHitNode());
      for (         int i=0;i<numHitsPerContrabandFail;i++) {
	parent->getAIState()->Communicate(hit);
      }
}

static bool InList (std::string item, Unit * un) {
  if (un) {
  for (unsigned int i=0;i<un->numCargo();i++) {
    if (item==un->GetCargo(i).content) {
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
      std::string item = u->GetManifest (which_cargo_item++,parent,SpeedAndCourse);
      static float speed_course_change = XMLSupport::parse_float (vs_config->getVariable ("AI","PercentageSpeedChangeToStopSearch","1"));
      if (u->CourseDeviation(SpeedAndCourse,u->GetVelocity())>speed_course_change) {
	CommunicationMessage c(parent,u,comm_face);
	c.SetCurrentState(c.fsm->GetContrabandWobblyNode());
	u->getAIState()->Communicate (c);
	GetMadAt(u,1);
	SpeedAndCourse=u->GetVelocity();
      }
      if (InList (item,_Universe->GetContraband(parent->faction))) {
	TerminateContrabandSearch(true);
      }
    }else {
      TerminateContrabandSearch(false);

    }
  }
}
void CommunicatingAI::InitiateContrabandSearch (float playaprob, float targprob) {
  Unit *u= GetRandomUnit (playaprob,targprob);
  if (u) {
    Unit * v;
    if ((v=contraband_searchee.GetUnit())) {
      if (v==u) {
	return;
      }
      TerminateContrabandSearch(false);
    }
    contraband_searchee.SetUnit (u);
    SpeedAndCourse = u->GetVelocity();
    CommunicationMessage c(parent,u,comm_face);
    c.SetCurrentState(c.fsm->GetContrabandInitiateNode());
    u->getAIState()->Communicate (c);
    which_cargo_item = 0;
  }
}

void CommunicatingAI::AdjustRelationTo (Unit * un, float factor) {
  Order::AdjustRelationTo(un,factor);
  //now we do our magik  insert 0 if nothing's there... and add on our faction
  relationmap::iterator i = effective_relationship.insert (pair<const Unit*,float>(un,0)).first;
  (*i).second+=factor;
  if ((*i).second<anger) {
    parent->Target(un);//he'll target you--even if he's friendly
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
  Vector localcoord;
  if (target==NULL?true:(!parent->InRange (target,localcoord))) {
    for (un_iter ui=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (*ui)!=NULL; ++ui) {
      if (parent->InRange ((*ui),localcoord)) {
	target = *ui;
      }
    }
  }
  return target;
}
void CommunicatingAI::RandomInitiateCommunication (float playaprob, float targprob) {
  Unit * target = GetRandomUnit(playaprob,targprob);
  if (target!=NULL) {
    for (unsigned int i=0;i<messagequeue.size();i++) {   
      Unit * un=messagequeue[i]->sender.GetUnit();
      if (un==target) {
	return;
      }
    }
    //ok we're good to put a default msg in the queue as a fake message;
    messagequeue.push_back (new CommunicationMessage (target,this->parent,comm_face));
  }
}

int CommunicatingAI::selectCommunicationMessage (CommunicationMessage &c) {
  if (mood==0) {
    FSM::Node * n = c.getCurrentState ();  
    if (n)
      return rand()%n->edges.size();
    else
      return 0;
  }else {
    return selectCommunicationMessageMood (c,mood);
  }
}

void CommunicatingAI::ProcessCommMessage (CommunicationMessage &c) {
  Order::ProcessCommMessage(c);
  FSM::Node * n = c.getCurrentState ();
  if (n) {
     if (n->edges.size()) {
      int b = selectCommunicationMessage (c);
      Unit * un = c.sender.GetUnit();
      if (un) {
	un->getAIState()->Communicate (CommunicationMessage (parent,un,c,b,comm_face));
      }
    }
  }
}
CommunicatingAI::~CommunicatingAI() {

}
