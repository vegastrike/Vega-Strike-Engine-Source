#ifndef _ASTEROID_H_
#define _ASTEROID_H_
#include "gfx/quaternion.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "cmd/script/flightgroup.h"
#include "cmd/collection.h"
#include "cmd/unit.h"
#include "unit_generic.h"




class Asteroid : public GameUnit
{
private:
    unsigned int asteroid_physics_offset;

public:
    Asteroid( const char *filename, int faction, Flightgroup *fg = NULL, int fg_snumber = 0, float difficulty = .01 );
    enum _UnitType isUnit() const
    {
        return _UnitType::asteroid;
    }

private:
/// default constructor forbidden
    Asteroid();
    Asteroid (std::vector <Mesh *> m,bool b,int i): GameUnit (m,b,i){}

/// copy constructor forbidden
    Asteroid( const Asteroid& );

/// assignment operator forbidden
    Asteroid& operator=( const Asteroid& );
};
#endif

