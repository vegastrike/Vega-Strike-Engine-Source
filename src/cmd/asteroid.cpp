#include "cmd/asteroid.h"
void Asteroid::reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
  Unit * t=this;
  switch (smaller->isUnit()) {
  case ASTEROIDPTR:
    break;
  case NEBULAPTR:
    smaller->reactToCollision(t,smalllocation,smallnormal,biglocation,bignormal,dist);
    break;
  default:
    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
    break;
  }
}
Asteroid::Asteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber, float difficulty):Unit (filename,true, false, faction,fg,fg_snumber) {
  for (int i=0;i<numsubunit;i++) {
    float x=2*difficulty*((float)rand())/RAND_MAX -difficulty;
    float y=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    float z=2*difficulty*((float)rand())/RAND_MAX-difficulty;
    subunits[i]->SetAngularVelocity(Vector(x,y,z));
  }
}
void Asteroid::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  Unit::UpdatePhysics (trans,transmat, CumulativeVelocity,ResolveLast,uc);
}
