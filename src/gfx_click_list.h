#include "vegastrike.h"
#include "gfx_transform_matrix.h"
#include "UnitCollection.h"

Vector MouseCoordinate (int x, int y, float zplane); //finds the vector of the location of the mouse cursor on the plane 'z' away from the camera

class ClickList {
 private:
  UnitCollection *parentIter;
 public:

  bool queryShip (int mouseX,int mouseY, Unit *, float [16]);//returns if the ship's in iterator utilizes
  ClickList (UnitCollection *parentIter);
  UnitCollection * requestIterator (int mouseX, int mouseY);
};
