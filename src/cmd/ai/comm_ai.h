#ifndef __COMM_AI_H
#define __COMM_AI_H

#include "order.h"
#include <map>
#include <vector>
/// this class is meant to be inherited by AI wishing to process comm messages


class CommunicatingAI:public Order{

  typedef std::map<const Unit*,float> relationmap;
 protected:
  unsigned char sex;//male female robot???
  float anger;//how easily he will target you if you taunt him enough (-.00001 is always -.4 is often -.6 is normal
  float appease;//how easily he will switch from you if you silence his will to kill (-.5)
  float moodswingyness;
  float randomresponse;
  float mood;
  float rank;
  std::vector <class Animation *> *comm_face;
  UnitContainer contraband_searchee;
  Vector SpeedAndCourse;
  int which_cargo_item;
  relationmap effective_relationship;
  void GetMadAt (Unit * which, int howMad);
 protected:
  float getAnger (const Unit * un)const;
  virtual std::vector <Animation *> *getCommFaces(unsigned char &sex);
 public:
  virtual float getMood() {return mood;}
  virtual float GetEffectiveRelationship (const Unit * target) const;
  Unit * GetRandomUnit (float PlayerProbability, float TargetProbability);
  void RandomInitiateCommunication (float PlayerProbability, float TargetProbability);
  void TerminateContrabandSearch(bool foundcontraband);
  void InitiateContrabandSearch(float PlayerProbability, float TargetProbability);
  void UpdateContrabandSearch();
  CommunicatingAI (int ttype,int stype, float rank=666, float mood=0, float anger=-666/*-.5*/, float appeasement=666, float moodswingyness=666/*.2*/, float randomnessresponse=666/*.8*/);
   virtual void ProcessCommMessage (class CommunicationMessage &c ); 
   virtual void AdjustRelationTo (Unit * un, float factor);
   virtual ~CommunicatingAI ();
   virtual int selectCommunicationMessage (class CommunicationMessage &c, Unit *);
   virtual int selectCommunicationMessageMood (CommunicationMessage &c, float mood);
   virtual void SetParent (Unit * parent1);
};

#endif
