
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
  DockingPorts (const Vector &min, const Vector &max, bool internal=true):radius((max-min).Magnitude()*.5) {
	pos = ((float).5)*(min+max);
	
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


class Cargo {
public:
  int quantity;
  std::string content;
  std::string category;
  float price;
  float mass;
  float volume;
  char * description;//mem leak... for the master part list (global var, no prob)
  Cargo () {mass=0; volume=0;price=0;quantity=1;description=NULL;}
  Cargo (std::string name, std::string cc, float pp,int qq, float mm, float vv) {
	quantity=qq;
	content=name;
	category=cc;
	price=pp;
	mass = mm;
	volume=vv;
	description=NULL;
  }
  void SetPrice (float price) {this->price=price;}
  void SetMass (float mass) {this->mass=mass;}
  void SetVolume (float vol) {this->volume=vol;}
  void SetQuantity (int quantity) {this->quantity=quantity;}
  void SetContent (std::string content) {this->content = content;}
  void SetCategory (std::string category) {this->category = category;}
  std::string GetCategory () {return category;}
  std::string GetContent () {return content;}
  int GetQuantity() {return quantity;}
  float GetVolume () {return volume;}
  float GetMass() {return mass;}
  float GetPrice () {return price;}
  std::string GetDescription() {if (description!=NULL) {return description;}return std::string("");}
  bool operator == (const Cargo & other) const {
    return content==other.content;
  }
  bool operator < (const Cargo &other)const {
    return (category==other.category)?(content<other.content):(category<other.category);
  }
};

struct UnitImages {
  std::string cockpitImage;
  std::string explosion_type;
  Vector CockpitCenter;
  Sprite * hudImage;
  ///The explosion starts at null, when activated time explode is incremented and ends at null  
  Animation *explosion; 
  float timeexplode;
  Box *selectionBox;

  float *cockpit_damage; //0 is radar, 1 to MAXVDU is vdus and >MAXVDU is gauges
  ///how likely to fool missiles
  short ecm;
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
  float unitscale;//for output
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
