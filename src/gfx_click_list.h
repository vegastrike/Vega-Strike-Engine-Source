class ClickIterator {


} 

class ClickList: UnitIterator
{
 private:
  iterator * UnitCollection;

 public:
  boolean queryShip (int mouseX,int mouseY);//returns if the ship's in iterator utilizes
  Clicklist (UnitCollection *parentIter, StarSystem *parentSystem);
  UnitCollection * requestIterator (int mouseX, int mouseY);
};
