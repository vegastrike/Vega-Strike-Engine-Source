///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * OPC_SweepAndPrune.h
 *
 * Copyright (C) 2001 Pierre Terdiman
 * Copyright (C) 2021, 2022, 2025 Stephen G. Tuggy
 * Copyright (C) 2023 Benjamen R. Meyer
 *
 * This file is part of OPCODE - Optimized Collision Detection
 * (http://www.codercorner.com/Opcode.htm) and has been
 * incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an implementation of the sweep-and-prune algorithm (moved from Z-Collide)
 *	\file		OPC_SweepAndPrune.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_SWEEP_AND_PRUNE_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_SWEEP_AND_PRUNE_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	User-callback, called by OPCODE for each colliding pairs.
 *	\param		id0			[in] id of colliding object
 *	\param		id1			[in] id of colliding object
 *	\param		user_data	[in] user-defined data
 *	\return		TRUE to continue enumeration
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef BOOL    (*PairCallback)(uint32_t id0, uint32_t id1, void *user_data);

class SAP_Element;
class SAP_EndPoint;
class SAP_Box;

class OPCODE_API SAP_PairData {
public:
    SAP_PairData();
    ~SAP_PairData();

    bool Init(uint32_t nb_objects);

    void AddPair(uint32_t id1, uint32_t id2);
    void RemovePair(uint32_t id1, uint32_t id2);

    void DumpPairs(Pairs &pairs) const;
    void DumpPairs(PairCallback callback, void *user_data) const;
private:
    uint32_t mNbElements;        //!< Total number of elements in the pool
    uint32_t mNbUsedElements;    //!< Number of used elements
    SAP_Element *mElementPool;        //!< Array of mNbElements elements
    SAP_Element *mFirstFree;            //!< First free element in the pool

    uint32_t mNbObjects;            //!< Max number of objects we can handle
    SAP_Element **mArray;                //!< Pointers to pool
    // Internal methods
    SAP_Element *GetFreeElem(uint32_t id, SAP_Element *next, uint32_t *remap = nullptr);
    inline_    void FreeElem(SAP_Element *elem);
    void Release();
};

class OPCODE_API SweepAndPrune {
public:
    SweepAndPrune();
    ~SweepAndPrune();

    bool Init(uint32_t nb_objects, const AABB **boxes);
    bool UpdateObject(uint32_t i, const AABB &box);

    void GetPairs(Pairs &pairs) const;
    void GetPairs(PairCallback callback, void *user_data) const;
private:
    SAP_PairData mPairs;

    uint32_t mNbObjects;
    SAP_Box *mBoxes;
    SAP_EndPoint *mList[3];
    // Internal methods
    bool CheckListsIntegrity();
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_OPC_SWEEP_AND_PRUNE_H
