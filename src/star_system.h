/* system.h
 * 
 * Scene management for a star system
 * - Physics simulation
 ***********/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <expat.h>
#include <string>
#include "xml_support.h"
using std::string;
#include "gfx_camera.h"
class Planet;
class UnitCollection;
class ClickList;
class Unit;
class TextPlane;
class InputDFA;
#define NUM_CAM		12
using XMLSupport::AttributeList;
struct Texture;
class Background;
class StarSystem {
 private:
  int numprimaries;
  Unit **primaries; // Stars, planets, etc. Orbital mechanics  // pre-calculated. Cannot be 
  struct StarXML { //Starsystem XML Struct
  int unitlevel;
  vector<Planet *> moons;
  string backgroundname;
  } *xml;
  UnitCollection *drawList;    // Everything to be drawn. Fold missiles in here someday
  UnitCollection *units;    // Objects subject to global physics
  UnitCollection *missiles; // no physics modelling, not searched
                            // through for clicks
  char * name; //system name
  TextPlane *tp;
  Camera cam[NUM_CAM];
  int currentcamera;
  InputDFA * systemInputDFA;
  void modelGravity();
  double time;
  Background * bg;
  Texture *LightMap[6]; //type Texture
 public:
  Background* getBackground() {return bg;}
  void activateLightMap();
  void LoadXML(const char*); //load the xml
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

  Camera *AccessCamera() {return &cam[currentcamera];}
  Camera *AccessCamera(int);
  void SelectCamera(int);
  void SetViewport() {
    cam[currentcamera].UpdateGFX();
  }
  StarSystem(char * filename);
  ~StarSystem();
  UnitCollection * getUnitList();
  ClickList *getClickList(); // returns xy sorted bounding spheres of 
                             // all units in current view

  void AddUnit(Unit *unit);
  void RemoveUnit(Unit *unit);

  void Draw();
  void Update(); // update a simulation atom
};

#endif
