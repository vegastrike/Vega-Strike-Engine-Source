#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "cmd/unit_generic.h"

class FSM {
 protected:

 public:
  struct Node {
    std::string message;
    vector <int> sound;//one sound for each sex
    float messagedelta;
    vector <unsigned int> edges;
    int GetSound (unsigned char sex);
    void AddSound (int sound, unsigned char sex);
    Node (const std::string &message, float messagedel): message(message),messagedelta(messagedel){}
  };
  vector <Node> nodes;
  FSM (const char * filename);
  void LoadXML(const char * factionfile);  
  void beginElement(const string &name, const AttributeList attributes);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  std::string GetEdgesString (int curstate);
  float getDeltaRelation (int prevstate, int curstate) const;
  int getCommMessageMood(int curstate, float mood, float randomresponsefactor) const;
  int getDefaultState (float relationship) const;
  int GetUnDockNode();
  int GetFailDockNode();
  int GetDockNode();
  int GetAbleToDockNode();
  int GetUnAbleToDockNode();
  int GetYesNode ();
  int GetNoNode();
  int GetHitNode ();
  int GetRequestLandNode();
  int GetContrabandInitiateNode();
  int GetContrabandUnDetectedNode();
  int GetContrabandDetectedNode();
  int GetContrabandWobblyNode();
};
class CommunicationMessage {
  void Init (Unit * send, Unit * recv);
  void SetAnimation (std::vector <class Animation *>*ani,unsigned char sex);
 public:
  FSM *fsm;//the finite state that this communcation stage is in
  class Animation * ani;
  unsigned char sex;//which sound should play
  int prevstate;
  int curstate;
  UnitContainer sender;
  CommunicationMessage(Unit * send, Unit * recv, std::vector <class Animation *>* ani, unsigned char sex);
  CommunicationMessage(Unit * send, Unit * recv, int curstate, std::vector <class Animation *>* ani,unsigned char sex);
  CommunicationMessage(Unit * send, Unit * recv, int prevvstate, int curstate, std::vector <class Animation *>* ani,unsigned char sex);
  CommunicationMessage(Unit * send, Unit * recv, const  CommunicationMessage &prevsvtate, int curstate, std::vector <class Animation *>* ani,unsigned char sex);
  void SetCurrentState(int message, std::vector <class Animation *> *ani,unsigned char sex);
  FSM::Node * getCurrentState() {return &fsm->nodes[curstate];}
  const vector <FSM::Node> &GetPossibleState () const;
  float getDeltaRelation()const {return fsm->getDeltaRelation (prevstate,curstate);}
};
#endif
