#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "cmd/unit.h"
class FSM {
  
 public:
  FSM (const char * filename) {
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
  }
  struct state {
    int whichstate;
    state (int a) {whichstate=a;}
  };
  float getDeltaRelation (FSM::state previous_state, FSM::state current_state) {
    return 0;
  }
};
class CommunicationMessage {
  FSM *currentstate;//the finite state that this communcation stage is in
  FSM::state previous_state;
  FSM::state current_state;
  UnitContainer sender;
 public:
  CommunicationMessage(Unit * send, Unit * recv, int messagechose);
  


};
#endif
