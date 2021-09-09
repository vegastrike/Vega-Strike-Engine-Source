/**
* car_assist.h
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

#include "xml_support.h"
using XMLSupport::EnumMap;
namespace CAR
{
const int FORWARD_BLINKEN = 1;
const int LEFT_BLINKEN    = 2;
const int RIGHT_BLINKEN   = 4;
const int SIREN_BLINKEN   = 8;
const int ON_NO_BLINKEN   = 16;

enum ALL_TYPES
{
    RUNNINGLIGHTS, HEADLIGHTS, LEFTBLINK, RIGHTBLINK, BRAKE, REVERSE, SIREN
};
const EnumMap::Pair type_names[8] = {
    EnumMap::Pair( "UNKNOWN",       RUNNINGLIGHTS ),
    EnumMap::Pair( "RUNNINGLIGHTS", RUNNINGLIGHTS ),
    EnumMap::Pair( "HEADLIGHTS",    HEADLIGHTS ),
    EnumMap::Pair( "LEFTBLINK",     LEFTBLINK ),
    EnumMap::Pair( "RIGHTBLINK",    RIGHTBLINK ),
    EnumMap::Pair( "BRAKE",         BRAKE ),
    EnumMap::Pair( "REVERSE",       REVERSE ),
    EnumMap::Pair( "SIREN",         SIREN )
};
const EnumMap type_map( type_names, 8 );
}

