
#ifndef _UNITCONTAINER_H_
#define _UNITCONTAINER_H_
class Unit;

class UnitContainer {
 protected:
  Unit * unit;
 public:
  UnitContainer ();
  UnitContainer (Unit *);
  UnitContainer (const UnitContainer &un) {
    unit =0;
    SetUnit (un.unit);
  }
  const UnitContainer & operator = (const UnitContainer &a) {
    SetUnit (a.unit);
    return a;
  }
  ~UnitContainer();
  void SetUnit (Unit *);
  inline Unit * GetUnit();
};
#endif
