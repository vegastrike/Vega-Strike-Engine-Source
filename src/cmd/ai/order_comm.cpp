#include "order.h"
#include "communication.h"

void Order::AdjustRelationTo (Unit * un, float factor) {
  //virtual stub function
}

void Order::Communicate (const CommunicationMessage &c) {
  if (this==NULL)
    return;
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
	  if (un!=parent) {
		AdjustRelationTo (un,newC->getDeltaRelation());
		messagequeue.push_back (newC);
	  }
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
            CommunicationMessage c(parent,un,NULL,0);
            
            if (GetEffectiveRelationship(un)>=0) {
	      parent->RequestClearance (un);
	      c.SetCurrentState (c.fsm->GetAbleToDockNode(),NULL,0);
            }else {
	      c.SetCurrentState (c.fsm->GetUnAbleToDockNode(),NULL,0);
            }
	    un->getAIState()->Communicate (c);
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
std::vector <Animation *>* Order::getCommFaces (unsigned char &sex) {
  std::vector <Animation *>* ani = NULL;
  //   return _Universe->GetRandAnimation(parent->faction,sex);
  for (unsigned int i=0;i<suborders.size();i++) {
    ani = suborders[i]->getCommFaces(sex);
    if (ani!=NULL) {
      return ani;
    }
  }
  return NULL;
}
extern float myroundclamp(float i);
Animation * Order::getCommFace(float mood, unsigned char & sex) {
  vector <Animation *> *ani = getCommFaces (sex);
  if (ani==NULL) {
    ani = _Universe->GetRandAnimation(parent->faction,sex);
    if (ani==NULL) {
      return NULL;
    }
  }
  if (ani->size()==0) {
    return NULL;
  }
  mood+=.1;
  mood*=(ani->size())/.2;
  unsigned int index=(unsigned int)myroundclamp(floor(mood));
  if (index>=ani->size()) {
    index=ani->size()-1;
  }
  return (*ani)[index];
}
