/*
 * IceRandom.h
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
 *	Contains code for random generators.
 *	\file		IceRandom.h
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RANDOM_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RANDOM_H

ICECORE_API void SRand(uint32_t seed);
uint32_t Rand();

//! Returns a unit random floating-point value
inline_ float UnitRandomFloat() {
    return float(Rand()) * ONE_OVER_RAND_MAX;
}

//! Returns a random index so that 0<= index < max_index
ICECORE_API uint32_t GetRandomIndex(uint32_t max_index);

class ICECORE_API BasicRandom {
public:

    //! Constructor
    inline_ BasicRandom(uint32_t seed = 0) : mRnd(seed) {
    }
    //! Destructor
    inline_                ~BasicRandom() {
    }

    inline_    void SetSeed(uint32_t seed) {
        mRnd = seed;
    }

    inline_    uint32_t GetCurrentValue() const {
        return mRnd;
    }

    inline_    uint32_t Randomize() {
        mRnd = mRnd * 2147001325 + 715136305;
        return mRnd;
    }

private:
    uint32_t mRnd;
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RANDOM_H
