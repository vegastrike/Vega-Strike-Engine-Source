#include <stdlib.h>
#include "UnitCollection.h"

void UnitCollection::UnitIterator::insert(Unit *unit) {
  if(pos->next!=NULL)
    pos->next->next = new UnitListNode(unit, NULL);
  else
    pos->next = new UnitListNode(unit, NULL);
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
