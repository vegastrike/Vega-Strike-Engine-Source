/*
 * IceAxes.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains axes definition.
 *	\file		IceAxes.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_AXES_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_AXES_H

enum PointComponent {
    _X = 0,
    _Y = 1,
    _Z = 2,
    _W = 3,

    _FORCE_DWORD = 0x7fffffff
};

enum AxisOrder {
    AXES_XYZ = (_X) | (_Y << 2) | (_Z << 4),
    AXES_XZY = (_X) | (_Z << 2) | (_Y << 4),
    AXES_YXZ = (_Y) | (_X << 2) | (_Z << 4),
    AXES_YZX = (_Y) | (_Z << 2) | (_X << 4),
    AXES_ZXY = (_Z) | (_X << 2) | (_Y << 4),
    AXES_ZYX = (_Z) | (_Y << 2) | (_X << 4),

    AXES_FORCE_DWORD = 0x7fffffff
};

class ICEMATHS_API Axes {
public:

    inline_ Axes(AxisOrder order) {
        mAxis0 = (order) & 3;
        mAxis1 = (order >> 2) & 3;
        mAxis2 = (order >> 4) & 3;
    }

    inline_            ~Axes() {
    }

    uint32_t mAxis0;
    uint32_t mAxis1;
    uint32_t mAxis2;
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_AXES_H
