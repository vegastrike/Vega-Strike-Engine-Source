#ifndef _IMAGES_H
#define _IMAGES_H

#include <string>
#include <vector>
#include "gfx/vec.h"
#include "container.h"
#include "../SharedPool.h"

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
  float minsize;
  DockingPorts(){}
  DockingPorts (const Vector &pos, float radius, float minradius, bool internal){
    this->pos=(pos); this->radius=(radius);
    min=Vector(pos-Vector (radius,radius,radius));
    max=Vector(pos+Vector (radius,radius,radius));
    this->internal=(internal);
    used=false;
    this->minsize=minradius;
  }
  DockingPorts (const Vector &min, const Vector &max, float minradius, bool internal):radius((max-min).Magnitude()*.5) {
	pos = ((float).5)*(min+max);
	
    this->min =(min);
    this->max =(max);
    this->internal=(internal);
    this->used= (false);
    this->minsize=minradius;
  }
};
struct DockedUnits {
  UnitContainer uc;
  unsigned int whichdock;
  DockedUnits (Unit * un, unsigned int w): uc(un),whichdock(w) {}
};


class Cargo {
public:
  StringPool::Reference content;
  StringPool::Reference category;
  StringPool::Reference description;

  int quantity;
  float price;
  float mass;
  float volume;
  bool mission;
  float functionality;
  float maxfunctionality;
  Cargo ()
  {
	mass=0; volume=0;price=0;quantity=1;mission=false;
	functionality=maxfunctionality=1.0f;
  }
  Cargo (std::string name, std::string cc, float pp,int qq, float mm, float vv,float func, float maxfunc) :
	content(name),
	category(cc)
  {
	quantity=qq;
	price=pp;
	mass = mm;
	volume=vv;
	mission=false;
	functionality=func;
	maxfunctionality=maxfunc;
  }
  Cargo (std::string name, std::string cc, float pp,int qq, float mm, float vv) :
	content(name),
	category(cc)
  {
	quantity=qq;
	price=pp;
	mass = mm;
	volume=vv;
	mission=false;    
  }
  float GetFunctionality() {
    return functionality;
  }
  float GetMaxFunctionality() {
    return maxfunctionality;
  }
  void SetFunctionality(float func) {
    functionality=func;
  }
  void SetMaxFunctionality(float func) {
    maxfunctionality=func;
  }
  void SetMissionFlag(bool flag){this->mission=flag;}
  void SetPrice (float price) {this->price=price;}
  void SetMass (float mass) {this->mass=mass;}
  void SetVolume (float vol) {this->volume=vol;}
  void SetQuantity (int quantity) {this->quantity=quantity;}
  void SetContent (const std::string &content) {this->content = content;}
  void SetCategory (const std::string &category) {this->category = category;}

  bool GetMissionFlag() const {return this->mission;}
  const std::string& GetCategory () const {return category;}
  const std::string& GetContent () const {return content;}
  const std::string& GetDescription() const {return description;}
  std::string GetCategoryPython() {return GetCategory();}
  std::string GetContentPython() {return GetContent();}
  std::string GetDescriptionPython() {return GetDescription();}
  int GetQuantity() const {return quantity;}
  float GetVolume () const {return volume;}
  float GetMass() const {return mass;}
  float GetPrice () const {return price;}
  bool operator == (const Cargo & other) const {
    return content==other.content;
  }
  bool operator < (const Cargo &other)const {
    return (category==other.category)?(content<other.content):(category<other.category);
  }
};
class Box;
class VSSprite;
class Animation;

struct UnitImages {
  UnitImages() {
    VSCONSTRUCT1('i')
  }
  ~UnitImages() {
    VSDESTRUCT1
  }
  StringPool::Reference cockpitImage;
  StringPool::Reference explosion_type;
  Vector CockpitCenter;
  VSSprite * hudImage;
  ///The explosion starts at null, when activated time explode is incremented and ends at null  
  Animation *explosion; 
  float timeexplode;
  //  Box *selectionBox;
  float *cockpit_damage; //0 is radar, 1 to MAXVDU is vdus and >MAXVDU is gauges
  ///how likely to fool missiles
  int ecm; //short fix
  ///holds the info for the repair bot type. 0 is no bot;
  unsigned char repair_droid;
  float next_repair_time;
  unsigned int next_repair_cargo; //(~0 : select randomly)
  ///How much energy cloaking takes per frame
  float cloakenergy;
  ///how fast this starship decloaks/close...if negative, decloaking
  int cloakrate;  //short fix
  ///If this unit cloaks like glass or like fading
  bool cloakglass;
  ///if the unit is a wormhole
  bool forcejump;
  float UpgradeVolume;
  float CargoVolume;///mass just makes you turn worse
  float equipment_volume;//this one should be more general--might want to apply it to radioactive goods, passengers, ships (hangar), etc
  float HiddenCargoVolume;
  std::vector <Cargo> cargo;
  std::vector <string> destination;
  std::vector <DockingPorts> dockingports;
  ///warning unreliable pointer, never dereference!
  std::vector <Unit *> clearedunits;
  std::vector <DockedUnits *> dockedunits;
  UnitContainer DockedTo;
  float unitscale;//for output
  class XMLSerializer *unitwriter;
  float fireControlFunctionality;
  float fireControlFunctionalityMax;
  float SPECDriveFunctionality;
  float SPECDriveFunctionalityMax;
  float CommFunctionality;
  float CommFunctionalityMax;
  float LifeSupportFunctionality;
  float LifeSupportFunctionalityMax;
  enum GAUGES {
      // Image-based gauges
    ARMORF,ARMORB,ARMORR,ARMORL,ARMOR4,ARMOR5,ARMOR6,ARMOR7,FUEL,SHIELDF,SHIELDR,SHIELDL,SHIELDB, SHIELD4,SHIELD5,SHIELD6,SHIELD7,
      ENERGY,AUTOPILOT,COLLISION,EJECT,LOCK,MISSILELOCK,JUMP,ECM,HULL,WARPENERGY, 
      KPS, // KEEP KPS HERE - it marks the start of text-based gauges
      SETKPS,COCKPIT_FPS,WARPFIELDSTRENGTH,MAXKPS,MAXCOMBATKPS,MAXCOMBATABKPS,
      NUMGAUGES // KEEP THIS LAST - obvious reasons, marks the end of all gauges
  };
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

// From star_system_jump.cpp
class StarSystem;
struct unorigdest {
  UnitContainer un;
  UnitContainer jumppoint;
  StarSystem * orig;
  StarSystem * dest;
  float delay;
  int animation;
  bool justloaded;
  bool ready;
  QVector final_location;
  unorigdest (Unit * un,Unit * jumppoint, StarSystem * orig, StarSystem * dest, float delay,  int ani, bool justloaded, QVector use_coordinates/*set to 0,0,0 for crap*/):un(un),jumppoint(jumppoint),orig(orig),dest(dest), delay(delay), animation(ani),justloaded(justloaded),ready(true), final_location(use_coordinates){}
};

#endif
