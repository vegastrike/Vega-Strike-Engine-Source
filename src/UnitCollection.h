
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
  } *units;
  bool persist;
 public:
  UnitCollection(bool persistant=false) : units(new UnitListNode(NULL)),persist(persistant) { }
  ~UnitCollection();
  class UnitIterator : public Iterator {
  private:
    UnitListNode *pos;
    bool persist;
  public:
    UnitIterator(UnitListNode *start, bool persistant) : pos(start), persist(persistant) {  }
    ~UnitIterator() { }

    void remove();
    void preinsert(Unit *unit);
    void postinsert(Unit *unit);
    Unit *current();
    Unit *advance();
  };

  friend UnitCollection::UnitIterator;
  
  UnitIterator *createIterator() { 
    return new UnitIterator(units,persist);
  }
  void prepend(Unit *unit);
  void prepend(Iterator *iter);
  void append(Unit *unit);
  void append(Iterator *iter);
  ///  Unit *item(unsigned i); //USELESS WITH PERSISTANT LISTS
};

#endif
