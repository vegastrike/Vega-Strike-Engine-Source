#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "cmd/unit.h"
class FSM {
 public:
  struct Node {
    std::string message;
    float messagedelta;
    vector <unsigned int> edges;
    Node (const std::string &message, float messagedel): message(message),messagedelta(messagedel){}
  };
  vector <Node> nodes;
  FSM (const char * filename);
  std::string GetEdgesString (int curstate);
  float getDeltaRelation (int prevstate, int curstate) const;
  int getDefaultState (float relationship) const;
};
class CommunicationMessage {
  void Init (Unit * send, Unit * recv);
 public:
  FSM *fsm;//the finite state that this communcation stage is in
  int prevstate;
  int curstate;
  UnitContainer sender;
  CommunicationMessage(Unit * send, Unit * recv);
  CommunicationMessage(Unit * send, Unit * recv, int prevvstate=0);
  CommunicationMessage(Unit * send, Unit * recv, const  CommunicationMessage &prevsvtate);
  void SetCurrentState(int message);
  FSM::Node * getCurrentState() {return &fsm->nodes[curstate];}

  const vector <FSM::Node> &GetPossibleState () const;
  float getDeltaRelation()const {return fsm->getDeltaRelation (prevstate,curstate);}
};
#endif
