/* unitCollection.h
 * 
 *****/

#ifndef _UNITCOLLECTION_H_
#define _UNITCOLLECTION_H_

#include "iterator.h"

class Unit;

class UnitCollection {
 private:
  class UnitList {
  public:
    Unit *unit;
    UnitList *next;
    
    UnitList(Unit *unit) : unit(unit), next(NULL) { }
    UnitList(Unit *unit, UnitList *next) : unit(unit), next(next) { }
    ~UnitList() { if(0!=next) delete next; }
    
    void insert(Unit *unit);
    void append(Unit *unit);
    void remove(Unit *unit);
  }
  *units;

 public:
  UnitCollection() : units(NULL) { }
  ~UnitCollection() { delete units; }

  class UnitIterator : public Iterator {
  private:
    UnitList *pos;
  public:
    UnitIterator(UnitList *start) : pos(start) {  }

    void insert(Unit *unit);
    Unit *current();
    Unit *advance();
  };
  
  UnitIterator *createIterator();
};

#endif
