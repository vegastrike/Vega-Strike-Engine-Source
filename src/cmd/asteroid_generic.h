#ifndef _ASTEROIDGENERIC_H_
#define _ASTEROIDGENERIC_H_
#include "unit_generic.h"
class Asteroid: public Unit {
  public:
	void Init( float difficulty);
	virtual enum clsptr isUnit()const {return ASTEROIDPTR;}
	virtual void reactToCollision(Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist);

	//virtual void UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
protected:
    /** Constructor that can only be called by the UnitFactory.
     */
    Asteroid(const char * filename, int faction, Flightgroup* fg=NULL, int fg_snumber=0, float difficulty=.01);

    friend class UnitFactory;

protected:
    /// default constructor forbidden
    Asteroid( ) {}
	Asteroid (std::vector <Mesh *> m,bool b,int i):Unit (m,b,i){}
    /// copy constructor forbidden
    //Asteroid( const Asteroid& );

    /// assignment operator forbidden
    //Asteroid& operator=( const Asteroid& );

private:
    unsigned int asteroid_physics_offset;
};

#endif
