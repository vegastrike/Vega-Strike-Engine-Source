#ifndef _ASTEROID_H_
#define _ASTEROID_H_
#include "cmd/unit.h"
class Asteroid: public GameUnit {
  public:
	virtual enum clsptr isUnit() {return ASTEROIDPTR;}
	virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);
  
	virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
protected:
    /** Constructor that can only be called by the UnitFactory.
     */
    Asteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber=0, float difficulty=.01);

    friend class GameUnitFactory;

private:
    /// default constructor forbidden
    Asteroid( );

    /// copy constructor forbidden
    Asteroid( const Asteroid& );

    /// assignment operator forbidden
    Asteroid& operator=( const Asteroid& );
};

#endif
