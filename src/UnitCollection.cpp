#include <stdlib.h>
#include "UnitCollection.h"

void UnitCollection::UnitIterator::preinsert(Unit *unit) {
  if(pos->next!=NULL)
    pos->next = new UnitListNode(unit, pos->next);
  else
    pos->next = new UnitListNode(unit, NULL);
}

void UnitCollection::UnitIterator::postinsert(Unit *unit) {
  if(pos->next!=NULL)
    pos->next->next = new UnitListNode(unit, pos->next->next);
  else
    pos->next = new UnitListNode(unit, NULL);
}

void UnitCollection::UnitIterator::remove() {
  if(pos->next!=NULL) {
    pos->next = pos->next->next;
  }
}

Unit *UnitCollection::UnitIterator::current() {
  if(pos->next!=NULL) return pos->next->unit;
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
