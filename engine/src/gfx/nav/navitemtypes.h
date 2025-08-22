/*
 * navitemtypes.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAV_ITEM_TYPES_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAV_ITEM_TYPES_H

enum NAVITEMTYPES {
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

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_NAV_ITEM_TYPES_H
