/**
* vs_limits.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#ifndef LIMITS_H
#define LIMITS_H

#include "gfx/vec.h"

class Limits
{
public:
//max ypr--both pos/neg are symmetrical
    float  yaw;
    float  pitch;
    float  roll;
//side-side engine thrust max
    float  lateral;
//vertical engine thrust max
    float  vertical;
//forward engine thrust max
    float  forward;
//reverse engine thrust max
    float  retro;
//after burner acceleration max
    float  afterburn;
//the vector denoting the "front" of the turret cone!
    Vector structurelimits;
//the minimum dot that the current heading can have with the structurelimit
    float  limitmin;

    Limits() : yaw( 0 )
        , pitch( 0 )
        , roll( 0 )
        , lateral( 0 )
        , vertical( 0 )
        , forward( 0 )
        , retro( 0 )
        , afterburn( 0 )
        , structurelimits( 0, 0, 0 )
        , limitmin( 0 ) {}
};

#endif // LIMITS_H
