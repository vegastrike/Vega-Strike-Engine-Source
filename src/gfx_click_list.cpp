#include "gfxlib.h"
#include "gfx_click_list.h"
#include "cmd_unit.h"
Vector MouseCoordinate (int x, int y, float zplane) {

  Vector xyz = Vector (0,0,zplane);
  //first get xyz in camera space
  xyz.i=zplane*(2*x/g_game.x_resolution-1)*g_game.MouseSensitivityX*GFXGetXInvPerspective();
  xyz.j=zplane*(2*y/g_game.y_resolution-1)*g_game.MouseSensitivityY*GFXGetYInvPerspective();
  return xyz;
}


bool queryShip (int mouseX, int mouseY,Unit *ship,Matrix vw) {  
  Vector pos = ship->Position();
  pos = Transform (vw,pos);//transforming pos into camspace
  //make pos into the position of the mouse cursor at the time;
  pos = MouseCoordinate (mouseX,mouseY,pos.k);
  //changed from || to && so it has to be first in the sphere then in the box... FIXME to maybe be one or the other!!!
  return ship->querySphere(vw,pos,0)&&ship->queryBoundingBox(vw,pos,0);  
}

ClickList::ClickList (UnitCollection *parIter) {
    parentIter = parIter;
}



UnitCollection * ClickList::requestIterator (int mouseX, int mouseY) {
    Matrix vw;
    GFXGetMatrix (VIEW,vw);
    UnitCollection * uc = new UnitCollection;
    UnitCollection::UnitIterator * UAye = uc->createIterator();
    UnitCollection::UnitIterator * myParent = parentIter->createIterator();
    while (myParent->advance()) {
      if (queryShip(mouseX,mouseY,myParent->current(),vw))
	UAye->insert (myParent->current());
    }
    return uc;
}
