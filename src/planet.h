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

struct Texture;
class Planet;
typedef float Matrix[16];

class PlanetaryOrbit : public AI {
 private:
  Planet *parent;
  double velocity;
  double theta;

  Vector x_size;
  Vector y_size;
  Vector focus;

 public:
  PlanetaryOrbit(Planet *p, double velocity, double initpos, const Vector &x_axis, const Vector &y_axis) : parent(p), velocity(velocity), theta(initpos), x_size(x_axis), y_size(y_axis) { 
    double delta = x_size.Magnitude() - y_size.Magnitude();
    if(delta == 0) {
      focus = Vector(0,0,0);
    }
    else if(delta>0) {
      focus = x_size*(delta/x_size.Magnitude());
    } else {
      focus = y_size*(-delta/y_size.Magnitude());
    }
  }
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

  /*  virtual void Draw();
  virtual void DrawStreak(const Vector &v);
  virtual void Draw(Matrix tmatrix);
  virtual void Draw(Matrix tmatrix, const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos);
  */
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
  Iterator *createIterator() { return new PlanetIterator(this);}

  friend Planet::PlanetIterator;
  friend PlanetaryOrbit;
};

#endif

