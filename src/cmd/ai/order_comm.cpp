#include "order.h"
#include "communication.h"

void Order::AdjustRelationTo (Unit * un, float factor) {
  //virtual stub function
}

void Order::Communicate (CommunicationMessage &c) {
  int completed=0;
  unsigned int i=0;
  for (i=0;i<suborders.size();i++) {
    if ((completed& ((suborders[i])->getType()&(MOVEMENT|FACING|WEAPON)))==0) {
      (suborders[i])->Communicate (c);
      completed|=(suborders[i])->getType();
    }
  }
  
  Unit * un;
  for (unsigned int i=0;i<messagequeue.size();i++) {

    un=messagequeue[i]->sender.GetUnit();
    if (un==NULL||un==c.sender.GetUnit()) {
      messagequeue.erase (messagequeue.begin()+i);
      i--;
    }
  }
  if ((un=c.sender.GetUnit())) {
    AdjustRelationTo (un,c.getDeltaRelation());
    messagequeue.push_back (new CommunicationMessage (c));
  }
}

void Order::ProcessCommMessage(CommunicationMessage & c) {
  
}

void Order::ProcessCommunicationMessages() {
  const float AICommresponseTime=5;
  float time = AICommresponseTime/SIMULATION_ATOM;
  if (time<=.001)
    time+=.001;
  if ((((float)rand())/RAND_MAX)<(1/time)) {
    if (messagequeue.size()) {
      ProcessCommMessage(*messagequeue.back());
      delete messagequeue.back();
      messagequeue.pop_back();
    }
  }
}
