#include "navigation.h"
namespace Orders {

class DockingOps : public MoveTo {
  UnitContainer docking;
  void * formerOwnerDoNotDereference;
  enum {GETCLEARENCE,DOCKING,DOCKED,UNDOCKING} state;
  Order * oldstate;
  float timer;
  int port;
  bool physicallyDock;
  bool facedtarget;
  bool keeptrying;
 public:
  QVector Movement(Unit * utdw);
  DockingOps (Unit * unitToDockWith, Order * oldstate, bool physicallyDock, bool keeptrying);
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
