#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <expat.h>
#include <string>
#include "xml_support.h"
using std::string;
#include "gfx/camera.h"
class Stars;
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


/**
 * Star System
 * Scene management for a star system
 * Per-Frame Drawing & Physics simulation
 **/
class StarSystem {
 private:
  ///Physics is divided into 3 stages spread over 3 frames
  enum PHYSICS_STAGE {PHY_AI,TERRAIN_BOLT_COLLIDE,PHY_COLLIDE, PHY_TERRAIN, PHY_RESOLV,PHY_NUM} current_stage;
  
  ///Stars, planets, etc. Orbital mechanics precalculated 
  int numprimaries; Unit **primaries; 
  ///Starsystem XML Struct For use with XML loading
  struct StarXML { 
  int unitlevel;
    std::vector <GFXLight> lights;
    std::vector <Planet *> moons;
    string backgroundname;
    Vector systemcentroid;
    Vector cursun;
    float reflectivity;
    int numnearstars;
    int numstars;
    bool fade;
    float starsp;
  } *xml;
  /// Everything to be drawn. Folded missiles in here oneday
  UnitCollection *drawList; 
/// Objects subject to global gravitron physics (disabled)   
  UnitCollection *units;    
  /// no physics modelling, not searched through for clicks (deprecated)
  UnitCollection *missiles; 
  ///system name             
  char * name; 
  ///All avail cameras in this system
  Camera cam[NUM_CAM];
  int currentcamera;
  ///Process global clicks for input/output
  InputDFA * systemInputDFA;
  ///to track the next given physics frame
  double time;
  ///The background associated with this system
  Background * bg;
  ///The moving, fading stars
  Stars *stars;
  ///The Light Map corresponding for the BP for spheremapping
  Texture *LightMap[6]; 
 public:
  Background* getBackground() {return bg;}
  ///activates the light map texture
  void activateLightMap();
  void LoadXML(const char*, const Vector & centroid);
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);

  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);
  ///Accesses the current camera
  Camera *AccessCamera() {return &cam[currentcamera];}
  ///Returns the passed in cam
  Camera *AccessCamera(int);
  ///Changes current camera to selected camera
  void SelectCamera(int);
  ///GFXLoadMatrix proper camera
  void SetViewport() {
    cam[currentcamera].UpdateGFX();
  }
  ///Loads the star system from an XML file
  StarSystem(char * filename, const Vector & centroid=Vector (0,0,0), const string planetname=string());
  ~StarSystem();
  UnitCollection * getUnitList();
  /// returns xy sorted bounding spheres of all units in current view
  ClickList *getClickList(); 
  ///Adds to draw list
  void AddUnit(Unit *unit);
  ///Removes from draw list
  void RemoveUnit(Unit *unit);
  ///Draws a frame of action, interpolating between physics frames
  void Draw();
  /// update a simulation atom
  void Update(); 
};

#endif

