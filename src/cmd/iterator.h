#ifndef _ITERATOR_H_
#define _ITERATOR_H_

class Unit;

class Iterator {
 public:
  virtual ~Iterator() { };
  virtual void insert(Unit *unit) { postinsert(unit);}
  virtual void preinsert(Unit *unit) = 0;
  virtual void postinsert(Unit *unit) = 0;
  virtual void remove() = 0;
  virtual Unit *current() = 0;
  virtual void advance() = 0;
  inline Unit * operator ++(int) {Unit * un = current();advance();return un;}
  inline Unit * operator ++() {advance();return current();}
  inline Unit * operator * () {return current();}
};

class ConstantIterator {
 public:
  virtual ~ConstantIterator() { };
  virtual const Unit *current()const = 0;
  virtual void advance() = 0;
  inline const Unit * operator ++() {advance();return current();}
  inline const Unit * operator ++(int) {const Unit * un=current();advance();return un;}
  inline const Unit * operator * ()const {return current();}
};


#endif

