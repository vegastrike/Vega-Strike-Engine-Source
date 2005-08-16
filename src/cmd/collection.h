
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
#ifdef _TEST_
  friend void Iterate (UnitCollection &c);
#endif
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

  private:
    UnitListNode( );
    UnitListNode( const UnitListNode& );
    UnitListNode& operator=( const UnitListNode& );
  } u;
  ///Destroys the list until init is called. Functions will segfault.
  void destr ();
  ///Initializes the list so that there are 2 empty nodes (u and u->next)  NULL unit terminates this list.
  void init () {u.next = new UnitListNode (NULL, new UnitListNode(NULL));}
 public:  
  ///Initislizes the first unit and then calls init;
  UnitCollection() : u(NULL) {init();}
  ///destroys the list permanently
  ~UnitCollection() {destr();}
  class UnitIterator{
#ifdef _TEST_
  friend void Iterate (UnitCollection &c);
#endif
  private:
    ///the position in the list
    UnitListNode *pos;
    ///Finds the next unit (or NULL) that isn't Killed()
    inline void GetNextValidUnit();
  public:
    UnitIterator() :pos(NULL) { }
    ///Creates this unit iterator
    UnitIterator(UnitListNode *start) : pos(start) {
        GetNextValidUnit();
    }
    UnitIterator( const UnitIterator& orig ) : pos(orig.pos) { }
    UnitIterator& UnitIterator::operator=( const UnitIterator& orig ) {
        pos = orig.pos; return *this;
    }
    ~UnitIterator() {
        pos = NULL;
    }

    bool isDone(){return current()==NULL;}
    bool notDone() {return current()!=NULL;}
    ///removes something after pos.  eg the first valid unit. or current()
    void remove();
    void moveBefore(UnitCollection& otherList);
    ///inserts in front of current
    void preinsert(Unit *unit){pos->next = new UnitListNode(unit, pos->next);}
    /// inserts after current
    void postinsert(Unit *unit);
    ///returns the unit pos is pointing at or NULL if all dead or end of list.
    Unit *current(){return pos->next->unit;}
    ///advances the counter
    Unit * next() {advance();return current();}	  
    void advance() {pos = pos->next;GetNextValidUnit();}
    inline Unit * operator ++(int) {Unit * un = current();advance();return un;}
    inline Unit * operator ++() {advance();return current();}
    inline Unit * operator * () {return current();}
  };
  class ConstIterator {
    private:
    const UnitListNode *pos;
    void GetNextValidUnit();
  public:
    ConstIterator() : pos(NULL) { }
    ConstIterator( const ConstIterator& orig ) : pos( orig.pos ) { }
    ConstIterator(const UnitListNode *start):pos(start) {
      GetNextValidUnit();
    }
    ConstIterator& ConstIterator::operator=( const ConstIterator& orig ) {
        pos = orig.pos;
        return *this;
    }
    ~ConstIterator( ) {
        pos = NULL;
    }
    const Unit * next() {advance();return current();}	  	  
    const Unit *current() const  {return pos->next->unit;}
	bool isDone()const{return current()==NULL;}
	bool notDone()const{return current()!=NULL;}	  
    void advance() {pos = pos->next;GetNextValidUnit();}
    inline const Unit * operator ++() {advance();return current();}
    inline const Unit * operator ++(int) {const Unit * un=current();advance();return un;}
    inline const Unit * operator * ()const {return current();}
  };

  class ConstFastIterator {
    private:
    const UnitListNode *pos;
  public:
    ConstFastIterator() :pos(NULL) {}
    ConstFastIterator( const ConstFastIterator& orig ) : pos( orig.pos ) {}
    ConstFastIterator(const UnitListNode *start):pos(start) {}
    ~ConstFastIterator() { pos=NULL; }
    const Unit *current()const {return pos->next->unit;}
    void advance() {pos = pos->next;}
    inline const Unit * operator ++() {advance();return current();}
    inline const Unit * operator ++(int) {const Unit * un=current();advance();return un;}
    inline const Unit * operator * ()const {return current();}
    const Unit * next() {advance();return current();}
  private:
    ConstFastIterator& operator=( const ConstFastIterator& );
  };

  class FastIterator {
    private:
    UnitListNode *pos;
  public:

    ///removes something after pos.  eg the first valid unit. or current()
    void remove();
    ///inserts in front of current
    void preinsert(Unit *unit){pos->next = new UnitListNode(unit, pos->next);}
    /// inserts after current
    void postinsert(Unit *unit);
    FastIterator() : pos(NULL) {}
    FastIterator( const FastIterator& orig ) : pos(orig.pos) {}
    FastIterator(UnitListNode *start):pos(start) {}
    ~FastIterator() { pos = NULL; }
    Unit *current() {return pos->next->unit;}
    void advance() {pos = pos->next;}
    inline Unit * operator ++(int) {Unit * un = current();advance();return un;}
    inline Unit * operator ++() {advance();return current();}
    inline Unit * operator * () {return current();}
    Unit * next() {advance();return current();}	  	  
  private:
    FastIterator& operator=( const FastIterator& );
  };
  static void FreeUnusedNodes();//not allowed to happen if any lists are traversing    
  static void * PushUnusedNode(UnitListNode * node);
#ifdef _TEST_
  friend void Iterate (UnitCollection &c);
#endif
  //could be empty and this returns false...but usually correct...never has units when it returns true
  bool empty() const {return (u.next->unit==NULL);}
  UnitIterator createIterator() {return UnitIterator(&u);}
  ConstIterator constIterator() const {return ConstIterator (&u);}
  FastIterator fastIterator() {return FastIterator (&u);}
  ConstFastIterator constFastIterator () const{return ConstFastIterator(&u);}
  void insert_unique(Unit *un) {
    for (UnitListNode * i=u.next;i!=NULL;i=i->next) if (i->unit==un) return;
    prepend (un);
  }
  void prepend(Unit *unit) {u.next= new UnitListNode (unit,u.next);}
  void prepend(UnitListNode *unitlistnode) {unitlistnode->next=u.next;u.next= unitlistnode;}
  void prepend(UnitIterator *iter);
  void append(Unit *unit);
  void append(UnitIterator *iter);
  void clear () {destr();init();}
  UnitCollection (const UnitCollection &c);
  const UnitCollection & operator = (const UnitCollection &c);
};
typedef UnitCollection::UnitIterator un_iter;
typedef UnitCollection::ConstIterator un_kiter;
typedef UnitCollection::ConstFastIterator un_fkiter;
typedef UnitCollection::FastIterator un_fiter;

#endif
