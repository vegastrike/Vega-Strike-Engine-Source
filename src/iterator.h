#ifndef _ITERATOR_H_
#define _ITERATOR_H_

class Unit;

class Iterator {
 public:
  virtual void insert(Unit *unit) = 0;
  virtual Unit *current() = 0;
  virtual Unit *advance() = 0;
};


#endif

