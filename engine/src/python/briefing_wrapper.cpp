/*
 * briefing_wrapper.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef PYTHON_STUB
#define PY_SSIZE_T_CLEAN
#include <math.h>
#include "python_class.h"
#include "cmd/briefing.h"
#include "cmd/script/mission.h"
#include "faction_generic.h"

namespace BriefingUtil {
int addShip(string name, string faction, Vector vec) {
    return mission->briefing->AddStarship(name.c_str(),
            FactionUtil::GetFactionIndex(faction),
            vec);
}

void removeShip(int whichship) {
    mission->briefing->RemoveStarship(whichship);
}

void enqueueOrder(int whichship, Vector vec, float time) {
    mission->briefing->EnqueueOrder(whichship, vec, time);
}

void replaceOrder(int whichship, Vector vec, float time) {
    mission->briefing->OverrideOrder(whichship, vec, time);
}

Vector getShipPosition(int whichship) {
    return mission->briefing->GetPosition(whichship);
}

void setShipPosition(int whichship, Vector vec) {
    mission->briefing->SetPosition(whichship, vec);
}

void terminate() {
    mission->BriefingEnd();
}

void setCamPosition(QVector p) {
    mission->briefing->cam.SetPosition(p, Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0));
}

void setCamOrientation(Vector p, Vector q, Vector r) {
    mission->briefing->cam.SetOrientation(p, q, r);
}

void setCloak(int whichship, float cloak) {
    mission->briefing->SetCloak(whichship, cloak);
}
}
PYTHON_BEGIN_MODULE(Briefing)
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::addShip, "addShip");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::removeShip, "removeShip");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::enqueueOrder, "enqueueOrder");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::replaceOrder, "replaceOrder");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::getShipPosition, "getShipPosition");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::setShipPosition, "setShipPosition");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::terminate, "terminate");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::setCamPosition, "setCamPosition");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::setCamOrientation, "setCamOrientation");
    PYTHON_DEFINE_GLOBAL(Briefing, &BriefingUtil::setCloak, "setCloak");
PYTHON_END_MODULE(Briefing)

void InitBriefing() {
    PyImport_AppendInittab("Briefing", PYTHON_MODULE_INIT_FUNCTION(Briefing));
}

void InitBriefing2() {
    Python::reseterrors();
    PYTHON_INIT_MODULE(Briefing);
    Python::reseterrors();
}

///////////////////////////////////////////////
//END C++
///////////////////////////////////////////////
//below replace ~ with enter
#else
#define MYPRINT( nam ) print #nam
#define WRAPPED0( type, name, aff ) def name( self ) : ~MYPRINT( name ) ~return aff
#define WRAPPED1( type, name, atype, a, aff ) def name( self, a ) : ~MYPRINT( name )  ~return aff
#define WRAPPED2( type, name, atype, a, btype, b, aff ) def name( self, a, b ) : ~MYPRINT( name ) ~return aff
#define WRAPPED3( type, name, atype, a, btype, b, ctype, c, aff ) def name( self, a, b, c ) : ~MYPRINT( name ) ~return aff
#define voidWRAPPED0( name ) def name( self ) : ~MYPRINT( name )
#define voidWRAPPED1( name, atype, a ) def name( self, a ) : ~MYPRINT( name )
#define voidWRAPPED2( name, atype, a, btype, b ) def name( self, a, b ) : ~MYPRINT( name )
#define voidWRAPPED3( name, atype, a, btype, b, ctype, c ) def name( self, a, b, c ) : ~MYPRINT( name )

WRAPPED3( int
          , addShip
          , string
          , name
          , string
          , faction
          , Vector
          , vec
          , 0 )
voidWRAPPED1( removeShip
              , int
              , ship )
voidWRAPPED3( enqueueOrder
              , int
              , ship
              , Vector
              , dest
              , float
              , time )
voidWRAPPED3( replaceOrder
              , int
              , ship
              , Vector
              , dest
              , float
              , time )
WRAPPED1( Vector
          , getShipPosition
          , int
          , ship
           , (50, 75, 100) )
voidWRAPPED2( setShipPosition
              , int
              , ship
              , Vector
              , vec )
voidWRAPPED0( terminate )
voidWRAPPED1( setCamPosition
              , Vector
              , pos )
voidWRAPPED3( setCamOrientation
              , Vector
              , p
              , Vector
              , q
              , Vector
              , r )
voidWRAPPED2( setCloak
              , int
              , ship
              , float
              , amount )

#endif

