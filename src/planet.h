/* planet.h
ok * 
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include "cmd_unit.h"
#include "cmd_ai.h"
#include "physics.h"
#include "UnitCollection.h"
#include "iterator.h"

/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

class Texture;
class Planet;
typedef float Matrix[16];

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
  float gravity;

  Vector origin;

  Planet **satellites;
  int numSatellites;

 public:
  Planet();
  Planet(char *filename);
  ~Planet();

  void InitPlanet(FILE *fp);

  virtual void Draw();
  virtual void DrawStreak(const Vector &v);
  virtual void Draw(Matrix tmatrix);
  virtual void Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);

  void gravitate(UnitCollection *units, Matrix t);
  void gravitate(UnitCollection *units);

  class PlanetIterator : public Iterator {
    UnitCollection planetStack;
    Iterator *pos;
  public:
    PlanetIterator(Planet *p) : planetStack() { 
      planetStack.append(p);
      pos = planetStack.createIterator();
    }
    virtual ~PlanetIterator() {
      delete pos;
    }

    virtual void preinsert(Unit *unit) {
      abort();
    }
    virtual void postinsert(Unit *unit) {
      abort();
    }
    virtual void remove() {
      abort();
    }
    virtual Unit *current() {
      return pos->current();
    }
    virtual Unit *advance() {
      if(pos->current()==NULL)
	return NULL;

      Planet *currentPlanet = (Planet*)pos->current();
      for(int a=0; a<currentPlanet->numSatellites; a++) {
	planetStack.append(currentPlanet->satellites[a]);
      }
      return pos->advance();
    }
  };
  friend Planet::PlanetIterator;
  
  Iterator *createIterator() { return new PlanetIterator(this);}
};

#endif

