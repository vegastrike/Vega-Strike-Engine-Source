Vector MouseCoordinate(int x, int y, float zplane); //finds the vector of the location of the mouse cursor on the plane 'z' away from the camera

class ClickList
{
 private:
  UnitCollection *parentIter;
  StarSystem *parentSys;
 public:

  bool queryShip (int mouseX,int mouseY);//returns if the ship's in iterator utilizes
  Clicklist (UnitCollection *parentIter, StarSystem *parentSystem);
  UnitCollection * requestIterator (int mouseX, int mouseY);
};
