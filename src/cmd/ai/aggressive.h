#include "fire.h"

namespace Orders {
class AggressiveAI: public FireAt {
protected:
  AIEvents::ElemAttrMap logic;
  bool ProcessLogicItem (const AIEvents::AIEvresult & item);
  bool ExecuteLogicItem (const AIEvents::AIEvresult & item);
  void ProcessLogic();
public:
  enum types {AGGAI, MOVEMENT, FACING, UNKNOWN, DISTANCE, THREAT, FSHIELD, LSHIELD, RSHIELD, BSHIELD, FARMOR, BARMOR, LARMOR, RARMOR, HULL};
  AggressiveAI (const char *file, Unit * target=NULL);
  void Execute ();
};
}
