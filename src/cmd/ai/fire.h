#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "order.h"
#include "event_xml.h"
//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public Order {
  bool itts;
  float rxntime;
  float delay;
  float agg;
  bool ShouldFire(Unit * targ);
protected:
  float distance;
  void ChooseTargets(int num);//chooses n targets and puts the best to attack in unit's target container
public:
  FireAt (float reaction_time, float aggressivitylevel, bool itts);//weapon prefs?
  virtual void Execute();
};



}
#endif


