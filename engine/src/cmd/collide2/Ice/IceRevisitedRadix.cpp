///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 * Updated by Benjamen R. Meyer 2025-01-14
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Revisited Radix Sort.
 *	This is my new radix routine:
 *  - it uses indices and doesn't recopy the values anymore, hence wasting less ram
 *  - it creates all the histograms in one run instead of four
 *  - it sorts words faster than dwords and bytes faster than words
 *  - it correctly sorts negative floating-point values by patching the offsets
 *  - it automatically takes advantage of temporal coherence
 *  - multiple keys support is a side effect of temporal coherence
 *  - it may be worth recoding in asm... (mainly to use FCOMI, FCMOV, etc) [it's probably memory-bound anyway]
 *
 *	History:
 *	- 08.15.98: very first version
 *	- 04.04.00: recoded for the radix article
 *	- 12.xx.00: code lifting
 *	- 09.18.01: faster CHECK_PASS_VALIDITY thanks to Mark D. Shattuck (who provided other tips, not included here)
 *	- 10.11.01: added local ram support
 *	- 01.20.02: bugfix! In very particular cases the last pass was skipped in the float code-path, leading to incorrect sorting......
 *	- 01.02.02:	- "mIndices" renamed => "mRanks". That's a rank sorter after all.
 *				- ranks are not "reset" anymore, but implicit on first calls
 *	- 07.05.02:	- offsets rewritten with one less indirection.
 *	- 11.03.02:	- "bool" replaced with RadixHint enum
 *
 *	\class		RadixSort
 *	\author		Pierre Terdiman
 *	\version	1.4
 *	\date		August, 15, 1998
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "cmd/collide2/Opcode.h"

/*
To do:
	- add an offset parameter between two input values (avoid some data recopy sometimes)
	- unroll ? asm ?
	- 11 bits trick & 3 passes as Michael did
	- prefetch stuff the day I have a P3
	- make a version with 16-bits indices ?
*/

using namespace Opcode;

#define INVALIDATE_RANKS    mCurrentSize|=0x80000000
#define VALIDATE_RANKS        mCurrentSize&=0x7fffffff
#define CURRENT_SIZE        (mCurrentSize&0x7fffffff)
#define INVALID_RANKS        (mCurrentSize&0x80000000)

#define CHECK_RESIZE(n)                                                                     \
    if(n!=mPreviousSize)                                                                    \
    {                                                                                       \
                if(n>mCurrentSize)    Resize(n);                                            \
        else                        ResetRanks();                                           \
        mPreviousSize = n;                                                                  \
    }

#define CREATE_HISTOGRAMS(type, buffer)                                                     \
    /* Clear counters/histograms */                                                         \
    ZeroMemory(mHistogram, 256*4*sizeof(uint32_t));                                         \
                                                                                            \
    /* Prepare to count */                                                                  \
    uint8_t* p = (uint8_t*)input;                                                           \
    uint8_t* pe = &p[nb*4];                                                                 \
    uint32_t* h0= &mHistogram[0];        /* Histogram for first pass (LSB)	*/              \
    uint32_t* h1= &mHistogram[256];    /* Histogram for second pass		*/                  \
    uint32_t* h2= &mHistogram[512];    /* Histogram for third pass			*/              \
    uint32_t* h3= &mHistogram[768];    /* Histogram for last pass (MSB)	*/                  \
                                                                                            \
    bool AlreadySorted = true;    /* Optimism... */                                         \
                                                                                            \
    if(INVALID_RANKS)                                                                       \
    {                                                                                       \
        /* Prepare for temporal coherence */                                                \
        type* Running = (type*)buffer;                                                      \
        type PrevVal = *Running;                                                            \
                                                                                            \
        while(p!=pe)                                                                        \
        {                                                                                   \
            /* Read input buffer in previous sorted order */                                \
            type Val = *Running++;                                                          \
            /* Check whether already sorted or not */                                       \
            if(Val<PrevVal)    { AlreadySorted = false; break; } /* Early out */            \
            /* Update for next iteration */                                                 \
            PrevVal = Val;                                                                  \
                                                                                            \
            /* Create histograms */                                                         \
            h0[*p++]++;    h1[*p++]++;    h2[*p++]++;    h3[*p++]++;                        \
        }                                                                                   \
                                                                                            \
        /* If all input values are already sorted, we just have to return and leave the */  \
        /* previous list unchanged. That way the routine may take advantage of temporal */  \
        /* coherence, for example when used to sort transparent faces.					*/  \
        if(AlreadySorted)                                                                   \
        {                                                                                   \
            mNbHits++;                                                                      \
            for(uint32_t i=0;i<nb;i++)    mRanks[i] = i;                                    \
            return *this;                                                                   \
        }                                                                                   \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        /* Prepare for temporal coherence */                                                \
        uint32_t* Indices = mRanks;                                                         \
        type PrevVal = (type)buffer[*Indices];                                              \
                                                                                            \
        while(p!=pe)                                                                        \
        {                                                                                   \
            /* Read input buffer in previous sorted order */                                \
            type Val = (type)buffer[*Indices++];                                            \
            /* Check whether already sorted or not */                                       \
            if(Val<PrevVal)    { AlreadySorted = false; break; } /* Early out */            \
            /* Update for next iteration */                                                 \
            PrevVal = Val;                                                                  \
                                                                                            \
            /* Create histograms */                                                         \
            h0[*p++]++;    h1[*p++]++;    h2[*p++]++;    h3[*p++]++;                        \
        }                                                                                   \
                                                                                            \
        /* If all input values are already sorted, we just have to return and leave the */  \
        /* previous list unchanged. That way the routine may take advantage of temporal */  \
        /* coherence, for example when used to sort transparent faces.					*/  \
        if(AlreadySorted)    { mNbHits++; return *this;    }                                \
    }                                                                                       \
                                                                                            \
    /* Else there has been an early out and we must finish computing the histograms */      \
    while(p!=pe)                                                                            \
    {                                                                                       \
        /* Create histograms without the previous overhead */                               \
        h0[*p++]++;    h1[*p++]++;    h2[*p++]++;    h3[*p++]++;                            \
    }

#define CHECK_PASS_VALIDITY(pass)                                                           \
    /* Shortcut to current counters */                                                      \
    uint32_t* CurCount = &mHistogram[pass<<8];                                              \
                                                                                            \
    /* Reset flag. The sorting pass is supposed to be performed. (default) */               \
    bool PerformPass = true;                                                                \
                                                                                            \
    /* Check pass validity */                                                               \
                                                                                            \
    /* If all values have the same byte, sorting is useless. */                             \
    /* It may happen when sorting bytes or words instead of dwords. */                      \
    /* This routine actually sorts words faster than dwords, and bytes */                   \
    /* faster than words. Standard running time (O(4*n))is reduced to O(2*n) */             \
    /* for words and O(n) for bytes. Running time for floats depends on actual values... */ \
                                                                                            \
    /* Get first byte */                                                                    \
    uint8_t UniqueVal = *(((uint8_t*)input)+pass);                                          \
                                                                                            \
    /* Check that byte's counter */                                                         \
    if(CurCount[UniqueVal]==nb)    PerformPass=false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort::RadixSort() : mCurrentSize(0), mRanks(nullptr), mRanks2(nullptr), mTotalCalls(0), mNbHits(0) {
#ifndef RADIX_LOCAL_RAM
    // Allocate input-independent ram
    mHistogram	= new uint32_t[256*4];
    mOffset		= new uint32_t[256];
#endif
    // Initialize indices
    INVALIDATE_RANKS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort::~RadixSort() {
    // Release everything
#ifndef RADIX_LOCAL_RAM
    DELETEARRAY(mOffset);
    DELETEARRAY(mHistogram);
#endif
    DELETEARRAY(mRanks2);
    DELETEARRAY(mRanks);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Resizes the inner lists.
 *	\param		nb	[in] new size (number of dwords)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RadixSort::Resize(uint32_t nb) {
    // Free previously used ram
    DELETEARRAY(mRanks2);
    DELETEARRAY(mRanks);

    // Get some fresh one
    mRanks = new uint32_t[nb];
    CHECKALLOC(mRanks);
    mRanks2 = new uint32_t[nb];
    CHECKALLOC(mRanks2);

    return true;
}

inline_ void RadixSort::CheckResize(uint32_t nb) {
    uint32_t CurSize = CURRENT_SIZE;
    if (nb != CurSize) {
        if (nb > CurSize) {
            Resize(nb);
        }
        mCurrentSize = nb;
        INVALIDATE_RANKS;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for integer values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input	[in] a list of integer values to sort
 *	\param		nb		[in] number of values to sort, must be < 2^31
 *	\param		hint	[in] RADIX_SIGNED to handle negative values, RADIX_UNSIGNED if you know your input buffer only contains positive values
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort &RadixSort::Sort(const uint32_t *input, uint32_t nb, RadixHint hint) {
    // Checkings
    if (!input || !nb || nb & 0x80000000) {
        return *this;
    }

    // Stats
    mTotalCalls++;

    // Resize lists if needed
    CheckResize(nb);

#ifdef RADIX_LOCAL_RAM
    // Allocate histograms & offsets on the stack
    uint32_t mHistogram[256 * 4];
//	uint32_t mOffset[256];
    uint32_t *mLink[256];
#endif

    // Create histograms (counters). Counters for all passes are created in one run.
    // Pros:	read input buffer once instead of four times
    // Cons:	mHistogram is 4Kb instead of 1Kb
    // We must take care of signed/unsigned values for temporal coherence.... I just
    // have 2 code paths even if just a single opcode changes. Self-modifying code, someone?
    if (hint == RADIX_UNSIGNED) {
        CREATE_HISTOGRAMS(uint32_t, input);
    } else {
        CREATE_HISTOGRAMS(int32_t, input);
    }

    // Compute #negative values involved if needed
    uint32_t NbNegativeValues = 0;
    if (hint == RADIX_SIGNED) {
        // An efficient way to compute the number of negatives values we'll have to deal with is simply to sum the 128
        // last values of the last histogram. Last histogram because that's the one for the Most Significant Byte,
        // responsible for the sign. 128 last values because the 128 first ones are related to positive numbers.
        uint32_t *h3 = &mHistogram[768];
        for (uint32_t i = 128; i < 256; i++) {
            NbNegativeValues += h3[i];
        }    // 768 for last histogram, 128 for negative part
    }

    // Radix sort, j is the pass number (0=LSB, 3=MSB)
    for (uint32_t j = 0; j < 4; j++) {
        CHECK_PASS_VALIDITY(j);

        // Sometimes the fourth (negative) pass is skipped because all numbers are negative and the MSB is 0xFF (for example). This is
        // not a problem, numbers are correctly sorted anyway.
        if (PerformPass) {
            // Should we care about negative values?
            if (j != 3 || hint == RADIX_UNSIGNED) {
                // Here we deal with positive values only

                // Create offsets
//				mOffset[0] = 0;
//				for(uint32_t i=1;i<256;i++)		mOffset[i] = mOffset[i-1] + CurCount[i-1];
                mLink[0] = mRanks2;
                for (uint32_t i = 1; i < 256; i++) {
                    mLink[i] = mLink[i - 1] + CurCount[i - 1];
                }
            } else {
                // This is a special case to correctly handle negative integers. They're sorted in the right order but at the wrong place.

                uint32_t i;
                // Create biased offsets, in order for negative numbers to be sorted as well
//				mOffset[0] = NbNegativeValues;												// First positive number takes place after the negative ones
                mLink[0] =
                        &mRanks2[NbNegativeValues];                                        // First positive number takes place after the negative ones
//				for(uint32_t i=1;i<128;i++)		mOffset[i] = mOffset[i-1] + CurCount[i-1];	// 1 to 128 for positive numbers
                for (i = 1; i < 128; i++) {
                    mLink[i] = mLink[i - 1] + CurCount[i - 1];
                }        // 1 to 128 for positive numbers

                // Fixing the wrong place for negative values
//				mOffset[128] = 0;
                mLink[128] = mRanks2;
//				for(i=129;i<256;i++)			mOffset[i] = mOffset[i-1] + CurCount[i-1];
                for (i = 129; i < 256; i++) {
                    mLink[i] = mLink[i - 1] + CurCount[i - 1];
                }
            }

            // Perform Radix Sort
            uint8_t *InputBytes = (uint8_t *) input;
            InputBytes += j;
            if (INVALID_RANKS) {
//				for(uint32_t i=0;i<nb;i++)	mRanks2[mOffset[InputBytes[i<<2]]++] = i;
                for (uint32_t i = 0; i < nb; i++) {
                    *mLink[InputBytes[i << 2]]++ = i;
                }
                VALIDATE_RANKS;
            } else {
                uint32_t *Indices = mRanks;
                uint32_t *IndicesEnd = &mRanks[nb];
                while (Indices != IndicesEnd) {
                    uint32_t id = *Indices++;
//					mRanks2[mOffset[InputBytes[id<<2]]++] = id;
                    *mLink[InputBytes[id << 2]]++ = id;
                }
            }

            // Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
            uint32_t *Tmp = mRanks;
            mRanks = mRanks2;
            mRanks2 = Tmp;
        }
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for floating-point values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input			[in] a list of floating-point values to sort
 *	\param		nb				[in] number of values to sort, must be < 2^31
 *	\return		Self-Reference
 *	\warning	only sorts IEEE floating-point values
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort &RadixSort::Sort(const float *input2, uint32_t nb) {
    // Checkings
    if (!input2 || !nb || nb & 0x80000000) {
        return *this;
    }

    // Stats
    mTotalCalls++;

    uint32_t *input = (uint32_t *) input2;

    // Resize lists if needed
    CheckResize(nb);

#ifdef RADIX_LOCAL_RAM
    // Allocate histograms & offsets on the stack
    uint32_t mHistogram[256 * 4];
//	uint32_t mOffset[256];
    uint32_t *mLink[256];
#endif

    // Create histograms (counters). Counters for all passes are created in one run.
    // Pros:	read input buffer once instead of four times
    // Cons:	mHistogram is 4Kb instead of 1Kb
    // Floating-point values are always supposed to be signed values, so there's only one code path there.
    // Please note the floating point comparison needed for temporal coherence! Although the resulting asm code
    // is dreadful, this is surprisingly not such a performance hit - well, I suppose that's a big one on first
    // generation Pentiums....We can't make comparison on integer representations because, as Chris said, it just
    // wouldn't work with mixed positive/negative values....
    { CREATE_HISTOGRAMS(float, input2); }

    // Compute #negative values involved if needed
    uint32_t NbNegativeValues = 0;
    // An efficient way to compute the number of negatives values we'll have to deal with is simply to sum the 128
    // last values of the last histogram. Last histogram because that's the one for the Most Significant Byte,
    // responsible for the sign. 128 last values because the 128 first ones are related to positive numbers.
    uint32_t *h3 = &mHistogram[768];
    uint32_t i;
    for (i = 128; i < 256; i++) {
        NbNegativeValues += h3[i];
    }    // 768 for last histogram, 128 for negative part

    // Radix sort, j is the pass number (0=LSB, 3=MSB)
    for (uint32_t j = 0; j < 4; j++) {
        // Should we care about negative values?
        if (j != 3) {
            // Here we deal with positive values only
            CHECK_PASS_VALIDITY(j);

            if (PerformPass) {
                // Create offsets
//				mOffset[0] = 0;
                mLink[0] = mRanks2;
//				for(uint32_t i=1;i<256;i++)		mOffset[i] = mOffset[i-1] + CurCount[i-1];
                for (uint32_t i = 1; i < 256; i++) {
                    mLink[i] = mLink[i - 1] + CurCount[i - 1];
                }

                // Perform Radix Sort
                uint8_t *InputBytes = (uint8_t *) input;
                InputBytes += j;
                if (INVALID_RANKS) {
//					for(i=0;i<nb;i++)	mRanks2[mOffset[InputBytes[i<<2]]++] = i;
                    for (uint32_t i = 0; i < nb; i++) {
                        *mLink[InputBytes[i << 2]]++ = i;
                    }
                    VALIDATE_RANKS;
                } else {
                    uint32_t *Indices = mRanks;
                    uint32_t *IndicesEnd = &mRanks[nb];
                    while (Indices != IndicesEnd) {
                        uint32_t id = *Indices++;
//						mRanks2[mOffset[InputBytes[id<<2]]++] = id;
                        *mLink[InputBytes[id << 2]]++ = id;
                    }
                }

                // Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
                uint32_t *Tmp = mRanks;
                mRanks = mRanks2;
                mRanks2 = Tmp;
            }
        } else {
            // This is a special case to correctly handle negative values
            CHECK_PASS_VALIDITY(j);

            if (PerformPass) {
                // Create biased offsets, in order for negative numbers to be sorted as well
//				mOffset[0] = NbNegativeValues;												// First positive number takes place after the negative ones
                mLink[0] =
                        &mRanks2[NbNegativeValues];                                        // First positive number takes place after the negative ones
//				for(uint32_t i=1;i<128;i++)		mOffset[i] = mOffset[i-1] + CurCount[i-1];	// 1 to 128 for positive numbers
                for (i = 1; i < 128; i++) {
                    mLink[i] = mLink[i - 1] + CurCount[i - 1];
                }        // 1 to 128 for positive numbers

                // We must reverse the sorting order for negative numbers!
//				mOffset[255] = 0;
                mLink[255] = mRanks2;
//				for(i=0;i<127;i++)		mOffset[254-i] = mOffset[255-i] + CurCount[255-i];	// Fixing the wrong order for negative values
                for (i = 0; i < 127; i++) {
                    mLink[254 - i] =
                            mLink[255 - i] + CurCount[255 - i];
                }        // Fixing the wrong order for negative values
//				for(i=128;i<256;i++)	mOffset[i] += CurCount[i];							// Fixing the wrong place for negative values
                for (i = 128; i < 256; i++) {
                    mLink[i] += CurCount[i];
                }                            // Fixing the wrong place for negative values

                // Perform Radix Sort
                if (INVALID_RANKS) {
                    for (uint32_t i = 0; i < nb; i++) {
                        uint32_t Radix = input[i]
                                >> 24;                            // Radix byte, same as above. AND is useless here (uint32_t).
                        // ### cmp to be killed. Not good. Later.
//						if(Radix<128)		mRanks2[mOffset[Radix]++] = i;		// Number is positive, same as above
//						else				mRanks2[--mOffset[Radix]] = i;		// Number is negative, flip the sorting order
                        if (Radix < 128)
                            *mLink[Radix]++ = i;        // Number is positive, same as above
                        else
                            *(--mLink[Radix]) = i;        // Number is negative, flip the sorting order
                    }
                    VALIDATE_RANKS;
                } else {
                    for (uint32_t i = 0; i < nb; i++) {
                        uint32_t Radix = input[mRanks[i]]
                                >> 24;                            // Radix byte, same as above. AND is useless here (uint32_t).
                        // ### cmp to be killed. Not good. Later.
//						if(Radix<128)		mRanks2[mOffset[Radix]++] = mRanks[i];		// Number is positive, same as above
//						else				mRanks2[--mOffset[Radix]] = mRanks[i];		// Number is negative, flip the sorting order
                        if (Radix < 128)
                            *mLink[Radix]++ = mRanks[i];        // Number is positive, same as above
                        else
                            *(--mLink[Radix]) = mRanks[i];        // Number is negative, flip the sorting order
                    }
                }
                // Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
                uint32_t *Tmp = mRanks;
                mRanks = mRanks2;
                mRanks2 = Tmp;
            } else {
                // The pass is useless, yet we still have to reverse the order of current list if all values are negative.
                if (UniqueVal >= 128) {
                    if (INVALID_RANKS) {
                        // ###Possible?
                        for (uint32_t i = 0; i < nb; i++) {
                            mRanks2[i] = nb - i - 1;
                        }
                        VALIDATE_RANKS;
                    } else {
                        for (uint32_t i = 0; i < nb; i++) {
                            mRanks2[i] = mRanks[nb - i - 1];
                        }
                    }

                    // Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
                    uint32_t *Tmp = mRanks;
                    mRanks = mRanks2;
                    mRanks2 = Tmp;
                }
            }
        }
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Gets the ram used.
 *	\return		memory used in bytes
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t RadixSort::GetUsedRam() const {
    uint32_t UsedRam = sizeof(RadixSort);
#ifndef RADIX_LOCAL_RAM
    UsedRam += 256*4*sizeof(uint32_t);			// Histograms
    UsedRam += 256*sizeof(uint32_t);				// Offsets
#endif
    UsedRam += 2 * CURRENT_SIZE * sizeof(uint32_t);    // 2 lists of indices
    return UsedRam;
}
