#ifndef _ASTEROID_H_
#define _ASTEROID_H_
#include "cmd/unit.h"
class Asteroid: public Unit {
  public:
	virtual enum clsptr isUnit() {return ASTEROIDPTR;}
	virtual void reactToCollision(Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist);
  
	Asteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber=0, float difficulty=.01);
	virtual void UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
};

#endif
