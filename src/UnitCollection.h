
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

 public:
  UnitCollection() : units(new UnitListNode(NULL)) { }
  ~UnitCollection() { if (units) delete units; }
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

  friend UnitCollection::UnitIterator;
  
  UnitIterator *createIterator() { 
    return new UnitIterator(units);
  }
  void prepend(Unit *unit) { units->next = new UnitListNode(unit, units->next); }
  void prepend(Iterator *iter) {
    UnitListNode *n = units;

    while(iter->current()!=NULL) {
      n->next = new UnitListNode(iter->current(), n->next);
      iter->advance();
    }
  }

  void append(Unit *unit) { 
    UnitListNode *n = units;
    while(n->next!=NULL) n = n->next;
    n->next = new UnitListNode(unit, NULL);
  }
  void append(Iterator *iter) {
    UnitListNode *n = units;
    while(n->next!=NULL) n = n->next;

    while(iter->current()!=NULL) {
      n->next = new UnitListNode(iter->current(), NULL);

      n = n->next;
      iter->advance();
    }
  }
};

#endif
