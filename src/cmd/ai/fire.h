#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "comm_ai.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public CommunicatingAI {


  bool ShouldFire(Unit * targ);
protected:
  float missileprobability;
  float rxntime;
  float delay;
  float agg;
  float distance;
  float gunspeed;
  float gunrange;
  void FireWeapons (bool shouldfire);
  bool DealWithMultipleTargets();
  void ChooseTargets(int num, bool force=false);//chooses n targets and puts the best to attack in unit's target container
public:
  FireAt (float reaction_time, float aggressivitylevel);//weapon prefs?
  virtual void Execute();
  virtual ~FireAt();
};



}
#endif


