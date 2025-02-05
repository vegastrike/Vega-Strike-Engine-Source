///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for box pruning.
 *	\file		IceBoxPruning.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	You could use a complex sweep-and-prune as implemented in I-Collide.
	You could use a complex hashing scheme as implemented in V-Clip or recently in ODE it seems.
	You could use a "Recursive Dimensional Clustering" algorithm as implemented in GPG2.

	Or you could use this.
	Faster ? I don't know. Probably not. It would be a shame. But who knows ?
	Easier ? Definitely. Enjoy the sheer simplicity.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include "cmd/collide2/Opcode.h"

using namespace Opcode;

inline_ void FindRunningIndex(uint32_t &index, float *array, uint32_t *sorted, int last, float max) {
    int First = index;
    while (First <= last) {
        index = (First + last) >> 1;

        if (max > array[sorted[index]]) {
            First = index + 1;
        } else {
            last = index - 1;
        }
    }
}
// ### could be log(n) !
// and maybe use cmp integers

// InsertionSort has better coherence, RadixSort is better for one-shot queries.
#define PRUNING_SORTER    RadixSort
//#define PRUNING_SORTER	InsertionSort

// Static for coherence
static PRUNING_SORTER *gCompletePruningSorter = nullptr;
static PRUNING_SORTER *gBipartitePruningSorter0 = nullptr;
static PRUNING_SORTER *gBipartitePruningSorter1 = nullptr;

inline_ PRUNING_SORTER *GetCompletePruningSorter() {
    if (!gCompletePruningSorter) {
        gCompletePruningSorter = new PRUNING_SORTER;
    }
    return gCompletePruningSorter;
}

inline_ PRUNING_SORTER *GetBipartitePruningSorter0() {
    if (!gBipartitePruningSorter0) {
        gBipartitePruningSorter0 = new PRUNING_SORTER;
    }
    return gBipartitePruningSorter0;
}

inline_ PRUNING_SORTER *GetBipartitePruningSorter1() {
    if (!gBipartitePruningSorter1) {
        gBipartitePruningSorter1 = new PRUNING_SORTER;
    }
    return gBipartitePruningSorter1;
}

void ReleasePruningSorters() {
    DELETESINGLE(gBipartitePruningSorter1);
    DELETESINGLE(gBipartitePruningSorter0);
    DELETESINGLE(gCompletePruningSorter);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Bipartite box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to a different set.
 *	\param		nb0		[in] number of boxes in the first set
 *	\param		array0	[in] array of boxes for the first set
 *	\param		nb1		[in] number of boxes in the second set
 *	\param		array1	[in] array of boxes for the second set
 *	\param		pairs	[out] array of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Opcode::BipartiteBoxPruning(uint32_t nb0,
        const AABB **array0,
        uint32_t nb1,
        const AABB **array1,
        Pairs &pairs,
        const Axes &axes) {
    // Checkings
    if (!nb0 || !array0 || !nb1 || !array1) {
        return false;
    }

    // Catch axes
    uint32_t Axis0 = axes.mAxis0;
    uint32_t Axis1 = axes.mAxis1;
    uint32_t Axis2 = axes.mAxis2;

    // Allocate some temporary data
    float *MinPosList0 = new float[nb0];
    float *MinPosList1 = new float[nb1];

    uint32_t i;
    // 1) Build main lists using the primary axis
    for (i = 0; i < nb0; i++) {
        MinPosList0[i] = array0[i]->GetMin(Axis0);
    }
    for (i = 0; i < nb1; i++) {
        MinPosList1[i] = array1[i]->GetMin(Axis0);
    }

    // 2) Sort the lists
    PRUNING_SORTER *RS0 = GetBipartitePruningSorter0();
    PRUNING_SORTER *RS1 = GetBipartitePruningSorter1();
    const uint32_t *Sorted0 = RS0->Sort(MinPosList0, nb0).GetRanks();
    const uint32_t *Sorted1 = RS1->Sort(MinPosList1, nb1).GetRanks();

    // 3) Prune the lists
    uint32_t Index0, Index1;

    const uint32_t *const LastSorted0 = &Sorted0[nb0];
    const uint32_t *const LastSorted1 = &Sorted1[nb1];
    const uint32_t *RunningAddress0 = Sorted0;
    const uint32_t *RunningAddress1 = Sorted1;

    while (RunningAddress1 < LastSorted1 && Sorted0 < LastSorted0) {
        Index0 = *Sorted0++;

        while (RunningAddress1 < LastSorted1 && MinPosList1[*RunningAddress1] < MinPosList0[Index0]) {
            RunningAddress1++;
        }

        const uint32_t *RunningAddress2_1 = RunningAddress1;

        while (RunningAddress2_1 < LastSorted1
                && MinPosList1[Index1 = *RunningAddress2_1++] <= array0[Index0]->GetMax(Axis0)) {
            if (array0[Index0]->Intersect(*array1[Index1], Axis1)) {
                if (array0[Index0]->Intersect(*array1[Index1], Axis2)) {
                    pairs.AddPair(Index0, Index1);
                }
            }
        }
    }

    ////

    while (RunningAddress0 < LastSorted0 && Sorted1 < LastSorted1) {
        Index0 = *Sorted1++;

        while (RunningAddress0 < LastSorted0 && MinPosList0[*RunningAddress0] <= MinPosList1[Index0]) {
            RunningAddress0++;
        }

        const uint32_t *RunningAddress2_0 = RunningAddress0;

        while (RunningAddress2_0 < LastSorted0
                && MinPosList0[Index1 = *RunningAddress2_0++] <= array1[Index0]->GetMax(Axis0)) {
            if (array0[Index1]->Intersect(*array1[Index0], Axis1)) {
                if (array0[Index1]->Intersect(*array1[Index0], Axis2)) {
                    pairs.AddPair(Index1, Index0);
                }
            }

        }
    }

    DELETEARRAY(MinPosList1);
    DELETEARRAY(MinPosList0);

    return true;
}

#define ORIGINAL_VERSION
//#define JOAKIM

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Complete box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to the same set.
 *	\param		nb		[in] number of boxes
 *	\param		array	[in] array of boxes
 *	\param		pairs	[out] array of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Opcode::CompleteBoxPruning(uint32_t nb, const AABB **array, Pairs &pairs, const Axes &axes) {
    // Checkings
    if (!nb || !array) {
        return false;
    }

    // Catch axes
    uint32_t Axis0 = axes.mAxis0;
    uint32_t Axis1 = axes.mAxis1;
    uint32_t Axis2 = axes.mAxis2;

#ifdef ORIGINAL_VERSION
    // Allocate some temporary data
//	float* PosList = new float[nb];
    float *PosList = new float[nb + 1];

    // 1) Build main list using the primary axis
    for (uint32_t i = 0; i < nb; i++) {
        PosList[i] = array[i]->GetMin(Axis0);
    }
    PosList[nb++] = MAX_FLOAT;

    // 2) Sort the list
    PRUNING_SORTER *RS = GetCompletePruningSorter();
    const uint32_t *Sorted = RS->Sort(PosList, nb).GetRanks();

    // 3) Prune the list
    const uint32_t *const LastSorted = &Sorted[nb];
    const uint32_t *RunningAddress = Sorted;
    uint32_t Index0, Index1;
    while (RunningAddress < LastSorted && Sorted < LastSorted) {
        Index0 = *Sorted++;

//		while(RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);
        while (PosList[*RunningAddress++] < PosList[Index0]) {
        }

        if (RunningAddress < LastSorted) {
            const uint32_t *RunningAddress2 = RunningAddress;

//			while(RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=array[Index0]->GetMax(Axis0))
            while (PosList[Index1 = *RunningAddress2++] <= array[Index0]->GetMax(Axis0)) {
//				if(Index0!=Index1)
//				{
                if (array[Index0]->Intersect(*array[Index1], Axis1)) {
                    if (array[Index0]->Intersect(*array[Index1], Axis2)) {
                        pairs.AddPair(Index0, Index1);
                    }
                }
//				}
            }
        }
    }

    DELETEARRAY(PosList);
#endif

#ifdef JOAKIM
    // Allocate some temporary data
//	float* PosList = new float[nb];
    float* MinList = new float[nb+1];

    // 1) Build main list using the primary axis
    for(uint32_t i=0;i<nb;i++)	MinList[i] = array[i]->GetMin(Axis0);
    MinList[nb] = MAX_FLOAT;

    // 2) Sort the list
    PRUNING_SORTER* RS = GetCompletePruningSorter();
    uint32_t* Sorted = RS->Sort(MinList, nb+1).GetRanks();

    // 3) Prune the list
//	const uint32_t* const LastSorted = &Sorted[nb];
//	const uint32_t* const LastSorted = &Sorted[nb-1];
    const uint32_t* RunningAddress = Sorted;
    uint32_t Index0, Index1;

//	while(RunningAddress<LastSorted && Sorted<LastSorted)
//	while(RunningAddress<LastSorted)
    while(RunningAddress<&Sorted[nb])
//	while(Sorted<LastSorted)
    {
//		Index0 = *Sorted++;
        Index0 = *RunningAddress++;

//		while(RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);
//		while(PosList[*RunningAddress++]<PosList[Index0]);
//RunningAddress = Sorted;
//		if(RunningAddress<LastSorted)
        {
            const uint32_t* RunningAddress2 = RunningAddress;

//			while(RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=array[Index0]->GetMax(Axis0))

//			float CurrentMin = array[Index0]->GetMin(Axis0);
            float CurrentMax = array[Index0]->GetMax(Axis0);

            while(MinList[Index1 = *RunningAddress2] <= CurrentMax)
//			while(PosList[Index1 = *RunningAddress] <= CurrentMax)
            {
//				if(Index0!=Index1)
//				{
                    if(array[Index0]->Intersect(*array[Index1], Axis1))
                    {
                        if(array[Index0]->Intersect(*array[Index1], Axis2))
                        {
                            pairs.AddPair(Index0, Index1);
                        }
                    }
//				}

                RunningAddress2++;
//				RunningAddress++;
            }
        }
    }

    DELETEARRAY(MinList);
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Brute-force versions are kept:
// - to check the optimized versions return the correct list of intersections
// - to check the speed of the optimized code against the brute-force one
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Brute-force bipartite box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to a different set.
 *	\param		nb0		[in] number of boxes in the first set
 *	\param		array0	[in] array of boxes for the first set
 *	\param		nb1		[in] number of boxes in the second set
 *	\param		array1	[in] array of boxes for the second set
 *	\param		pairs	[out] array of overlapping pairs
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Opcode::BruteForceBipartiteBoxTest(uint32_t nb0,
        const AABB **array0,
        uint32_t nb1,
        const AABB **array1,
        Pairs &pairs) {
    // Checkings
    if (!nb0 || !array0 || !nb1 || !array1) {
        return false;
    }

    // Brute-force nb0*nb1 overlap tests
    for (uint32_t i = 0; i < nb0; i++) {
        for (uint32_t j = 0; j < nb1; j++) {
            if (array0[i]->Intersect(*array1[j])) {
                pairs.AddPair(i, j);
            }
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Complete box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to the same set.
 *	\param		nb		[in] number of boxes
 *	\param		array	[in] array of boxes
 *	\param		pairs	[out] array of overlapping pairs
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Opcode::BruteForceCompleteBoxTest(uint32_t nb, const AABB **array, Pairs &pairs) {
    // Checkings
    if (!nb || !array) {
        return false;
    }

    // Brute-force n(n-1)/2 overlap tests
    for (uint32_t i = 0; i < nb; i++) {
        for (uint32_t j = i + 1; j < nb; j++) {
            if (array[i]->Intersect(*array[j])) {
                pairs.AddPair(i, j);
            }
        }
    }
    return true;
}

