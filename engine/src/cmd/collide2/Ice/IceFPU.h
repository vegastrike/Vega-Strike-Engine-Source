/*
 * IceFPU.h
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
 *	Contains FPU related code.
 *	\file		IceFPU.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2021-07-07
 *  Updated by Stephen G. Tuggy 2022-01-06
 *  Updated by Benjamen R. Meyer 2023-05-27
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_FPU_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_FPU_H

#define    SIGN_BITMASK            0x80000000

union ice_union {
    int32_t sint32_member;
    uint32_t uint32_member;
    float float_member;
};

// 2021-07-07 SGT: It seems that the below macros cannot be converted to inline functions,
//                 because they need to be assignable in some contexts.
//! Integer representation of a floating-point value.
#define IR(x)                   (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->uint32_member)

//! Signed integer representation of a floating-point value.
#define SIR(x)                  (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->sint32_member)

//! Absolute integer representation of a floating-point value
#define AIR(x)                    (IR(x)&0x7fffffff)

//! Floating-point representation of an integer value.
#define FR(x)                   (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->float_member)

//! Is the float valid ?
inline_ bool IsNAN(float value) {
    return (IR(value) & 0x7f800000) == 0x7f800000;
}

inline_ bool IsIndeterminate(float value) {
    return IR(value) == 0xffc00000;
}

inline_ bool IsPlusInf(float value) {
    return IR(value) == 0x7f800000;
}

inline_ bool IsMinusInf(float value) {
    return IR(value) == 0xff800000;
}

inline_    bool IsValidFloat(float value) {
    if (IsNAN(value)) {
        return false;
    }
    if (IsIndeterminate(value)) {
        return false;
    }
    if (IsPlusInf(value)) {
        return false;
    }
    if (IsMinusInf(value)) {
        return false;
    }
    return true;
}

#define CHECK_VALID_FLOAT(x)    OPASSERT(IsValidFloat(x));

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_FPU_H
