#include "order.h"
#include "communication.h"

void Order::AdjustRelationTo (Unit * un, float factor) {
  //virtual stub function
}

void Order::Communicate (const CommunicationMessage &c) {
  int completed=0;
  unsigned int i=0;
  CommunicationMessage * newC = new CommunicationMessage (c);
  for (i=0;i<suborders.size();i++) {
    if ((completed& ((suborders[i])->getType()&(MOVEMENT|FACING|WEAPON)))==0) {
      (suborders[i])->Communicate (*newC);
      completed|=(suborders[i])->getType();
    }
  }
  
  Unit * un;
  for (list<CommunicationMessage *>::iterator ii=messagequeue.begin();ii!=messagequeue.end();ii++) {
    un=(*ii)->sender.GetUnit();
    if (un==NULL||un==newC->sender.GetUnit()) {
      delete (*ii);
      ii=messagequeue.erase (ii);
    }
  }
  if ((un=newC->sender.GetUnit())) {
    AdjustRelationTo (un,newC->getDeltaRelation());
    messagequeue.push_back (newC);
  }
}

void Order::ProcessCommMessage(CommunicationMessage & c) {
  
}
float Order::GetEffectiveRelationship (const Unit * target)const {
  return _Universe->GetRelation (parent->faction,target->faction);
}
void Order::ProcessCommunicationMessages(float AICommresponseTime, bool RemoveMessageProcessed) {
  float time = AICommresponseTime/SIMULATION_ATOM;
  if (time<=.001)
    time+=.001;
  if (!messagequeue.empty()) {
    bool cleared=false;
	if (messagequeue.back()->curstate==messagequeue.back()->fsm->GetRequestLandNode()) {
	  cleared=true;
	  Unit * un=messagequeue.back()->sender.GetUnit();
	  if (un) {
	    if (GetEffectiveRelationship(un)>=0) {
	      parent->RequestClearance (un);
	    }
	  }  
	}
	if (cleared||(((float)rand())/RAND_MAX)<(1/time)) {
	  
	  FSM::Node *n;
	  if ((n=messagequeue.back()->getCurrentState())) {
	    ProcessCommMessage(*messagequeue.back());
	  }
	  if (RemoveMessageProcessed) {
	    delete messagequeue.back();
	    messagequeue.pop_back();
	  }else {
	    messagequeue.push_front (messagequeue.back());
	    messagequeue.pop_back();
	  }
	}
  }
}
