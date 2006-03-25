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
  bool already_communicated=false;
  for (list<CommunicationMessage *>::iterator ii=messagequeue.begin();ii!=messagequeue.end();ii++) {
    un=(*ii)->sender.GetUnit();
    bool thisissender=( un==newC->sender.GetUnit());
    if (un==NULL||thisissender) {
      delete (*ii);
      if (thisissender) already_communicated=true;
      if ((ii=messagequeue.erase (ii))==messagequeue.end())
		  break;
    }
  }
  if ((un=newC->sender.GetUnit())) {
	  if (un!=parent) {
		static bool talk_more_helps=XMLSupport::parse_bool(vs_config->getVariable("AI","talking_faster_helps","true"));
		static float talk_factor=XMLSupport::parse_float(vs_config->getVariable("AI","talk_relation_factor",".5"));
	    if (talk_more_helps||!already_communicated){
	      AdjustRelationTo (un,newC->getDeltaRelation()*talk_factor);
	    }
	    messagequeue.push_back (newC);
	  }
  }
}

void Order::ProcessCommMessage(CommunicationMessage & c) {
  
}
float Order::GetEffectiveRelationship (const Unit * target)const {
  float staticrel = FactionUtil::GetIntRelation (parent->faction,target->faction);
  for (unsigned int i=0;i<suborders.size();i++) {
    float effrel = suborders[i]->GetEffectiveRelationship(target);
    if (effrel !=staticrel) {
      return effrel;
    }
  }
  return staticrel;
}
void Order::ProcessCommunicationMessages(float AICommresponseTime, bool RemoveMessageProcessed) {
  float time = AICommresponseTime/SIMULATION_ATOM;
  if (time<=.001)
    time+=.001;
  if (!messagequeue.empty()) {
    bool cleared=false;
	if (messagequeue.back()->curstate==messagequeue.back()->fsm->GetRequestLandNode()) {
	  cleared=true;
          RemoveMessageProcessed=true;
	  Unit * un=messagequeue.back()->sender.GetUnit();
	  if (un) {
            CommunicationMessage c(parent,un,NULL,0);
            
            if (GetEffectiveRelationship(un)>=0||(parent->getFlightgroup()&&parent->getFlightgroup()->name=="Base")) {
	      parent->RequestClearance (un);
	      c.SetCurrentState (c.fsm->GetAbleToDockNode(),NULL,0);
            }else {
	      c.SetCurrentState (c.fsm->GetUnAbleToDockNode(),NULL,0);
            }
			Order * o  = un->getAIState();
			if (o)
				o->Communicate (c);
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
  //   return _Universe->GetRandCommAnimation(parent->faction,parent,sex);
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
    ani = FactionUtil::GetRandCommAnimation(parent->faction,parent,sex);
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
