#include "cmd_fireat.h"

namespace Orders {
class AggressiveAI: public FireAt {
protected:
  AIEvents::ElemAttrMap logic;
  float distance;
  short count;
  bool ProcessLogicItem (const AIEvents::AIEvresult & item);
  void ExecuteLogicItem (const AIEvents::AIEvresult & item);
  void ProcessLogic();
public:
  enum types {AGGAI, UNKNOWN, DISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR, BARMOR, LARMOR, RARMOR, HULL, FACING, MOVEMENT};
  AggressiveAI (const char *file, Unit * target=NULL);
  void Execute ();
};
}
