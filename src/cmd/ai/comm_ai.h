#include "order.h"
#include <map>
/// this class is meant to be inherited by AI wishing to process comm messages


class CommunicatingAI:public Order{
  typedef std::map<const Unit*,float> relationmap;
  float anger;//how easily he will target you if you taunt him enough (-.00001 is always -.4 is often -.6 is normal
  float moodswingyness;
  float randomresponse;
  float mood;
  Animation * comm_face;
  UnitContainer contraband_searchee;
  bool contraband_detected;
  Vector SpeedAndCourse;
  relationmap effective_relationship;
 public:
  float GetEffectiveRelationship (const Unit * target) const;
  Unit * GetRandomUnit (float PlayerProbability, float TargetProbability);
  void RandomInitiateCommunication (float PlayerProbability, float TargetProbability);
  void TerminateContrabandSearch();
  void InitiateContrabandSearch(float PlayerProbability, float TargetProbability);
   CommunicatingAI (int ttype, float anger=-.5, float moodswingyness=.2, float randomnessresponse=.8,float mood=0);
   virtual void ProcessCommMessage (class CommunicationMessage &c); 
   virtual void AdjustRelationTo (Unit * un, float factor);
   virtual ~CommunicatingAI ();
   virtual int selectCommunicationMessage (class CommunicationMessage &c);
   virtual int selectCommunicationMessageMood (CommunicationMessage &c, float mood);
   virtual void SetParent (Unit * parent1);
};
