#include "communication.h"

FSM::FSM (const char * filename) {
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
  nodes.push_back (Node("welcome to cachunkcachunk.com",0));
  nodes.push_back (Node("I love you!",.1));
  nodes.push_back (Node("J00 0wnz m3",.08));
  nodes.push_back (Node("You are cool!",.06));
  nodes.push_back (Node("You are nice!",.05));
  nodes.push_back (Node("Ya you're naled! NALED PAL!",-.02));
  nodes.push_back (Node("i 0wnz j00, b17c#",-.08));
  nodes.push_back (Node("Fuck you!",-.1));
  nodes.push_back (Node("Request Clearence To Land!",0));
  vector <unsigned int> edges;
  for (unsigned int i=0;i<nodes.size();i++) {
    edges.push_back (i);
  }
  for (unsigned int i=0;i<nodes.size();i++) {
    nodes[i].edges = edges;
  }
  
}

int FSM::getDefaultState (float relationship) const{
  return 0;
}
std::string FSM::GetEdgesString (int curstate) {
  std::string retval="\n";
  for (unsigned int i=0;i<nodes[curstate].edges.size();i++) {
    retval+= tostring ((int)((i+1)%10))+"."+nodes[nodes[curstate].edges[i]].message+"\n";
  }
  return retval;
}
float FSM::getDeltaRelation (int prevstate, int current_state) const{
  return nodes[current_state].messagedelta;
}

void CommunicationMessage::Init (Unit * send, Unit * recv) {
  fsm = _Universe->GetConversation (send->faction,recv->faction);
  sender.SetUnit (send);
  this->prevstate=this->curstate = fsm->getDefaultState(_Universe->GetRelation(send->faction,recv->faction));
}



CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int messagechoice) {
  Init (send,recv);
  prevstate=fsm->getDefaultState (_Universe->GetRelation (send->faction,recv->faction));
  if (fsm->nodes[prevstate].edges.size()) {
    curstate = fsm->nodes[prevstate].edges[messagechoice%fsm->nodes[prevstate].edges.size()];
  }
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int laststate, int thisstate) {
  Init (send,recv);
  prevstate=laststate;
  curstate = thisstate;
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv) {
  Init (send,recv);
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, const CommunicationMessage &prevstate, int curstate) {
  Init (send,recv);
  this->prevstate = prevstate.curstate;
  if (fsm->nodes[this->prevstate].edges.size()) {
    this->curstate = fsm->nodes[this->prevstate].edges[curstate%fsm->nodes[this->prevstate].edges.size()];
  }

}
