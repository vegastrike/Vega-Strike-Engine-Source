#include "cmd/asteroid.h"
#include "cmd/script/flightgroup.h"
#include "cmd/collection.h"
#include "gfx/vec.h"
#include "gfx/quaternion.h"
#include "gfx/matrix.h"
#include "cmd/unit.h"


static void RecursiveSetSchedule(Unit *un)
{
    if (un) {
        if (un->SubUnits.empty())
            un->schedule_priority = Unit::scheduleRoid;
        else {
            un->schedule_priority = Unit::scheduleAField;
            un->do_subunit_scheduling = true;
            for (un_iter it=un->getSubUnits(); !it.isDone(); ++it)
                RecursiveSetSchedule(*it);
        }
    }
}

Asteroid::Asteroid( const char *filename, int faction, Flightgroup *fg, int fg_snumber,
                            float difficulty ) : GameUnit< AsteroidGeneric > ( filename, false, faction, string( "" ), fg, fg_snumber )
{
    asteroid_physics_offset=0;
    un_iter iter = getSubUnits();
    while(*iter) {
        float x=2*difficulty*((float)rand())/RAND_MAX-difficulty;
        float y=2*difficulty*((float)rand())/RAND_MAX-difficulty;
        float z=2*difficulty*((float)rand())/RAND_MAX-difficulty;
        (*iter)->SetAngularVelocity(Vector(x,y,z));
        ++iter;
    }
    RecursiveSetSchedule(this);
}



