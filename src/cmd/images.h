struct Dock {
  Vector pos;
  float radius;
  Dock (const Vector &pos, float radius) :pos(pos), radius(radius){}
};
struct UnitImages {
  std::string cockpitImage;
  Vector CockpitCenter;
  Sprite * hudImage;
  ///The explosion starts at null, when activated time explode is incremented and ends at null  
  Animation *explosion; 
  float timeexplode;
  Box *selectionBox;
  ///how fast this starship decloaks/close...if negative, decloaking
  short cloakrate;
  ///How much energy cloaking takes per frame
  float cloakenergy;
  bool cloakglass;
  std::vector <Dock> docks;

};

struct UnitSounds {
  int engine;
  int shield;
  int armor;
  int hull;
  int explode;
  int cloak;
  int jump;
};
