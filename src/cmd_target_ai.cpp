#include "cmd_target_ai.h"
using Orders::FireAt;
using Orders::AggressiveAI;

FireAt::FireAt (float reaction_time, float aggressivitylevel): Order (WEAPON|TARGET), rxntime (reaction_time), delay(0), agg (aggressivitylevel){
  
  
}

void FireAt::Execute () {
  UnitCollection::UnitIterator *iter = targets->createIterator();
  Unit * targ;
  while ((targ = iter->current())!=NULL) {
    

  }
  delete iter;
}
