/* planet.h
ok * 
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include <stdio.h>

#include "cmd_unit.h"
#include "cmd_order.h"
#include "physics.h"
#include "UnitCollection.h"
#include "iterator.h"
struct GFXMaterial;
/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

struct Texture;
class Planet;
typedef float Matrix[16];

class PlanetaryOrbit : public Order {
 private:
  Unit *parent;
  double velocity;
  double theta;

  Vector x_size;
  Vector y_size;
  Vector focus;

 public:

  PlanetaryOrbit(Unit *p, double velocity, double initpos, const Vector &x_axis, const Vector &y_axis, const Vector &Centre, Unit * target=NULL); 
  ~PlanetaryOrbit();
  void Execute();

};

class Planet : public Unit {
 private:
  float radius;
  float gravity;
  vector <char *> destination;
  UnitCollection satellites;

 public:
  Planet();
  void endElement();
  void beginElement(Vector x,Vector y,float vely,float velx,float gravity,float radius,char * filename,char * alpha,vector<char *>dest,int level, const GFXMaterial &ourmat, bool isunit=false);
  Planet(Vector x,Vector y,float vely,float velx,float gravity,float radius,char * filename,char *alpha,vector<char *> dest, const Vector &orbitcent, Unit * parent, const GFXMaterial & ourmat);
  ~Planet();
  virtual enum clsptr isUnit() {return PLANETPTR;}

//  void InitPlanet(FILE *fp);
  virtual void Kill();
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
	
      Unit *currentPlanet = pos->current();
	  if (currentPlanet->isUnit()==PLANETPTR) {
	    UnitCollection::UnitIterator * tmp;
	    for(tmp = ((Planet *)currentPlanet)->satellites.createIterator(); tmp->current()!=NULL; tmp->advance()) {
			planetStack.append(tmp->current());
		}
	    delete tmp;
	  }
      return pos->advance();
    }
  };
  Iterator *createIterator() { return new PlanetIterator(this);}

  friend class Planet::PlanetIterator;
  friend class PlanetaryOrbit;
};

#endif

