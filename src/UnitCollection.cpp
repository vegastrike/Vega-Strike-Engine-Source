#include <stdlib.h>
#include "UnitCollection.h"
#include "cmd_unit.h"


UnitCollection::UnitListNode::UnitListNode (Unit *unit):unit(unit), next(NULL){
  if (unit) {
    unit->Ref();
  }
}
UnitCollection::UnitListNode::UnitListNode(Unit *unit, UnitListNode *next) : unit(unit), next(next) { 
  if (unit) {
    unit->Ref();
  }
}

UnitCollection::UnitListNode::~UnitListNode() { 
  if(NULL!=unit) {
    unit->UnRef(); 	
  }
}
void UnitCollection::clear() {
  UnitListNode *tmp;
  while (units) {
    tmp = units;
    units = units->next;
    delete tmp;
  }  
}
UnitCollection::~UnitCollection() {
  clear();
}

void UnitCollection::prepend(Unit *unit) { 
  if (unit->Killed()) {
    return;
  }
  units->next = new UnitListNode(unit, units->next); 
}

void UnitCollection::prepend(Iterator *iter) {
  UnitListNode *n = units;
  Unit * tmp;
  while((tmp=iter->current())) {//iter->current checks for killed()
    n->next = new UnitListNode(tmp, n->next);
    iter->advance();
  }
}
void UnitCollection::append(Iterator *iter) {
  UnitListNode *n = units;
  while(n->next!=NULL) n = n->next;
  Unit * tmp;
  while((tmp=iter->current())) {
    n->next = new UnitListNode(tmp, NULL);
    n = n->next;
    iter->advance();
  }
}
void UnitCollection::append(Unit *unit) { 
  if (unit->Killed()){
    return;//don't do anything with dead units...buah
  }
  UnitListNode *n = units;
  while(n->next!=NULL) n = n->next;
  n->next = new UnitListNode(unit, NULL);
}	
void UnitCollection::UnitIterator::preinsert(Unit *unit) {
  if (unit->Killed())
    return;
  pos->next = new UnitListNode(unit, pos->next);
}

void UnitCollection::UnitIterator::postinsert(Unit *unit) {
  if (unit->Killed())
    return;
  if(pos->next!=NULL)
    pos->next->next = new UnitListNode(unit, pos->next->next);
  else
    pos->next = new UnitListNode(unit, NULL);
}

void UnitCollection::UnitIterator::remove() {
  if (pos->next) {
    UnitListNode * tmp = pos->next->next;
    delete pos->next; //takes care of unref!
    pos->next = tmp;
  }
}

Unit *UnitCollection::UnitIterator::current() {
  while (pos->next) {
    if (pos->next->unit->Killed()) {
      remove();
    }	else {
      break;//unit not dead, return it!
    }
  }
  return pos->next?pos->next->unit:NULL;//if pos->next return that unit, otherwise NULL;
}

Unit *UnitCollection::UnitIterator::advance() {
  if(pos->next!=NULL) {
    pos = pos->next;
    return current();
  } else {
    return NULL;
  }
}

