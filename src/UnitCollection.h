
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
    
    UnitListNode(Unit *unit);
    UnitListNode(Unit *unit, UnitListNode *next);
    ~UnitListNode();
  } *units;
 public:
  UnitCollection() : units(new UnitListNode(NULL)) { }
  ~UnitCollection();
  class UnitIterator : public Iterator {
  private:
    UnitListNode *pos;
  public:
    UnitIterator(UnitListNode *start) : pos(start) {  }
    ~UnitIterator() { }

    void remove();
    void preinsert(Unit *unit);
    void postinsert(Unit *unit);
    Unit *current();
    Unit *advance();
  };

  friend class UnitCollection::UnitIterator;
  
  UnitIterator *createIterator() { 
    return new UnitIterator(units);
  }
  void prepend(Unit *unit);
  void prepend(Iterator *iter);
  void append(Unit *unit);
  void append(Iterator *iter);
  void clear ();
};

#endif
