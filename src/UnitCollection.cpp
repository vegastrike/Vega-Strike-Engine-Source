#include <stdlib.h>
#include "UnitCollection.h"
#include "cmd_unit.h"


UnitCollection::~UnitCollection() {
  UnitListNode *tmp =units;
  if (persist){
    while (tmp) {
      if (tmp->unit)
	tmp->unit->UnRef();
      tmp = tmp->next;
    }
  }
  if (units) 
    delete units; 

}

void UnitCollection::prepend(Unit *unit) { 
  if (persist) {
    if (unit->Killed()) {
      return;
    }
    unit->Ref();
  }
  units->next = new UnitListNode(unit, units->next); 
}

void UnitCollection::prepend(Iterator *iter) {
  UnitListNode *n = units;
  Unit * tmp;
  while(tmp=iter->current()) {
    if (persist) {
      if (tmp->Killed()){
	iter->advance();
	continue;
      }
      tmp->Ref();
    }
    n->next = new UnitListNode(tmp, n->next);
    iter->advance();
  }
}
void UnitCollection::append(Iterator *iter) {
  UnitListNode *n = units;
  while(n->next!=NULL) n = n->next;
  Unit * tmp;
  while(tmp=iter->current()) {
    if (persist) {
      if (tmp->Killed()) {
	iter->advance();
	continue;
      }
      tmp->Ref();
    }
    n->next = new UnitListNode(tmp, NULL);
    n = n->next;
    iter->advance();
  }
}
void UnitCollection::append(Unit *unit) { 
  if (persist) {
    if (unit->Killed()){
      return;//don't do anything with dead units...buah
    }
    unit->Ref();
  }
  UnitListNode *n = units;
  while(n->next!=NULL) n = n->next;
  n->next = new UnitListNode(unit, NULL);
}	
void UnitCollection::UnitIterator::preinsert(Unit *unit) {
  if (persist) {
    if (unit->Killed())
      return;
    unit->Ref();
  }
  if(pos->next!=NULL)
    pos->next = new UnitListNode(unit, pos->next);
  else
    pos->next = new UnitListNode(unit, NULL);
}

void UnitCollection::UnitIterator::postinsert(Unit *unit) {
  if (persist) {
    if (unit->Killed())
      return;
    unit->Ref();
  }
  if(pos->next!=NULL)
    pos->next->next = new UnitListNode(unit, pos->next->next);
  else
    pos->next = new UnitListNode(unit, NULL);
}

void UnitCollection::UnitIterator::remove() {
  if(pos->next!=NULL&&pos->unit!=NULL) {
    pos->next = pos->next->next;
    free (pos->next);
  }
}

Unit *UnitCollection::UnitIterator::current() {
  if(pos->next!=NULL) {
    if (persist) {
      while (pos->next&&pos->next->unit->Killed()) {
	pos->next->unit->UnRef();
	remove();
	pos->next = pos->next->next;
	if (pos->next==NULL)
	  return NULL;
      }
    }
    return pos->next->unit;
  }
  else return NULL;
}

Unit *UnitCollection::UnitIterator::advance() {
  if(pos->next!=NULL) {
    pos = pos->next;
    return current();
  } else {
    return NULL;
  }
}



/*
Unit *UnitCollection::item(unsigned i){
  UnitListNode *n = units->next;
  while(n!=NULL) {
    if(i==0) return n->unit;
    n=n->next;
    i--;
  }
  return NULL;
}
*/
