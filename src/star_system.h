#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include <expat.h>
#include <string>
#include "star_system_generic.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"
using std::string;
class ClickList;
class TextPlane;
class InputDFA;


using XMLSupport::AttributeList;
struct Texture;
class Background;
class Terrain;
class ContinuousTerrain;
class Atmosphere;
/**
 * Star System
 * Scene management for a star system
 * Per-Frame Drawing & Physics simulation
 **/
class GameStarSystem: public StarSystem {
 private:
  ///Starsystem XML Struct For use with XML loading
  struct StarXML {
    Terrain * parentterrain;
    ContinuousTerrain * ct;
    int unitlevel;
    std::vector <GFXLight> lights;
    std::vector <Planet *> moons;
    string backgroundname;
    Vector systemcentroid;
    Vector cursun;
    float timeofyear;
    float reflectivity;
    int numnearstars;
    int numstars;
    bool fade;
    float starsp;
    float scale;
	vector <AtmosphericFogMesh> fog;
  } *xml;
  void LoadXML(const char*, const Vector & centroid, const float timeofyear);
  void beginElement(const string &name, const AttributeList &attributes);
  void endElement(const string &name);

  std::vector <Terrain *> terrains;
  std::vector <ContinuousTerrain *>contterrains;
  /// Objects subject to global gravitron physics (disabled)   
  ///Process global clicks for input/output
  InputDFA * systemInputDFA;
  ///The background associated with this system
  Background * bg;
  ///The Light Map corresponding for the BP for spheremapping
  Texture *LightMap[6]; 
  //vector <class MissileEffect *> dischargedMissiles;
 public:
  GameStarSystem::GameStarSystem(const char * filename, const Vector & centr=Vector(0,0,0),const float timeofyear=0);
  ~GameStarSystem();
  //void UpdateUnitPhysics(bool firstframe);
  //class CollideTable *collidetable;
  //class bolt_draw *bolts;
  Background* getBackground() {return bg;}
  ///activates the light map texture
  void activateLightMap();
  static void DrawJumpStars();
  Terrain * getTerrain (unsigned int which) {return terrains[which];}
  unsigned int numTerrain () {return terrains.size();}
  ContinuousTerrain * getContTerrain (unsigned int which) {return contterrains[which];}
  unsigned int numContTerrain () {return contterrains.size();}
  ///Loads the star system from an XML file
  /// returns xy sorted bounding spheres of all units in current view
  ClickList *getClickList(); 
  ///Adds to draw list
  ///Draws a frame of action, interpolating between physics frames
  void Draw(bool DrawCockpit=true);
  /// update a simulation atom ExecuteDirector must be false if star system is just loaded before mission is loaded
  void Update(float priority, bool executeDirector); 
  ///re-enables the included lights and terrains
  void SwapIn ();
  ///Disables included lights and terrains
  void SwapOut ();
  //  bool JumpTo (Unit * unit, Unit * jumppoint, const std::string &system);
  virtual void VolitalizeJumpAnimation (const int ani);
  virtual void DoJumpingComeSightAndSound (Unit * un);
  virtual int DoJumpingLeaveSightAndSound (Unit * un);
  friend class Atmosphere;
};

#endif

