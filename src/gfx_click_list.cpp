#include "gfxlib.h"
#include "gfx_click_list.h"
#include "cmd_unit.h"



bool ClickList::queryShip (int mouseX, int mouseY,Unit *ship) {   
  if (ship->querySphere(mouseX,mouseY,0)){
    //fprintf (stderr,"bingo A");
    if (ship->queryBoundingBox(mouseX,mouseY,0)) {
      //fprintf (stderr,"BONGO BOB!!!!!");
      return true;
    }
  }
  return false;
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
