/*
 * OPC_SweepAndPrune.cpp
 *
 * Copyright (C) 2001-2025 Pierre Terdiman, Daniel Horn, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains an implementation of the sweep-and-prune algorithm (moved from Z-Collide)
 *	\file		OPC_SweepAndPrune.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "Stdafx.h"

using namespace Opcode;

inline_ void Sort(uint32_t &id0, uint32_t &id1) {
    if (id0 > id1) {
        Swap(id0, id1);
    }
}

class Opcode::SAP_Element {
public:
    inline_ SAP_Element() {
    }

    inline_ SAP_Element(uint32_t id, SAP_Element *next) : mID(id), mNext(next) {
    }

    inline_                    ~SAP_Element() {
    }

    uint32_t mID;
    SAP_Element *mNext;
};

class Opcode::SAP_Box {
public:
    SAP_EndPoint *Min[3];
    SAP_EndPoint *Max[3];
};

class Opcode::SAP_EndPoint {
public:
    float Value;        // Min or Max value
    SAP_EndPoint *Previous;    // Previous EndPoint whose Value is smaller than ours (or nullptr)
    SAP_EndPoint *Next;        // Next EndPoint whose Value is greater than ours (or nullptr)
    uint32_t Data;        // Parent box ID *2 | MinMax flag

    inline_    void SetData(uint32_t box_id, bool is_max) {
        Data = (box_id << 1) | (is_max ? 1 : 0);
    }

    inline_    bool IsMax() const {
        return Data & 1;
    }

    inline_    uint32_t GetBoxID() const {
        return Data >> 1;
    }

    inline_    void InsertAfter(SAP_EndPoint *element) {
        if (this != element && this != element->Next) {
            // Remove
            if (Previous) {
                Previous->Next = Next;
            }
            if (Next) {
                Next->Previous = Previous;
            }

            // Insert
            Next = element->Next;
            if (Next) {
                Next->Previous = this;
            }

            element->Next = this;
            Previous = element;
        }
    }

    inline_    void InsertBefore(SAP_EndPoint *element) {
        if (this != element && this != element->Previous) {
            // Remove
            if (Previous) {
                Previous->Next = Next;
            }
            if (Next) {
                Next->Previous = Previous;
            }

            // Insert
            Previous = element->Previous;
            element->Previous = this;

            Next = element;
            if (Previous) {
                Previous->Next = this;
            }
        }
    }
};










///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_PairData::SAP_PairData() :
        mNbElements(0),
        mNbUsedElements(0),
        mElementPool(nullptr),
        mFirstFree(nullptr),
        mNbObjects(0),
        mArray(nullptr) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_PairData::~SAP_PairData() {
    Release();
}

void SAP_PairData::Release() {
    mNbElements = 0;
    mNbUsedElements = 0;
    mNbObjects = 0;
    DELETEARRAY(mElementPool);
    DELETEARRAY(mArray);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes.
 *	\param		nb_objects	[in] 
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool SAP_PairData::Init(uint32_t nb_objects) {
    // Make sure everything has been released
    Release();
    if (!nb_objects) {
        return false;
    }

    mArray = new SAP_Element *[nb_objects];
    CHECKALLOC(mArray);
    ZeroMemory(mArray, nb_objects * sizeof(SAP_Element *));
    mNbObjects = nb_objects;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Remaps a pointer when pool gets resized.
 *	\param		element	[in/out] remapped element
 *	\param		delta	[in] offset in bytes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void Remap(SAP_Element *&element, uint32_t delta) {
    if (element) {
        element = (SAP_Element *) (uintptr_t(element) + delta);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets a free element in the pool.
 *	\param		id		[in] element id
 *	\param		next	[in] next element
 *	\param		remap	[out] possible remapping offset
 *	\return		the new element
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SAP_Element *SAP_PairData::GetFreeElem(uint32_t id, SAP_Element *next, uint32_t *remap) {
    if (remap) {
        *remap = 0;
    }

    SAP_Element *FreeElem;
    if (mFirstFree) {
        // Recycle
        FreeElem = mFirstFree;
        mFirstFree = mFirstFree->mNext;    // First free = next free (or nullptr)
    } else {
        if (mNbUsedElements == mNbElements) {
            // Resize
            mNbElements = mNbElements ? (mNbElements << 1) : 2;

            SAP_Element *NewElems = new SAP_Element[mNbElements];

            if (mNbUsedElements) {
                CopyMemory(NewElems, mElementPool, mNbUsedElements * sizeof(SAP_Element));
            }

            // Remap everything
            {
                ptrdiff_t Delta = uintptr_t(NewElems)
                        - uintptr_t(mElementPool);    // Hmm. Pointer arithmetic. That might not be such a good idea

                uint32_t i;
                for (i = 0; i < mNbUsedElements; i++) {
                    Remap(NewElems[i].mNext, Delta);
                }
                for (i = 0; i < mNbObjects; i++) {
                    Remap(mArray[i], Delta);
                }

                Remap(mFirstFree, Delta);
                Remap(next, Delta);

                if (remap) {
                    *remap = Delta;
                }
            }

            DELETEARRAY(mElementPool);
            mElementPool = NewElems;
        }

        FreeElem = &mElementPool[mNbUsedElements++];
    }

    FreeElem->mID = id;
    FreeElem->mNext = next;

    return FreeElem;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Frees an element of the pool.
 *	\param		elem	[in] element to free/recycle
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ void SAP_PairData::FreeElem(SAP_Element *elem) {
    elem->mNext = mFirstFree;    // Next free
    mFirstFree = elem;
}

// Add a pair to the set.
void SAP_PairData::AddPair(uint32_t id1, uint32_t id2) {
    // Order the ids
    Sort(id1, id2);

    OPASSERT(id1 < mNbObjects);
    if (id1 >= mNbObjects) {
        return;
    }

    // Select the right list from "mArray".
    SAP_Element *Current = mArray[id1];

    if (!Current) {
        // Empty slot => create new element
        mArray[id1] = GetFreeElem(id2, nullptr);
    } else if (Current->mID > id2) {
        // The list is not empty but all elements are greater than id2 => insert id2 in the front.
        mArray[id1] = GetFreeElem(id2, mArray[id1]);
    } else {
        // Else find the correct location in the sorted list (ascending order) and insert id2 there.
        while (Current->mNext) {
            if (Current->mNext->mID > id2) {
                break;
            }

            Current = Current->mNext;
        }

        if (Current->mID == id2) {
            return;
        }    // The pair already exists

//		Current->mNext = GetFreeElem(id2, Current->mNext);
        uint32_t Delta;
        SAP_Element *E = GetFreeElem(id2, Current->mNext, &Delta);
        if (Delta) {
            Remap(Current, Delta);
        }
        Current->mNext = E;
    }
}

// Delete a pair from the set.
void SAP_PairData::RemovePair(uint32_t id1, uint32_t id2) {
    // Order the ids.
    Sort(id1, id2);

    // Exit if the pair doesn't exist in the set
    if (id1 >= mNbObjects) {
        return;
    }

    // Otherwise, select the correct list.
    SAP_Element *Current = mArray[id1];

    // If this list is empty, the pair doesn't exist.
    if (!Current) {
        return;
    }

    // Otherwise, if id2 is the first element, delete it.
    if (Current->mID == id2) {
        mArray[id1] = Current->mNext;
        FreeElem(Current);
    } else {
        // If id2 is not the first element, start traversing the sorted list.
        while (Current->mNext) {
            // If we have moved too far away without hitting id2, then the pair doesn't exist
            if (Current->mNext->mID > id2) {
                return;
            }

            // Otherwise, delete id2.
            if (Current->mNext->mID == id2) {
                SAP_Element *Temp = Current->mNext;
                Current->mNext = Temp->mNext;
                FreeElem(Temp);
                return;
            }
            Current = Current->mNext;
        }
    }
}

void SAP_PairData::DumpPairs(Pairs &pairs) const {
    // ### Ugly and slow
    for (uint32_t i = 0; i < mNbObjects; i++) {
        SAP_Element *Current = mArray[i];
        while (Current) {
            OPASSERT(Current->mID < mNbObjects);

            pairs.AddPair(i, Current->mID);
            Current = Current->mNext;
        }
    }
}

void SAP_PairData::DumpPairs(PairCallback callback, void *user_data) const {
    if (!callback) {
        return;
    }

    // ### Ugly and slow
    for (uint32_t i = 0; i < mNbObjects; i++) {
        SAP_Element *Current = mArray[i];
        while (Current) {
            OPASSERT(Current->mID < mNbObjects);

            if (!(callback)(i, Current->mID, user_data)) {
                return;
            }
            Current = Current->mNext;
        }
    }
}




























///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SweepAndPrune::SweepAndPrune() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SweepAndPrune::~SweepAndPrune() {
}

void SweepAndPrune::GetPairs(Pairs &pairs) const {
    mPairs.DumpPairs(pairs);
}

void SweepAndPrune::GetPairs(PairCallback callback, void *user_data) const {
    mPairs.DumpPairs(callback, user_data);
}

bool SweepAndPrune::Init(uint32_t nb_objects, const AABB **boxes) {
    // 1) Create sorted lists
    mNbObjects = nb_objects;

    mBoxes = new SAP_Box[nb_objects];
//	for(uint32_t i=0;i<nb_objects;i++)	mBoxes[i].Box = *boxes[i];

    float *Data = new float[nb_objects * 2];

    for (uint32_t Axis = 0; Axis < 3; Axis++) {
        mList[Axis] = new SAP_EndPoint[nb_objects * 2];

        uint32_t i;
        for (i = 0; i < nb_objects; i++) {
            Data[i * 2 + 0] = boxes[i]->GetMin(Axis);
            Data[i * 2 + 1] = boxes[i]->GetMax(Axis);
        }
        RadixSort RS;
        const uint32_t *Sorted = RS.Sort(Data, nb_objects * 2).GetRanks();

        SAP_EndPoint *PreviousEndPoint = nullptr;

        for (i = 0; i < nb_objects * 2; i++) {
            uint32_t SortedIndex = *Sorted++;
            float SortedCoord = Data[SortedIndex];
            uint32_t BoxIndex = SortedIndex >> 1;

            OPASSERT(BoxIndex < nb_objects);

            SAP_EndPoint *CurrentEndPoint = &mList[Axis][SortedIndex];
            CurrentEndPoint->Value = SortedCoord;
//			CurrentEndPoint->IsMax		= SortedIndex&1;		// ### could be implicit ?
//			CurrentEndPoint->ID			= BoxIndex;				// ### could be implicit ?
            CurrentEndPoint->SetData(BoxIndex, SortedIndex & 1);    // ### could be implicit ?
            CurrentEndPoint->Previous = PreviousEndPoint;
            CurrentEndPoint->Next = nullptr;
            if (PreviousEndPoint) {
                PreviousEndPoint->Next = CurrentEndPoint;
            }

            if (CurrentEndPoint->IsMax()) {
                mBoxes[BoxIndex].Max[Axis] = CurrentEndPoint;
            } else {
                mBoxes[BoxIndex].Min[Axis] = CurrentEndPoint;
            }

            PreviousEndPoint = CurrentEndPoint;
        }
    }

    DELETEARRAY(Data);

    CheckListsIntegrity();

    // 2) Quickly find starting pairs

    mPairs.Init(nb_objects);

    {
        Pairs P;
        CompleteBoxPruning(nb_objects, boxes, P, Axes(AXES_XZY));
        for (uint32_t i = 0; i < P.GetNbPairs(); i++) {
            const Pair *PP = P.GetPair(i);

            uint32_t id0 = PP->id0;
            uint32_t id1 = PP->id1;

            if (id0 != id1 && boxes[id0]->Intersect(*boxes[id1])) {
                mPairs.AddPair(id0, id1);
            } else OPASSERT(0);
        }
    }

    return true;
}

bool SweepAndPrune::CheckListsIntegrity() {
    for (uint32_t Axis = 0; Axis < 3; Axis++) {
        // Find list head
        SAP_EndPoint *Current = mList[Axis];
        while (Current->Previous) {
            Current = Current->Previous;
        }

        uint32_t Nb = 0;

        SAP_EndPoint *Previous = nullptr;
        while (Current) {
            Nb++;

            if (Previous) {
                OPASSERT(Previous->Value <= Current->Value);
                if (Previous->Value > Current->Value) {
                    return false;
                }
            }

            OPASSERT(Current->Previous == Previous);
            if (Current->Previous != Previous) {
                return false;
            }

            Previous = Current;
            Current = Current->Next;
        }

        OPASSERT(Nb == mNbObjects * 2);
    }
    return true;
}

inline_ bool Intersect(const AABB &a, const SAP_Box &b) {
    if (b.Max[0]->Value < a.GetMin(0) || a.GetMax(0) < b.Min[0]->Value
            || b.Max[1]->Value < a.GetMin(1) || a.GetMax(1) < b.Min[1]->Value
            || b.Max[2]->Value < a.GetMin(2) || a.GetMax(2) < b.Min[2]->Value) {
        return FALSE;
    }

    return TRUE;
}

bool SweepAndPrune::UpdateObject(uint32_t i, const AABB &box) {
    for (uint32_t Axis = 0; Axis < 3; Axis++) {
//		uint32_t Base = (uint32_t)&mList[Axis][0];

        // Update min
        {
            SAP_EndPoint *const CurrentMin = mBoxes[i].Min[Axis];
            OPASSERT(!CurrentMin->IsMax());

            const float Limit = box.GetMin(Axis);
            if (Limit == CurrentMin->Value) {
            } else if (Limit < CurrentMin->Value) {
                CurrentMin->Value = Limit;

                // Min is moving left:
                SAP_EndPoint *NewPos = CurrentMin;
                OPASSERT(NewPos);

                SAP_EndPoint *tmp;
                while ((tmp = NewPos->Previous) && tmp->Value > Limit) {
                    NewPos = tmp;

                    if (NewPos->IsMax()) {
                        // Our min passed a max => start overlap
                        //uint32_t SortedIndex = (uint32_t(CurrentMin) - Base)/sizeof(NS_EndPoint);
                        const uint32_t id0 = CurrentMin->GetBoxID();
                        const uint32_t id1 = NewPos->GetBoxID();

                        if (id0 != id1 && Intersect(box, mBoxes[id1])) {
                            mPairs.AddPair(id0, id1);
                        }
                    }
                }

                CurrentMin->InsertBefore(NewPos);
            } else// if(Limit > CurrentMin->Value)
            {
                CurrentMin->Value = Limit;

                // Min is moving right:
                SAP_EndPoint *NewPos = CurrentMin;
                OPASSERT(NewPos);

                SAP_EndPoint *tmp;
                while ((tmp = NewPos->Next) && tmp->Value < Limit) {
                    NewPos = tmp;

                    if (NewPos->IsMax()) {
                        // Our min passed a max => stop overlap
                        const uint32_t id0 = CurrentMin->GetBoxID();
                        const uint32_t id1 = NewPos->GetBoxID();

                        if (id0 != id1) {
                            mPairs.RemovePair(id0, id1);
                        }
                    }
                }

                CurrentMin->InsertAfter(NewPos);
            }
        }

        // Update max
        {
            SAP_EndPoint *const CurrentMax = mBoxes[i].Max[Axis];
            OPASSERT(CurrentMax->IsMax());

            const float Limit = box.GetMax(Axis);
            if (Limit == CurrentMax->Value) {
            } else if (Limit > CurrentMax->Value) {
                CurrentMax->Value = Limit;

                // Max is moving right:
                SAP_EndPoint *NewPos = CurrentMax;
                OPASSERT(NewPos);

                SAP_EndPoint *tmp;
                while ((tmp = NewPos->Next) && tmp->Value < Limit) {
                    NewPos = tmp;

                    if (!NewPos->IsMax()) {
                        // Our max passed a min => start overlap
                        const uint32_t id0 = CurrentMax->GetBoxID();
                        const uint32_t id1 = NewPos->GetBoxID();

                        if (id0 != id1 && Intersect(box, mBoxes[id1])) {
                            mPairs.AddPair(id0, id1);
                        }
                    }
                }

                CurrentMax->InsertAfter(NewPos);
            } else// if(Limit < CurrentMax->Value)
            {
                CurrentMax->Value = Limit;

                // Max is moving left:
                SAP_EndPoint *NewPos = CurrentMax;
                OPASSERT(NewPos);

                SAP_EndPoint *tmp;
                while ((tmp = NewPos->Previous) && tmp->Value > Limit) {
                    NewPos = tmp;

                    if (!NewPos->IsMax()) {
                        // Our max passed a min => stop overlap
                        const uint32_t id0 = CurrentMax->GetBoxID();
                        const uint32_t id1 = NewPos->GetBoxID();

                        if (id0 != id1) {
                            mPairs.RemovePair(id0, id1);
                        }
                    }
                }

                CurrentMax->InsertBefore(NewPos);
            }
        }
    }

    return true;
}
