#ifndef _GFX_CLICK_LIST_H_
#define _GFX_CLICK_LIST_H_

//#include "vegastrike.h"
#include "gfx/matrix.h"
#include "UnitCollection.h"
#include "star_system.h"
Vector MouseCoordinate (int x, int y);//FIXME


class ClickList {
 private:
  UnitCollection *parentIter;
  StarSystem *parentSystem;
  UnitCollection * lastCollection;
  Unit * lastSelected;
 public:
  //gets passed in unnormalized mouse values btw 0 and g_game.x_resolution&& g_game.y_resolution
  bool queryShip (int mouseX,int mouseY, Unit *);//returns if the ship's in iterator utilizes
  ClickList (StarSystem *parSystem , UnitCollection *parentIter);
  ~ClickList (){}
  UnitCollection * requestIterator (int mouseX, int mouseY);
  UnitCollection * requestIterator (int minX,int minY, int maxX, int maxY);
  Unit * requestShip (int mouseX, int mouseY);
};
#endif
