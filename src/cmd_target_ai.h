#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "cmd_order.h"

//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public Order {
  float rxntime;
  float delay;
  float agg;
  bool ShouldFire(Unit * targ);
protected:
  void ChooseTargets(int num);//chooses n targets and puts the best to attack in unit's target container
public:
  FireAt (float reaction_time, float aggressivitylevel);//weapon prefs?
  virtual void Execute();
};

class AggressiveAI: public FireAt {
protected:	
  bool facingtarg;

public:
  AggressiveAI (Unit * target=NULL);
  void Execute ();
};

}
#endif


