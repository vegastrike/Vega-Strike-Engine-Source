#ifndef _CMD_TARGET_AI_H_
#define _CMD_TARGET_AI_H_
#include "cmd_order.h"

//all unified AI's should inherit from FireAt, so they can choose targets together.
namespace Orders {

class FireAt: public Order {
  float rxntime;
  float delay;
  float agg;
public:
  FireAt (float reaction_time, float aggressivitylevel);//weapon prefs?
  virtual void Execute();
};

class AggressiveAI: public FireAt {
protected:	
  
  void ChooseTargets(int num);//chooses n targets and puts the best to attack in unit's target container
public:
  AggressiveAI (char * filetable, Unit * target=NULL);
  void Execute ();
};

}
#endif


