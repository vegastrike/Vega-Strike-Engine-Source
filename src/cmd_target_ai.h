#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "cmd_order.h"

//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public Order {
  bool itts;
  float rxntime;
  float delay;
  float agg;
  bool ShouldFire(Unit * targ);
protected:
  void ChooseTargets(int num);//chooses n targets and puts the best to attack in unit's target container
public:
  FireAt (float reaction_time, float aggressivitylevel, bool itts);//weapon prefs?
  virtual void Execute();
};

  struct AIEvent {

    
  };

class AggressiveAI: public FireAt {
protected:	
  bool threatened;
public:
  enum types {UNKNOWN, DISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR, BARMOR, LARMOR, RARMOR, HULL, FACING, MOVEMENT};
  AggressiveAI (Unit * target=NULL);
  void Execute ();
};

}
#endif


