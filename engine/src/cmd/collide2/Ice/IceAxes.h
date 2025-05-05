///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *  Contains axes definition.
 *  \file		IceAxes.h
 *  \author		Pierre Terdiman
 *  \date		January, 29, 2000
 *
 *  Copyright (C) 1998-2025 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
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
