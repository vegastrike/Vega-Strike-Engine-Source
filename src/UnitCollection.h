
/* unitCollection.h
 * 
 *****/

#ifndef _UNITCOLLECTION_H_
#define _UNITCOLLECTION_H_

#include "iterator.h"

class Unit;

class UnitCollection {
 private:
  class UnitListNode {
  public:
    Unit *unit;
    UnitListNode *next;
    
    UnitListNode(Unit *unit) : unit(unit), next(0) { }
    UnitListNode(Unit *unit, UnitListNode *next) : unit(unit), next(next) { }
    ~UnitListNode() { if(0!=next) delete next; }
  } *units, *tail;

 public:
  UnitCollection() : units(new UnitListNode(NULL)), tail(0) { }
  ~UnitCollection() { if (units) delete units; }
  class UnitIterator : public Iterator {
  private:
    UnitListNode *pos;
  public:
    UnitIterator(UnitListNode *start) : pos(start) {  }

    void insert(Unit *unit);
    Unit *current();
    Unit *advance();
  };
  
  UnitIterator *createIterator() { 
    return new UnitIterator(units);
  } // UnitIterator does not have a nasty destructor, so this should be fine
};

#endif
