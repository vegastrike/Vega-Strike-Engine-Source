#ifndef _NEBULA_H_
#define _NEBULA_H_
#include "nebula_generic.h"
#include "unit.h"

class GameNebula: public GameUnit<Nebula> {
protected:
    /// constructor only to be called by UnitFactory
    GameNebula( const char * unitfile,
            bool SubU,
	    int faction,
	    Flightgroup* fg=NULL,
	    int fg_snumber=0 );

    friend class UnitFactory;

public:
	virtual void UpdatePhysics2 (const Transformation &trans,const  Transformation& transmat, const Matrix & m, float diff, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL);
	void SetFogState();
	void PutInsideCam(int);

private:
    /// default constructor forbidden
    GameNebula( );
    /// copy constructor forbidden
    GameNebula( const Nebula& );
    /// assignment operator forbidden
    GameNebula& operator=( const Nebula& );
};

#endif
