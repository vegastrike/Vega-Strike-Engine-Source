#include "unit.h"
class Terrain;
class ContinuousTerrain;
class Flightgroup;
class Building : public Unit {
  union Buildingparent{
    Terrain * terrain;
    ContinuousTerrain * plane;
  } parent;
  bool continuous;
  bool vehicle;
 public:
  Building (ContinuousTerrain * parent, bool vehicle, const char * filename, bool SubUnit, int faction, const std::string &unitModifications=std::string(""),Flightgroup * fg=NULL);
  Building (Terrain * parent, bool vehicle, const char *filename, bool SubUnit, int faction, const std::string &unitModifications=std::string(""),Flightgroup * fg=NULL);
  virtual enum clsptr isUnit () {return BUILDINGPTR;}
  virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  bool ownz (void * parent) {return (this->parent.terrain == (Terrain *)parent);}
};
