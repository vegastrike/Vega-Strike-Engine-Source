#ifndef _GENERICSYSTEM_H_
#define _GENERICSYSTEM_H_

#include <expat.h>
#include <string>
#include "xml_support.h"
using std::string;
#include "gfx/vec.h"
#include "cmd/collection.h"
class Stars;
//#include "planet_generic.h"
class Planet;
class ClickList;
class Unit;
class Background;
class Terrain;
class ContinuousTerrain;
//class TextPlane;
struct AtmosphericFogMesh {
	std::string meshname;
	double scale;
	float er;float eg; float eb; float ea;
	float dr;float dg; float db; float da;	
	double focus;
	double concavity;
	int tail_mode_start;
	int tail_mode_end;
	int min_alpha;
	int max_alpha;
	AtmosphericFogMesh ();
};

using XMLSupport::AttributeList;
//class Atmosphere;
/**
 * Star System
 * Scene management for a star system
 * Per-Frame Drawing & Physics simulation
 **/
class StarSystem {
 public:
  StarSystem();
  StarSystem(const char * filename, const Vector & centroid=Vector (0,0,0), const float timeofyear=0);
  virtual ~StarSystem();
 protected:
  ///Physics is divided into 3 stages spread over 3 frames
  enum PHYSICS_STAGE {PHY_AI,TERRAIN_BOLT_COLLIDE,MISSION_SIMULATION,PHY_COLLIDE, PHY_TERRAIN, PHY_RESOLV,PHY_NUM} current_stage;
  
  ///Stars, planets, etc. Orbital mechanics precalculated 

 private:
  ///Starsystem XML Struct For use with XML loading
  struct StarXMLstring {
    //Terrain * parentterrain;
    //ContinuousTerrain * ct;
    int unitlevel;
    //std::vector <GFXLight> lights;
    std::vector <string> moons;
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
	std::vector <AtmosphericFogMesh> fog;
  } *xml_string;

 protected:
  //std::vector <Terrain *> terrains;
  //std::vector <ContinuousTerrain *>contterrains;
  /// Everything to be drawn. Folded missiles in here oneday
  UnitCollection drawList; 
  /// Objects subject to global gravitron physics (disabled)   
  UnitCollection units;    
  unsigned char no_collision_time;
  ///system name             
  char * name; 
  std::string filename;
  ///Process global clicks for input/output
  //InputDFA * systemInputDFA;
  ///to track the next given physics frame
  double time;
  ///The background associated with this system
  //Background * bg;
  ///The moving, fading stars
  Stars *stars;
  ///The Light Map corresponding for the BP for spheremapping
  //Texture *LightMap[6]; 
  void AddStarsystemToUniverse(const string &filename);
  void RemoveStarsystemFromUniverse();
  int lightcontext;
  std::vector <class MissileEffect *> dischargedMissiles;
 public:
  virtual void AddMissileToQueue(class MissileEffect *);
  virtual void UpdateMissiles();
  void UpdateUnitPhysics(bool firstframe);
  virtual void activateLightMap() {}
  ///activates the light map texture
  virtual unsigned int numTerrain () {return 0;}
  virtual unsigned int numContTerrain () {return 0;}
  ///Draws a frame of action, interpolating between physics frames
  virtual void Draw(bool DrawCockpit=true) {}
  /// update a simulation atom ExecuteDirector must be false if star system is just loaded before mission is loaded
  void Update(float priority, bool executeDirector);
  // This one is temporarly used on server side
  void Update(float priority);
  ///re-enables the included lights and terrains
  virtual void SwapIn () {}
  ///Disables included lights and terrains
  virtual void SwapOut () {}
  virtual Terrain * getTerrain (unsigned int which) {return NULL;}
  virtual ContinuousTerrain * getContTerrain (unsigned int which) {return NULL;}
  virtual Background* getBackground() {return NULL;}
  virtual ClickList *getClickList() { return NULL;} 

  void ExecuteUnitAI();
  class CollideTable *collidetable;
  class bolt_draw *bolts;
  virtual void LoadXML(const char*, const Vector & centroid, const float timeofyear) {}
  static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
  static void endElement(void *userData, const XML_Char *name);
  virtual void beginElement(const string &name, const AttributeList &attributes) {}
  virtual void endElement(const string &name) {}
  string getFileName();
  string getName();
  ///Loads the star system from an XML file
  UnitCollection& getUnitList() {return drawList;}
  UnitCollection& gravitationalUnits() {return units;}
  /// returns xy sorted bounding spheres of all units in current view
  ///Adds to draw list
  void AddUnit(Unit *unit);
  ///Removes from draw list
  bool RemoveUnit(Unit *unit);
  virtual bool JumpTo (Unit * unit, Unit * jumppoint, const std::string &system);
  static void ProcessPendingJumps ();
  virtual void VolitalizeJumpAnimation(const int ani){}
  virtual void DoJumpingComeSightAndSound(Unit * un){}
  virtual int DoJumpingLeaveSightAndSound (Unit * un){return -1;}
  //friend class Atmosphere;
};
bool PendingJumpsEmpty();
#endif

