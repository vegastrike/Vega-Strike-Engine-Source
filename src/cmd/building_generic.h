#ifndef __BUILDING_GENERIC_H
#define __BUILDING_GENERIC_H

#include "unit_generic.h"
class Terrain;
class ContinuousTerrain;
class Flightgroup;
class Building : public Unit {
protected:
  union Buildingparent{
    Terrain * terrain;
    ContinuousTerrain * plane;
  } parent;
  bool continuous;
  bool vehicle;

protected:
  Building (ContinuousTerrain * parent, bool vehicle, const char * filename, bool SubUnit, int faction, const std::string &unitModifications=std::string(""),Flightgroup * fg=NULL);
  Building (Terrain * parent, bool vehicle, const char *filename, bool SubUnit, int faction, const std::string &unitModifications=std::string(""),Flightgroup * fg=NULL);

  friend class UnitFactory;

 public:
	
  virtual enum clsptr isUnit () const{return BUILDINGPTR;}
  virtual void UpdatePhysics2 (const Transformation &trans, const Transformation &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL) {GameUnit<Building>::UpdatePhysics2(trans,transmat,CumulativeVelocity,ResolveLast,uc);}
  bool ownz (void * parent) {return (this->parent.terrain == (Terrain *)parent);}

protected:
	Building (std::vector <Mesh *> m,bool b,int i):Unit (m,b,i){}
    /// default constructor forbidden
    Building( ) {}
    /// copy constructor forbidden
    //Building( const Building& );
    /// assignment operator forbidden
    //Building& operator=( const Building& );
};

#endif
