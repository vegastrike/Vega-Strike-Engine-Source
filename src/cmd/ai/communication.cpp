#include "communication.h"
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>

FSM::FSM (const char * filename) {
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
  if (strlen(filename)==0) {
    nodes.push_back (Node("welcome to cachunkcachunk.com",0));
    nodes.push_back (Node("I love you!",.1));
    nodes.push_back (Node("J00 0wnz m3",.08));
    nodes.push_back (Node("You are cool!",.06));
    nodes.push_back (Node("You are nice!",.05));
    nodes.push_back (Node("Ya you're naled! NALED PAL!",-.02));
    nodes.push_back (Node("i 0wnz j00",-.08));
    nodes.push_back (Node("I hate you!",-.1));

    nodes.push_back (Node("Docking operation complete.",0));
    nodes.push_back (Node("Please move into a green docking box and press d.",0));
    nodes.push_back (Node("Docking operation begun.",0));
    nodes.push_back (Node("Clearance denied.",0));
    nodes.push_back (Node("Clearance granted.",0));
    nodes.push_back (Node("No.",0));
    nodes.push_back (Node("Yes.",0));
    nodes.push_back (Node("Prepare To Be Searched. Maintain Speed and Course.",0));
    nodes.push_back (Node("No contraband detected: You may proceed.",0));
    nodes.push_back (Node("Contraband detected! All units close and engage!",0));
    nodes.push_back (Node("Your Course is deviating! Maintain Course!",0));
    nodes.push_back (Node("Request Clearence To Land.",0));
    nodes.push_back (Node("*hit*",-.2));
    vector <unsigned int> edges;
    unsigned int i;
    for (i=0;i<nodes.size()-13;i++) {
      edges.push_back (i);
    }
    for (i=0;i<nodes.size();i++) {
      nodes[i].edges = edges;
    }
  } else {
    LoadXML(filename);
  }
}
int FSM::GetUnDockNode() const{
  return nodes.size()-16;
}
int FSM::GetFailDockNode()const {
  return nodes.size()-15;
}
int FSM::GetDockNode() const{
  return nodes.size()-14;
}
int FSM::GetUnAbleToDockNode() const{
  return nodes.size()-13;
}
int FSM::GetAbleToDockNode() const{
  return nodes.size()-12;
}
int FSM::GetNoNode() const{
  return nodes.size()-11;
}
int FSM::GetYesNode() const{
  return nodes.size()-10;
}
int FSM::GetContrabandInitiateNode() const{
  return nodes.size()-9;
}
int FSM::GetContrabandUnDetectedNode() const{
  return nodes.size()-8;
}
int FSM::GetContrabandDetectedNode() const{
  return nodes.size()-7;
}
int FSM::GetContrabandWobblyNode() const{
  return nodes.size()-6;
}
int FSM::GetRequestLandNode () const{
  return nodes.size()-5;
}
int FSM::GetHitNode () const{
  return nodes.size()-1;
}
int FSM::GetDamagedNode () const{
  return nodes.size()-2;
}
int FSM::GetDealtDamageNode () const{
  return nodes.size()-3;
}
int FSM::GetScoreKillNode () const{
  return nodes.size()-4;
}
static float sq (float i) {return i*i;}
bool nonneg (float i) {return i>=0;}
int FSM::Node::GetSound (unsigned char sex) {
  if (((unsigned int)sex)<sound.size()) {
    return sound[sex];
  }else {
    return -1;
  }
}
void FSM::Node::AddSound (int sounds, unsigned char sex) {
  while (((unsigned int) sex)>=sound.size()) {
    sound.push_back (-1);
  }
  sound[sex]=sounds;
}

int FSM::getCommMessageMood (int curstate, float mood, float randomresponse,float relationship) const{
  const FSM::Node *n = curstate<nodes.size()?(&nodes[curstate]):(&nodes[getDefaultState(relationship)]);
  mood+=-randomresponse+2*randomresponse*((float)rand())/RAND_MAX;
  
  int choice=0;
#if 0
  float bestchoice=4;
  bool fitmood=false;
  for (unsigned i=0;i<n->edges.size();i++) {
    float md = nodes[n->edges[i]].messagedelta;
    bool newfitmood=nonneg(mood)==nonneg(md);
    if ((!fitmood)||newfitmood) {
      float newbestchoice=sq(md-mood);
      if ((newbestchoice<=bestchoice)||(fitmood==false&&newfitmood==true)) {
	if ((newbestchoice==bestchoice&&rand()%2)||newbestchoice<bestchoice) {
	  //to make sure some variety happens
	  fitmood=newfitmood;
	  choice =i;
	  bestchoice = newbestchoice;
	}
      }
    }
  }
#endif
	vector<unsigned int> g;
	vector <unsigned int>b;
	static float pos_limit =XMLSupport::parse_float(vs_config->getVariable ("AI",
											"LowestPositiveCommChoice",
											"-.015"));
	static float neg_limit =XMLSupport::parse_float(vs_config->getVariable ("AI",
											"LowestNegativeCommChoice",
											".025"));

	for (unsigned int i=0;i<n->edges.size();i++) {
		float md=nodes[n->edges[i]].messagedelta;
		if (md>=pos_limit) {
			g.push_back(i);
		}
		if (md<=neg_limit) {
			b.push_back(i);
		}
	}
	if(((g.size()!=0)&&(mood)>0)||(b.size()==0)) {
		choice=g[(rand()%g.size())];
	}else {
		if (b.size()) {
			choice = b[rand()%b.size()];
		}
	}
	return choice;
}
int FSM::getDefaultState (float relationship) const{

  float mood=relationship;
  float randomresponse=.01;
  int curstate=0;

  const FSM::Node *n = &nodes[curstate];
  mood+=-randomresponse+2*randomresponse*((float)rand())/RAND_MAX;
  
  int choice=0;
  float bestchoice=4;
  bool fitmood=false;
  for (unsigned i=0;i<n->edges.size();i++) {
    float md = nodes[n->edges[i]].messagedelta;
    bool newfitmood=nonneg(mood)==nonneg(md);
    if ((!fitmood)||newfitmood) {
      float newbestchoice=sq(md-mood);
      if ((newbestchoice<=bestchoice)||(fitmood==false&&newfitmood==true)) {
	if ((newbestchoice==bestchoice&&rand()%2)||newbestchoice<bestchoice) {
	  //to make sure some variety happens
	  fitmood=newfitmood;
	  choice =i;
	  bestchoice = newbestchoice;
	}
      }
    }
  }// (0,relationship,.01)
  return nodes[0].edges[choice];
}
std::string FSM::GetEdgesString (int curstate) {
  std::string retval="\n";
  for (unsigned int i=0;i<nodes[curstate].edges.size();i++) {
    retval+= tostring ((int)((i+1)%10))+"."+nodes[nodes[curstate].edges[i]].message+"\n";
  }
  retval+= "0. Request Docking Clearence";
  return retval;
}
float FSM::getDeltaRelation (int prevstate, int current_state) const{
  return nodes[current_state].messagedelta;
}

void CommunicationMessage::Init (Unit * send, Unit * recv) {
  fsm = FactionUtil::GetConversation (send->faction,recv->faction);
  sender.SetUnit (send);
  this->prevstate=this->curstate = fsm->getDefaultState(send->getRelation(recv));
}
float myround (float i) {
	float j= floor(i);
	if (i-j>=.5)
		return j+1;
	return j;
}
float myroundclamp (float i) {
float j=myround (i);
if (j<0)
j=0;
return j;
}

void CommunicationMessage::SetAnimation (std::vector <Animation *>*ani,unsigned char sex) {
  this->sex=sex;//for audio
  if (ani){ 
    if (ani->size()>0) {
	float mood= fsm->getDeltaRelation(this->prevstate,this->curstate);
	mood+=.1;
	mood*=(ani->size())/.2;
	unsigned int index=(unsigned int)myroundclamp(floor(mood));
	if (index>=ani->size()) {
	  index=ani->size()-1;
	}
	this->ani=(*ani)[index];
    } else {
      this->ani=NULL;
    }
  }else {
    this->ani=NULL;
  }
}

void CommunicationMessage::SetCurrentState (int msg,std::vector <Animation *>*ani, unsigned char sex) {
  curstate = msg;
  SetAnimation(ani,sex);
  assert (this->curstate>=0);
}

CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int messagechoice, std::vector <Animation *>* ani, unsigned char sex) {
  Init (send,recv);
  prevstate=fsm->getDefaultState (send->getRelation (recv));
  if (fsm->nodes[prevstate].edges.size()) {
    curstate = fsm->nodes[prevstate].edges[messagechoice%fsm->nodes[prevstate].edges.size()];
  }
  SetAnimation(ani,sex);
  assert (this->curstate>=0);

}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, int laststate, int thisstate, std::vector <Animation *>* ani, unsigned char sex) {
  Init (send,recv);
  prevstate=laststate;
  curstate = thisstate;
  SetAnimation(ani,sex);
    assert (this->curstate>=0);

}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv,std::vector<Animation *>* ani, unsigned char sex) {
  Init (send,recv);
  SetAnimation(ani,sex);
  assert (this->curstate>=0);

}
CommunicationMessage::CommunicationMessage (Unit * send, Unit * recv, const CommunicationMessage &prevstate, int curstate, std::vector<Animation *>* ani, unsigned char sex) {
  Init (send,recv);
  this->prevstate = prevstate.curstate;
  if (fsm->nodes[this->prevstate].edges.size()) {
    this->curstate = fsm->nodes[this->prevstate].edges[curstate%fsm->nodes[this->prevstate].edges.size()];
  }
  SetAnimation(ani,sex);
  assert (this->curstate>=0);

}
