#ifndef __PLANETGENERIC_H
#define __PLANETGENERIC_H

#include "unit_generic.h"
//#include "unit_factory.h"
#include "ai/order.h"
#include "configxml.h"
//#include "gfx/planetary_transform.h"
#include "images.h"
class Atmosphere;
class PlanetaryTransform;
class ContinuousTerrain;
struct GFXLightLocal;

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

class Planet : public Unit {
 protected:
  PlanetaryTransform * terraintrans;
  Atmosphere * atmosphere;
  ContinuousTerrain *terrain;
  Vector TerrainUp;
  Vector TerrainH;
  bool inside;
  bool atmospheric;//then users can go inside!
  float radius;
  float gravity;
  UnitCollection insiders;
  std::vector <int> lights;
  //Animation *shine;
protected:
    /// default constructor - only to be called by UnitFactory
  Planet();
    /// constructor - only to be called by UnitFactory
    Planet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename, vector<char *> dest, const QVector &orbitcent, Unit * parent, int faction,string fullname, bool inside_out=false, unsigned int lights_num=0);
	void InitPlanet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename, vector<char *> dest, const QVector &orbitcent, Unit * parent, int faction,string fullname, bool inside_out, unsigned int lights_num);

    friend class UnitFactory;
	Planet (std::vector <Mesh *> m,bool b,int i):Unit (m,b,i){}
public:
  UnitCollection satellites;
  virtual ~Planet();
  virtual Vector AddSpaceElevator (const std::string &name, const std::string & faction, char direction);
  virtual void AddFog (const vector <AtmosphericFogMesh> & meshes){}
  virtual void AddAtmosphere (const std::string &texture, float radius, BLENDFUNC blendSrc, BLENDFUNC blendDst) {}
  virtual void AddRing (const std::string &texture,float iradius,float oradius, const QVector &r,const QVector &s, int slices, int numwrapx, int numwrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst) {}
  virtual void AddCity (const std::string &texture,float radius,int numwrapx, int numwrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst, bool inside_out=false,bool reverse_normals=true) {}
  virtual void DisableLights () {}
  virtual void EnableLights() {}
  void AddSatellite (Unit * orbiter);
  void endElement();
  string getCargoUnitName () const {return getFullname();}
  string getHumanReadablePlanetType () const ;
  Unit * beginElement(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename,BLENDFUNC blendsrc, BLENDFUNC blenddst, vector<char *>dest,int level, const GFXMaterial &ourmat, const std::vector <GFXLightLocal> &ligh, bool isunit, int faction,string fullname, bool inside_out);
  Planet * GetTopPlanet (int level);
  virtual enum clsptr isUnit()const {return PLANETPTR;}
  virtual void Draw(const Transformation & quat = identity_transformation, const Matrix &m = identity_matrix) {}
  virtual void DrawTerrain() {}
  static void ProcessTerrains() {}
  virtual void Kill(bool erasefromsave=false);

  virtual PlanetaryTransform*  setTerrain (ContinuousTerrain *, float ratiox, int numwraps, float scaleatmos) { return NULL;}
  virtual ContinuousTerrain * getTerrain(PlanetaryTransform *&t) {return NULL;}
  virtual void setAtmosphere (Atmosphere *) {}
  virtual Atmosphere * getAtmosphere () {return NULL;}
  virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal,  float dist) {}
  virtual void gravitate(UnitCollection *units);

  class PlanetIterator : public UnIterator {
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
  UnIterator *createIterator() { return new PlanetIterator(this);}
  bool isAtmospheric  () {
    return hasLights()||atmospheric;
  }

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

