/* system.h
 * 
 * Scene management for a star system
 * - Physics simulation
 ***********/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_



#include "gfx_camera.h"

class Planet;
class UnitCollection;
class ClickList;
class Unit;
#define NUM_CAM		12

class StarSystem {
 private:
  Planet *primaries; // Stars, planets, etc. Orbital mechanics
                   // pre-calculated. Cannot be 

  UnitCollection *drawList;    // Everything to be drawn. Fold missiles in here someday
  UnitCollection *units;    // Objects subject to global physics
  UnitCollection *missiles; // no physics modelling, not searched
                            // through for clicks
  
  Camera cam[NUM_CAM];
  int currentcamera;

  void modelGravity();
 public:
  Camera *AccessCamera() {return &cam[currentcamera];}
  Camera *AccessCamera(int);
  void SelectCamera(int);
  void SetViewport() {
    cam[currentcamera].UpdateGFX();
  }
  StarSystem(Planet *primaries);
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
