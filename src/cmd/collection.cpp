#include <stdlib.h>
#include "collection.h"
#ifndef LIST_TESTING
#include "unit.h"
#endif

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
void UnitCollection::destr() {
  UnitListNode *tmp;
  while (u.next) {
    tmp = u.next;
    u.next = u.next->next;
    delete tmp;
  }  
}

void UnitCollection::prepend(Iterator *iter) {
  UnitListNode *n = &u;
  Unit * tmp;
  while((tmp=iter->current())) {//iter->current checks for killed()
    n->next = new UnitListNode(tmp, n->next);
    iter->advance();
  }
}
void UnitCollection::append(Iterator *iter) {
  UnitListNode *n = &u;
  while(n->next->unit!=NULL) n = n->next;
  Unit * tmp;
  while((tmp=iter->current())) {
    n->next = new UnitListNode(tmp,n->next);
    n = n->next;
    iter->advance();
  }
}
void UnitCollection::append(Unit *unit) { 
  UnitListNode *n = &u;
  while(n->next->unit!=NULL) n = n->next;
  n->next = new UnitListNode(unit, n->next);
}	
void UnitCollection::UnitListNode::PostInsert (Unit * unit) {
  if(next->unit!=NULL)
    next->next = new UnitListNode(unit, next->next);
  else
    next = new UnitListNode(unit, next);
}
void UnitCollection::UnitIterator::postinsert(Unit *unit) {
  pos->PostInsert (unit);
}
void UnitCollection::FastIterator::postinsert(Unit *unit) {
  pos->PostInsert (unit);
}
void UnitCollection::UnitListNode::Remove () {
  if (next->unit) {
    UnitListNode * tmp = next->next;
    delete next; //takes care of unref!
    next = tmp;
  }
}
void UnitCollection::UnitIterator::remove() {
  pos->Remove ();
}
void UnitCollection::FastIterator::remove() {
  pos->Remove ();
}


void UnitCollection::UnitIterator::GetNextValidUnit () {
  while (pos->next->unit?pos->next->unit->Killed():false) {
    remove();
  }
}


void UnitCollection::ConstIterator::GetNextValidUnit () {
  while (pos->next->unit?pos->next->unit->Killed():false) {
    pos = pos->next;
  }
}
