#include "gfxlib.h"
#include "gfx_click_list.h"
#include "cmd_unit.h"



bool queryShip (int mouseX, int mouseY,Unit *ship) {  
  //changed from || to && so it has to be first in the sphere then in the box... FIXME to maybe be one or the other!!!
  return ship->querySphere(mouseX,mouseY,0)&&ship->queryBoundingBox(mouseX,mouseY,0);  
}

ClickList::ClickList (UnitCollection *parIter) {
    parentIter = parIter;
}



UnitCollection * ClickList::requestIterator (int mouseX, int mouseY) {

    UnitCollection * uc = new UnitCollection;
    UnitCollection::UnitIterator * UAye = uc->createIterator();
    UnitCollection::UnitIterator * myParent = parentIter->createIterator();
    while (myParent->advance()) {
      if (queryShip(mouseX,mouseY,myParent->current()))
	UAye->insert (myParent->current());
    }
    return uc;
}
