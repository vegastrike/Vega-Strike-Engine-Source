
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
  DockingPorts(){}
  DockingPorts (const Vector &pos, float radius, bool internal=true){
	this->pos=(pos); this->radius=(radius);
    min=Vector(pos-Vector (radius,radius,radius));
    max=Vector(pos+Vector (radius,radius,radius));
    internal=(internal);
    used=false;
  }
  DockingPorts (const Vector &min, const Vector &max, bool internal=true) :
  pos(.5*(min+max)), radius((max-min).Magnitude()*.5) {
    this->min =(min);
    this->max =(max);
    this->internal=(internal);
    this->used= (false);
  }
};
struct DockedUnits {
  UnitContainer uc;
  unsigned int whichdock;
  DockedUnits (Unit * un, unsigned int w): uc(un),whichdock(w) {}
};


struct Cargo {
  int quantity;
  std::string content;
  std::string category;
  float price;
  float mass;
  float volume;
  Cargo () {mass=0; volume=0;price=0;quantity=1;}
  bool operator == (const Cargo & other) const {
    return content==other.content;
  }
  bool operator < (const Cargo &other)const {
    return (category==other.category)?(content<other.content):(category<other.category);
  }
};

struct UnitImages {
  std::string cockpitImage;
  Vector CockpitCenter;
  Sprite * hudImage;
  ///The explosion starts at null, when activated time explode is incremented and ends at null  
  Animation *explosion; 
  float timeexplode;
  Box *selectionBox;

  float *cockpit_damage; //0 is radar, 1 to MAXVDU is vdus and >MAXVDU is gauges
  ///how likely to fool missiles
  float ecm;
  ///holds the info for the repair bot type. 0 is no bot;
  unsigned char repair_droid;
  ///How much energy cloaking takes per frame
  float cloakenergy;
  ///how fast this starship decloaks/close...if negative, decloaking
  short cloakrate;
  ///If this unit cloaks like glass or like fading
  bool cloakglass;
  ///if the unit is a wormhole
  bool forcejump;
  float cargo_volume;///mass just makes you turn worse
  std::vector <Cargo> cargo;
  std::vector <char *> destination;
  std::vector <DockingPorts> dockingports;
  ///warning unreliable pointer, never dereference!
  std::vector <Unit *> clearedunits;
  std::vector <DockedUnits *> dockedunits;
  UnitContainer DockedTo;
  class XMLSerializer *unitwriter;
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
