#include "communication.h"

FSM::FSM (const char * filename) {
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
  nodes.push_back (Node("welcome to cachunkcachunk.com",0));
  nodes.push_back (Node("I love you!",.1));
  nodes.push_back (Node("J00 0wnz m3",.08));
  nodes.push_back (Node("You are cool!",.06));
  nodes.push_back (Node("You are nice!",.05));
  nodes.push_back (Node("Ya you're naled! NALED PAL!",-.02));
  nodes.push_back (Node("i 0wnz j00",-.08));
  nodes.push_back (Node("I hate you!",-.1));


  nodes.push_back (Node("Prepare To Be Searched. Maintain Speed and Course.",0));
  nodes.push_back (Node("No contraband detected: You may proceed.",0));
  nodes.push_back (Node("Contraband detected! All units close and engage!",0));
  nodes.push_back (Node("Your Course is deviating! Maintain Course!",0));
  nodes.push_back (Node("Request Clearence To Land.",0));
  nodes.push_back (Node("*hit*",-.2));
  vector <unsigned int> edges;
  unsigned int i;
  for (i=0;i<nodes.size()-6;i++) {
    edges.push_back (i);
  }
  for (i=0;i<nodes.size();i++) {
    nodes[i].edges = edges;
  }
  
}
int FSM::GetContrabandInitiateNode() {
  return nodes.size()-6;
}
int FSM::GetContrabandUnDetectedNode() {
  return nodes.size()-5;
}
int FSM::GetContrabandDetectedNode() {
  return nodes.size()-4;
}
int FSM::GetContrabandWobblyNode() {
  return nodes.size()-3;
}


int FSM::GetRequestLandNode () {
  return nodes.size()-2;
}
int FSM::GetHitNode () {
  return nodes.size()-1;
}

int FSM::getDefaultState (float relationship) const{
  return 0;
}
std::string FSM::GetEdgesString (int curstate) {
  std::string retval="\n";
  for (unsigned int i=0;i<nodes[curstate].edges.size();i++) {
    retval+= tostring ((int)((i+1)%10))+"."+nodes[nodes[curstate].edges[i]].message+"\n";
  }
  retval+= "0 Request Docking Clearence";
  return retval;
}
float FSM::getDeltaRelation (int prevstate, int current_state) const{
  return nodes[current_state].messagedelta;
}

void CommunicationMessage::Init (Unit * send, Unit * recv, Animation * ani) {
  this->ani = ani;
  fsm = _Universe->GetConversation (send->faction,recv->faction);
  sender.SetUnit (send);
  this->prevstate=this->curstate = fsm->getDefaultState(_Universe->GetRelation(send->faction,recv->faction));
}

void CommunicationMessage::SetCurrentState (int msg) {
  curstate = msg;
}

CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int messagechoice, Animation * ani) {
  Init (send,recv,ani);
  prevstate=fsm->getDefaultState (_Universe->GetRelation (send->faction,recv->faction));
  if (fsm->nodes[prevstate].edges.size()) {
    curstate = fsm->nodes[prevstate].edges[messagechoice%fsm->nodes[prevstate].edges.size()];
  }
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int laststate, int thisstate, Animation * ani) {
  Init (send,recv,ani);
  prevstate=laststate;
  curstate = thisstate;
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv,Animation * ani) {
  Init (send,recv,ani);
}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, const CommunicationMessage &prevstate, int curstate, Animation * ani) {
  Init (send,recv,ani);
  this->prevstate = prevstate.curstate;
  if (fsm->nodes[this->prevstate].edges.size()) {
    this->curstate = fsm->nodes[this->prevstate].edges[curstate%fsm->nodes[this->prevstate].edges.size()];
  }

}
