#include "unit.h"
class Terrain;
class ContinuousTerrain;
class Flightgroup;
class Building : public GameUnit {
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
  virtual enum clsptr isUnit () {return BUILDINGPTR;}
  virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
  bool ownz (void * parent) {return (this->parent.terrain == (Terrain *)parent);}

private:
    /// default constructor forbidden
    Building( );
    /// copy constructor forbidden
    Building( const Building& );
    /// assignment operator forbidden
    Building& operator=( const Building& );
};
