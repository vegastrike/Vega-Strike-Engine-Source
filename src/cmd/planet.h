/* planet.h
ok * 
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include <stdio.h>

#include "unit.h"
#include "ai/order.h"
//#include "physics.h"
#include "collection.h"
#include "iterator.h"
#include <vector>

struct GFXMaterial;
/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

struct Texture;
class Atmosphere;
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
 protected:
  ///A vector containing all lihgts currently activated on current planet
  std::vector <int> lights;

 public:

  PlanetaryOrbit(Unit *p, double velocity, double initpos, const Vector &x_axis, const Vector &y_axis, const Vector &Centre, Unit *target=NULL); 
  ~PlanetaryOrbit();
  void Execute();

};

struct GFXLightLocal {
  GFXLight ligh;
  bool islocal;
};
class ContinuousTerrain;
class PlanetaryTransform;
class Planet : public Unit {
 private:
  PlanetaryTransform * terraintrans;
  Atmosphere * atmosphere;
  ContinuousTerrain *terrain;
  Vector TerrainUp;
  Vector TerrainH;
  bool inside;
  float radius;
  float gravity;
  vector <char *> destination;
  UnitCollection satellites;
  UnitCollection insiders;
  std::vector <int> lights;
 public:
  Planet();
  void DisableLights ();
  void EnableLights();
  void AddSatellite (Unit * orbiter);
  void endElement();
  void beginElement(Vector x,Vector y,float vely,float velx,float gravity,float radius,char * filename,char * alpha,vector<char *>dest,int level, const GFXMaterial &ourmat, const std::vector <GFXLightLocal> &ligh, bool isunit, int faction,string fullname);
  Planet(Vector x,Vector y,float vely,float velx,float gravity,float radius,char * filename,char *alpha,vector<char *> dest, const Vector &orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &, int faction,string fullname);
  Planet * GetTopPlanet (int level);
  ~Planet();
  virtual enum clsptr isUnit() {return PLANETPTR;}
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix m = identity_matrix);
  void DrawTerrain();
  static void ProcessTerrains();
//  void InitPlanet(FILE *fp);
  virtual void Kill();
  const vector <char *> &GetDestinations () {return destination;}
  PlanetaryTransform*  setTerrain (ContinuousTerrain *, float ratiox, int numwraps); 
  ContinuousTerrain * getTerrain(PlanetaryTransform *&t) {t = terraintrans;return terrain;}
  void setAtmosphere (Atmosphere *);
  Atmosphere * getAtmosphere () {return atmosphere;}
  void reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal,  float dist);
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

  bool hasLights() { return !lights.empty(); }
  const std::vector <int> & activeLights() { return lights; }

  friend class Planet::PlanetIterator;
  friend class PlanetaryOrbit;
};

#endif


