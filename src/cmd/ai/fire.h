#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "comm_ai.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public CommunicatingAI {
  friend class PythonAI;//docu said this was neceessary

  bool ShouldFire(Unit * targ,bool &missilelock);
protected:
  float missileprobability;
  float rxntime;
  float delay;
  float agg;
  float distance;
  float gunspeed;
  float gunrange;
  void FireWeapons (bool shouldfire,bool lockmissile);
  //  bool DealWithMultipleTargets();
  void ChooseTargets(int num, bool force=false);//chooses n targets and puts the best to attack in unit's target container
  bool isJumpablePlanet(Unit *);
  void ReInit (float rxntime, float agglevel);
public:
  Unit * GetParent() {return CommunicatingAI::GetParent();}
  FireAt (float reaction_time, float aggressivitylevel);//weapon prefs?
  FireAt();
  virtual void Execute();
  virtual std::string Pickle() {return std::string();}//these are to serialize this AI
  virtual void UnPickle(std::string) {}
  virtual ~FireAt();
};



}
#endif


