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

class PlanetaryOrbit : public Order {
 private:
  Unit *parent;
  double velocity;
  double theta;

  QVector x_size;
  QVector y_size;
  QVector focus;
 protected:
  ///A vector containing all lihgts currently activated on current planet
  std::vector <int> lights;

 public:

  PlanetaryOrbit(Unit *p, double velocity, double initpos, const QVector &x_axis, const QVector &y_axis, const QVector &Centre, Unit *target=NULL); 
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
  bool atmospheric;//then users can go inside!
  float radius;
  float gravity;
  UnitCollection satellites;
  UnitCollection insiders;
  std::vector <int> lights;
  Animation *shine;

protected:
    /// default constructor - only to be called by UnitFactory
  Planet();

    /// constructor - only to be called by UnitFactory
    Planet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,char * filename,char *alpha,vector<char *> dest, const QVector &orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &, int faction,string fullname);

    friend class UnitFactory;

public:
  virtual ~Planet();

  void DisableLights ();
  void EnableLights();
  void AddSatellite (Unit * orbiter);
  void endElement();
  void beginElement(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,char * filename,char * alpha,vector<char *>dest,int level, const GFXMaterial &ourmat, const std::vector <GFXLightLocal> &ligh, bool isunit, int faction,string fullname);
  Planet * GetTopPlanet (int level);
  virtual enum clsptr isUnit() {return PLANETPTR;}
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix);
  void DrawTerrain();
  static void ProcessTerrains();
//  void InitPlanet(FILE *fp);
  virtual void Kill(bool erasefromsave=false);

  PlanetaryTransform*  setTerrain (ContinuousTerrain *, float ratiox, int numwraps, float scaleatmos); 
  ContinuousTerrain * getTerrain(PlanetaryTransform *&t) {t = terraintrans;return terrain;}
  void setAtmosphere (Atmosphere *);
  Atmosphere * getAtmosphere () {return atmosphere;}
  void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal,  float dist);
  void gravitate(UnitCollection *units);

  class PlanetIterator : public Iterator {
    UnitCollection planetStack;
    un_iter *pos;
  public:
    PlanetIterator(Planet *p) : planetStack() { 
      planetStack.append(p);
      pos = new un_iter (planetStack.createIterator());
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
    virtual void advance() {
      if(pos->current()==NULL)
	return;
	
      Unit *currentPlanet = pos->current();
	  if (currentPlanet->isUnit()==PLANETPTR) {
	    
	    for(un_iter tmp (((Planet *)currentPlanet)->satellites.createIterator()); tmp.current()!=NULL; tmp.advance()) {
			planetStack.append(tmp.current());
		}
	  }
      pos->advance();
    }
  };
  Iterator *createIterator() { return new PlanetIterator(this);}

  bool hasLights() { return !lights.empty(); }
  const std::vector <int> & activeLights() { return lights; }

  friend class Planet::PlanetIterator;
  friend class PlanetaryOrbit;

private:
    /// copy constructor forbidden
    Planet( const Planet& );
    /// assignment operator forbidden
    Planet& operator=( const Planet& );
};

#endif


