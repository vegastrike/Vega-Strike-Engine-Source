#include "comm_ai.h"
#include "communication.h"
CommunicatingAI::CommunicatingAI (int ttype) :Order (ttype) {
  
}

int CommunicatingAI::selectCommunicationMessage (CommunicationMessage &c) {
  FSM::Node * n = c.getCurrentState ();  
  if (n)
    return rand()%n->edges.size();
  else
    return 0;
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
void CommunicatingAI::AdjustRelationTo (Unit * un, float factor) {
  Order::AdjustRelationTo(un,factor);
  //now we do our magik
}
CommunicatingAI::~CommunicatingAI() {

}
