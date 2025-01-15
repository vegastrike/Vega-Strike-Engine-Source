///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains a simple pair class.
 *	\file		IcePairs.h
 *	\author		Pierre Terdiman
 *	\date		January, 13, 2003
 *
 *  Copyright (C) 2021, 2022, 2025 Stephen G. Tuggy
 *  Copyright (C) 2023 Benjamen R. Meyer
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
