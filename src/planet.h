/* planet.h
ok * 
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include "cmd_unit.h"
#include "cmd_ai.h"
#include "physics.h"

/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

class Texture;
class Planet;

class PlanetaryOrbit : public AI {
 protected:
  double radius;
  double angular_delta;
  double theta;
 public:
  PlanetaryOrbit(double radius, double velocity, double initpos) : radius(radius), angular_delta(velocity/radius * SIMULATION_ATOM), theta(initpos) { }
  AI *Execute();

  friend Planet;
};

class Planet : public Unit {
 private:
  float radius;
 public:
  Planet(FILE *fp);
  Planet(char *filename);
  ~Planet();
};

#endif

