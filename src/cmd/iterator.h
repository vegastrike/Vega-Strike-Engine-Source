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
  virtual Unit *advance() = 0;
};

#endif

