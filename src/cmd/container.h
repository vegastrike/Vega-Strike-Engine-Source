
#ifndef _UNITCONTAINER_H_
#define _UNITCONTAINER_H_
class Unit;

class UnitContainer {
 private:
  Unit * unit;
 public:
  UnitContainer ();
  UnitContainer (Unit *);
  ~UnitContainer();
  void SetUnit (Unit *);
  inline Unit * GetUnit();
};
#endif
