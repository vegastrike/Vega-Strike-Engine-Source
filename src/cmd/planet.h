/* planet.h
ok * 
 * Unit type that is a gravity source, and follows a set orbit pattern
 */
#ifndef _PLANET_H_
#define _PLANET_H_

#include <stdio.h>

#include "unit.h"
#include "planet_generic.h"
#include "ai/order.h"
//#include "physics.h"
#include "collection.h"
#include "iterator.h"
#include <vector>


struct GFXMaterial;
/* Orbits in the xy plane with the given radius. Depends on a reorientation of coordinate bases */

struct Texture;
class Atmosphere;

struct GFXLightLocal {
  GFXLight ligh;
  bool islocal;
};
class ContinuousTerrain;
class PlanetaryTransform;
class GamePlanet : public GameUnit<class Planet> {
 private:
  Animation *shine;
protected:
    /// default constructor - only to be called by UnitFactory
  GamePlanet();

    /// constructor - only to be called by UnitFactory
    GamePlanet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename, BLENDFUNC blendsrc, BLENDFUNC blenddst, vector<char *> dest, const QVector &orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &, int faction,string fullname, bool inside_out=false);

    friend class UnitFactory;

public:
  UnitCollection satellites;
  virtual ~GamePlanet();
  void AddAtmosphere (const std::string &texture, float radius, BLENDFUNC blendSrc, BLENDFUNC blendDst);
  void AddRing (const std::string &texture,float iradius,float oradius, const QVector &r,const QVector &s, int slices, int numwrapx, int numwrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst);
  void AddCity (const std::string &texture,float radius,int numwrapx, int numwrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst, bool inside_out=false);
  void DisableLights ();
  void EnableLights();
  //void AddSatellite (Unit * orbiter);
  //void endElement();
  //string getCargoUnitName () const {return getFullname();}
  //string getHumanReadablePlanetType () const ;
  //Unit * beginElement(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename,BLENDFUNC blendsrc, BLENDFUNC blenddst, vector<char *>dest,int level, const GFXMaterial &ourmat, const std::vector <GFXLightLocal> &ligh, bool isunit, int faction,string fullname, bool inside_out);
  //Planet * GetTopPlanet (int level);
  //virtual enum clsptr isUnit() {return PLANETPTR;}
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

  friend class Planet::PlanetIterator;
  friend class PlanetaryOrbit;

private:
    /// copy constructor forbidden
    GamePlanet( const Planet& );
    /// assignment operator forbidden
    GamePlanet& operator=( const Planet& );
};

#endif


