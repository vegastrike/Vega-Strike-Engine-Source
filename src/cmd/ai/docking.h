#include "navigation.h"
namespace Orders {

class DockingOps : public MoveTo {
  UnitContainer docking;
  Unit * formerOwnerDoNotDereference;
  enum {GETCLEARENCE,DOCKING,DOCKED,UNDOCKING} state;
  Order * oldstate;
  float timer;
  int port;
 public:
  
  DockingOps (Unit * unitToDockWith, Order * oldstate);
  virtual void SetParent (Unit * par);
  void Execute();
  bool RequestClearence(Unit * );
  bool DockToTarget(Unit * );
  bool PerformDockingOperations(Unit *);
  bool Undock(Unit *);
  virtual void Destroy();
  void RestoreOldAI();
};

}
