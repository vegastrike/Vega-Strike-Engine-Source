
struct UnitImages {
  std::string cockpitImage;
  Vector CockpitCenter;
  Sprite * hudImage;
  ///The explosion starts at null, when activated time explode is incremented and ends at null  
  Animation *explosion; 
  float timeexplode;
  Box *selectionBox;
};

struct UnitSounds {
    int engine;
    int shield;
    int armor;
    int hull;
    int explode;
    int cloak;
  };
