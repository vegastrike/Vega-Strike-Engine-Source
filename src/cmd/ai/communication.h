#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_
#include "cmd/unit_generic.h"

class FSM {
 protected:

 public:
  struct Node {
    vector <std::string> messages;
    vector <int> sounds;//messages.size() sound for each sex
    float messagedelta;
    vector <unsigned int> edges;
    int GetSound (unsigned char sex, unsigned int multiple)const;
    std::string GetMessage(unsigned int &multiple)const;
    void AddSound (int sound, unsigned char sex);
    Node (const vector<std::string> &message, float messagedel): messages(message),messagedelta(messagedel){if (messages.size()==0) messages.push_back("<static>");}
    static Node MakeNode(std::string message,float messagedel) {
      vector<string> tmp;tmp.push_back(message);
      return Node(tmp,messagedel);
    }
  };
  vector <Node> nodes;
  FSM (const char * filename);
  void LoadXML(const char * factionfile);  
  void beginElement(const string &name, const AttributeList attributes);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  std::string GetEdgesString (int curstate);
  float getDeltaRelation (int prevstate, int curstate) const;
  int getCommMessageMood(int curstate, float mood, float randomresponsefactor,float relationship) const;
  int getDefaultState (float relationship) const;
  int GetUnDockNode()const ;
  int GetFailDockNode()const;
  int GetDockNode()const;
  int GetAbleToDockNode()const;
  int GetUnAbleToDockNode()const;
  int GetYesNode ()const;
  int GetNoNode()const;
  int GetHitNode ()const;
  int GetDamagedNode() const;
  int GetDealtDamageNode() const;
  int GetScoreKillNode() const;
  int GetRequestLandNode()const;
  int GetContrabandInitiateNode()const;
  int GetContrabandUnDetectedNode()const;
  int GetContrabandDetectedNode()const;
  int GetContrabandWobblyNode()const;
  
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
  FSM::Node * getCurrentState() {if (curstate<(int)fsm->nodes.size()) return &fsm->nodes[curstate]; else return &fsm->nodes[fsm->getDefaultState(0)];}
  const vector <FSM::Node> &GetPossibleState () const;
  float getDeltaRelation()const {return fsm->getDeltaRelation (prevstate,curstate);}
};
#endif
