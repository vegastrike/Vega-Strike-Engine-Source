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
 public:
  Building (ContinuousTerrain * parent, const char * filename, bool xml, bool SubUnit, int faction, Flightgroup * fg=NULL);
  Building (Terrain * parent, const char *filename, bool xml, bool SubUnit, int faction, Flightgroup * fg=NULL);
  virtual enum clsptr isUnit () {return BUILDINGPTR;}
  virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  bool ownz (void * parent) {return (this->parent.terrain == (Terrain *)parent);}
};
