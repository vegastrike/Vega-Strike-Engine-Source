#include "vegastrike.h"
#include "gfx_transform_matrix.h"
#include "UnitCollection.h"
#include "star_system.h"
Vector MouseCoordinate (int x, int y, float zplane); //finds the vector of the location of the mouse cursor on the plane 'z' away from the camera

class ClickList {
 private:
  UnitCollection *parentIter;
  StarSystem *parentSystem;
 public:
  //gets passed in unnormalized mouse values btw 0 and g_game.x_resolution&& g_game.y_resolution
  bool queryShip (int mouseX,int mouseY, Unit *);//returns if the ship's in iterator utilizes
  ClickList (StarSystem *parSystem , UnitCollection *parentIter);
  ~ClickList ();
  UnitCollection * requestIterator (int mouseX, int mouseY);
  UnitCollection * requestIterator (int minX,int minY, int maxX, int maxY);
};
