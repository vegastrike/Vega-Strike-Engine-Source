
/* unitCollection.h
 * 
 *****/

#ifndef _UNITCOLLECTION_H_
#define _UNITCOLLECTION_H_

#include "iterator.h"
#include <stdlib.h>
class Unit;

class UnitCollection {
 private:
  ///units is the beginning of the list. It isn't a pointer to reduce indirection
  class UnitListNode {
  public:
    Unit *unit;
    UnitListNode *next;
    
    UnitListNode(Unit *unit);
    ///These are functions for iterator use only
    void PostInsert (Unit * un);
    /// iterator use only
    void Remove ();
    UnitListNode(Unit *unit, UnitListNode *next);
    ~UnitListNode();
  } u;
  ///Destroys the list until init is called. Functions will segfault.
  void destr ();
  ///Initializes the list so that there are 2 empty nodes (u and u->next)  NULL unit terminates this list.
  void init () {u.next = new UnitListNode (NULL);}
 public:  
  ///Initislizes the first unit and then calls init;
  UnitCollection() : u(NULL) {init();}
  ///destroys the list permanently
  ~UnitCollection() {destr();}
  class UnitIterator : public Iterator {
  private:
    ///the position in the list
    UnitListNode *pos;
    ///Finds the next unit (or NULL) that isn't Killed()
    void GetNextValidUnit();
  public:
    UnitIterator(){}
    ///Creates this unit iterator
    UnitIterator(UnitListNode *start) : pos(start) {GetNextValidUnit();}
    ///removes something after pos.  eg the first valid unit. or current()
    void remove();
    ///inserts in front of current
    void preinsert(Unit *unit){pos->next = new UnitListNode(unit, pos->next);}
    /// inserts after current
    void postinsert(Unit *unit);
    ///returns the unit pos is pointing at or NULL if all dead or end of list.
    Unit *current(){return pos->next->unit;}
    ///advances the counter
    void advance() {pos = pos->next;GetNextValidUnit();}
  };
  class ConstIterator:public ConstantIterator {
    private:
    const UnitListNode *pos;
    void GetNextValidUnit();
  public:
    ConstIterator() {}
    ConstIterator(const UnitListNode *start):pos(start) {
      GetNextValidUnit();
    }
    const Unit *current() const  {return pos->next->unit;}
    void advance() {pos = pos->next;GetNextValidUnit();}
  };

  class ConstFastIterator:public ConstantIterator{
    private:
    const UnitListNode *pos;
  public:
    ConstFastIterator(){}
    ConstFastIterator(const UnitListNode *start):pos(start) {}
    const Unit *current()const {return pos->next->unit;}
    void advance() {pos = pos->next;}
  };
  class FastIterator: public Iterator{
    private:
    UnitListNode *pos;
  public:
    ///removes something after pos.  eg the first valid unit. or current()
    void remove();
    ///inserts in front of current
    void preinsert(Unit *unit){pos->next = new UnitListNode(unit, pos->next);}
    /// inserts after current
    void postinsert(Unit *unit);
    FastIterator(){}
    FastIterator(UnitListNode *start):pos(start) {}
    Unit *current() {return pos->next->unit;}
    void advance() {pos = pos->next;}
  };
    
  bool empty() const {return (u.next->unit==NULL);}
  UnitIterator createIterator() {return UnitIterator(&u);}
  ConstIterator constIterator() const {return ConstIterator (&u);}
  FastIterator fastIterator() {return FastIterator (&u);}
  ConstFastIterator constFastIterator () const{return ConstFastIterator(&u);}
  void prepend(Unit *unit) {u.next= new UnitListNode (unit,u.next);}
  void prepend(Iterator *iter);
  void append(Unit *unit);
  void append(Iterator *iter);
  void clear () {destr();init();}
  UnitCollection (const UnitCollection &c);
};
typedef UnitCollection::UnitIterator un_iter;
typedef UnitCollection::ConstIterator un_kiter;
typedef UnitCollection::ConstFastIterator un_fkiter;
typedef UnitCollection::FastIterator un_fiter;

#endif
