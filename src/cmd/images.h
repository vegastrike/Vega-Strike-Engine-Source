#ifndef _IMAGES_H
#define _IMAGES_H

#include <string>
#include <vector>
#include "gfx/vec.h"
#include "container.h"

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
  int quantity;
  std::string content;
  std::string category;
  float price;
  float mass;
  float volume;
  std::string description;
  bool mission;
  float functionality;
  float maxfunctionality;
  Cargo () {mass=0; volume=0;price=0;quantity=1;mission=false;functionality=maxfunctionality=1.0f;}
  Cargo (std::string name, std::string cc, float pp,int qq, float mm, float vv,float func, float maxfunc) {
	quantity=qq;
	content=name;
	category=cc;
	price=pp;
	mass = mm;
	volume=vv;
	mission=false;
        functionality=func;
        maxfunctionality=maxfunc;
  }
  Cargo (std::string name, std::string cc, float pp,int qq, float mm, float vv) {
	quantity=qq;
	content=name;
	category=cc;
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
  bool GetMissionFlag() {return this->mission;}
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
  std::string GetDescription() {return description;}
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
  std::string cockpitImage;
  std::string explosion_type;
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
  ///How much energy cloaking takes per frame
  float cloakenergy;
  ///how fast this starship decloaks/close...if negative, decloaking
  int cloakrate;  //short fix
  ///If this unit cloaks like glass or like fading
  bool cloakglass;
  ///if the unit is a wormhole
  bool forcejump;
  float cargo_volume;///mass just makes you turn worse
  float equipment_volume;///mass just makes you turn worse
  std::vector <Cargo> cargo;
  std::vector <char *> destination;
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
  enum GAUGES {ARMORF,ARMORB,ARMORR,ARMORL,FUEL, SHIELDF,SHIELDR,SHIELDL,SHIELDB, ENERGY, AUTOPILOT,COLLISION,EJECT, LOCK, MISSILELOCK, JUMP, ECM, HULL,WARPENERGY, KPS, SETKPS, COCKPIT_FPS, WARPFIELDSTRENGTH, NUMGAUGES};
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
  unorigdest (Unit * un,Unit * jumppoint, StarSystem * orig, StarSystem * dest, float delay,  int ani, bool justloaded):un(un),jumppoint(jumppoint),orig(orig),dest(dest), delay(delay), animation(ani),justloaded(justloaded),ready(true){}
};

#endif
