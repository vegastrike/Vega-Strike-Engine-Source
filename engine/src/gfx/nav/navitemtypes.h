/**
* navitemtypes.h
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

#ifndef _NAVITEMTYPES_H_
#define _NAVITEMTYPES_H_

enum NAVITEMTYPES
{
    navsun,
    navplanet,
    navplayer,
    navcurrentplayer,
    navstation,
    navfighter,
    navcapship,
    navmissile,
    navasteroid,
    navnebula,
    navjump,
    navambiguous
};

#define navsunsize 0.03
#define navplanetsize 0.02
#define navplayersize 0.01
#define navcurrentplayersize 0.01
#define navstationsize 0.015
#define navfightersize 0.01
#define navcapshipsize 0.0125
#define navmissilesize 0.005
#define navasteroidsize 0.015
#define navnebulasize 0.03
#define navjumpsize 0.0125
#define navambiguoussize 0.01

#endif

