#include "cmd/asteroid.h"
void Asteroid::reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
  Unit * t=this;
  switch (smaller->isUnit()) {
  case ASTEROIDPTR:
  case ENHANCEMENTPTR:
    break;
  case NEBULAPTR:
    smaller->reactToCollision(t,smalllocation,smallnormal,biglocation,bignormal,dist);
    break;
  default:
    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
    break;
  }
}
Asteroid::Asteroid(const char * filename, int faction, Flightgroup* fg, int fg_snumber, float difficulty):Unit (filename,false, faction,string(""),fg,fg_snumber) {
  UnitCollection::UnitIterator iter = getSubUnits();
  Unit *unit;
  while((unit = iter.current())!=NULL) {
    float x=2*difficulty*((float)rand())/RAND_MAX -difficulty;
    float y=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    float z=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    unit->SetAngularVelocity(Vector(x,y,z));
    iter.advance();
  }

}
void Asteroid::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  Unit::UpdatePhysics (trans,transmat, CumulativeVelocity,ResolveLast,uc);
}
