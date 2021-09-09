/**
* asteroid.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

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
                            float difficulty ) : GameUnit ( filename, false, faction, string( "" ), fg, fg_snumber )
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



