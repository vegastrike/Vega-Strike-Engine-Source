struct DockingPorts {
  ///Center
  Vector pos;
  ///Radius from center
  float radius;
  ///axis aligned bounding box min
  Vector min;
  ///bounding box max
  Vector max;
  bool internal;
  bool used;
  DockingPorts (const Vector &pos, float radius, bool internal=true) :
    pos(pos), radius(radius),
    min (pos-Vector (radius,radius,radius)),
    max (pos+Vector (radius,radius,radius)),
    internal(internal),
    used (false)
  {}
  DockingPorts (const Vector &min, const Vector &max, bool internal=true) :
    pos(.5*(min+max)), radius((max-min).Magnitude()*.5),
    min (min),
    max (max),
    internal(internal),
    used (false)
  {}
};
struct DockedUnits {
  UnitContainer uc;
  unsigned int whichdock;
  DockedUnits (Unit * un, unsigned int w): uc(un),whichdock(w) {}
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
  std::vector <DockingPorts> dockingports;
  ///warning unreliable pointer, never dereference!
  std::vector <Unit *> clearedunits;
  std::vector <DockedUnits *> dockedunits;
  UnitContainer DockedTo;
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
