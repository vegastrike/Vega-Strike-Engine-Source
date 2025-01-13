/*
 * IcePairs.h
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
 *	Contains a simple pair class.
 *	\file		IcePairs.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2003
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
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PAIRS_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PAIRS_H

//! A generic couple structure
struct ICECORE_API Pair {
    inline_ Pair() {
    }

    inline_ Pair(uint32_t i0, uint32_t i1) : id0(i0), id1(i1) {
    }

    uint32_t id0;    //!< First index of the pair
    uint32_t id1;    //!< Second index of the pair
};

class ICECORE_API Pairs : private Container {
public:
    // Constructor / Destructor
    Pairs() {
    }

    ~Pairs() {
    }

    inline_    uint32_t GetNbPairs() const {
        return GetNbEntries() >> 1;
    }

    inline_    const Pair *GetPairs() const {
        return (const Pair *) GetEntries();
    }

    inline_    const Pair *GetPair(uint32_t i) const {
        return (const Pair *) &GetEntries()[i + i];
    }

    inline_    BOOL HasPairs() const {
        return IsNotEmpty();
    }

    inline_    void ResetPairs() {
        Reset();
    }

    inline_    void DeleteLastPair() {
        DeleteLastEntry();
        DeleteLastEntry();
    }

    inline_    void AddPair(const Pair &p) {
        Add(p.id0).Add(p.id1);
    }

    inline_    void AddPair(uint32_t id0, uint32_t id1) {
        Add(id0).Add(id1);
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_PAIRS_H
