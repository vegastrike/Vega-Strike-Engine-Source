#include "comm_ai.h"
#include "communication.h"
#include "cmd/collection.h"
#include "gfx/cockpit.h"

CommunicatingAI::CommunicatingAI (int ttype, float anger, float moodswingyness, float randomresp, float mood) :Order (ttype),anger(anger),moodswingyness(moodswingyness),randomresponse (randomresp),mood(mood) {
  
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

void CommunicatingAI::RandomInitiateCommunication (float playaprob, float targprob) {
  float a =rand ();
  Unit * target=NULL;
  if (a<RAND_MAX*playaprob&&_Universe->AccessCockpit()->GetParent()!=parent) {
    target = _Universe->AccessCockpit()->GetParent();
  }
  if (a>RAND_MAX*(1-targprob)) {
    target = parent->Target();
  }
  if (target==NULL) {
    for (un_iter ui=_Universe->activeStarSystem()->getUnitList().createIterator();
	 (*ui)!=NULL; ++ui) {
      Vector localcoord;
      if (parent->InRange ((*ui),localcoord)) {
	target = *ui;
      }
    }
  }
  if (target!=NULL) {
    for (unsigned int i=0;i<messagequeue.size();i++) {   
      Unit * un=messagequeue[i]->sender.GetUnit();
      if (un==target) {
	return;
      }
    }
    //ok we're good to put a default msg in the queue as a fake message;
    messagequeue.push_back (new CommunicationMessage (target,this->parent));
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
	un->getAIState()->Communicate (CommunicationMessage (parent,un,c,b));
      }
    }
  }
}
CommunicatingAI::~CommunicatingAI() {

}
