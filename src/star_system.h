/* system.h
 * 
 * Scene management for a star system
 * - Physics simulation
 ***********/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define NUM_CAM 10

#include "gfx_camera.h"

class Planet;
class UnitCollection;
class ClickList;
class Unit;

class StarSystem {
 private:
  Planet *primaries; // Stars, planets, etc. Orbital mechanics
                   // pre-calculated. Cannot be 

  UnitCollection *units;    // Objects subject to global physics
  UnitCollection *missiles; // no physics modelling, not searched
                            // through for clicks

  Camera cam[NUM_CAM];
  int active_camera;

 public:
  StarSystem(Planet *primaries);

  ClickList *getClickList(); // returns xy sorted bounding spheres of 
                             // all units in current view

  void AddUnit(Unit *unit);
  void RemoveUnit(Unit *unit);

  void modelPhysics();
  void Draw();
};

#endif
