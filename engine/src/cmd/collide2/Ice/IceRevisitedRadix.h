///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Copyright (C) 2000-2025 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RADIXSORT_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RADIXSORT_H

//! Allocate histograms & offsets locally
#define RADIX_LOCAL_RAM

enum RadixHint {
    RADIX_SIGNED,        //!< Input values are signed
    RADIX_UNSIGNED,        //!< Input values are unsigned

    RADIX_FORCE_DWORD = 0x7fffffff
};

class ICECORE_API RadixSort {
public:
    // Constructor/Destructor
    RadixSort();
    ~RadixSort();
    // Sorting methods
    RadixSort &Sort(const uint32_t *input, uint32_t nb, RadixHint hint = RADIX_SIGNED);
    RadixSort &Sort(const float *input, uint32_t nb);

    //! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
    inline_    const uint32_t *GetRanks() const {
        return mRanks;
    }

    //! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
    inline_    uint32_t *GetRecyclable() const {
        return mRanks2;
    }

    // Stats
    uint32_t GetUsedRam() const;
    //! Returns the total number of calls to the radix sorter.
    inline_    uint32_t GetNbTotalCalls() const {
        return mTotalCalls;
    }
    //! Returns the number of eraly exits due to temporal coherence.
    inline_    uint32_t GetNbHits() const {
        return mNbHits;
    }

private:
#ifndef RADIX_LOCAL_RAM
    uint32_t*			mHistogram;			//!< Counters for each byte
    uint32_t*			mOffset;			//!< Offsets (nearly a cumulative distribution function)
#endif
    uint32_t mCurrentSize;        //!< Current size of the indices list
    uint32_t *mRanks;                //!< Two lists, swapped each pass
    uint32_t *mRanks2;
    // Stats
    uint32_t mTotalCalls;        //!< Total number of calls to the sort routine
    uint32_t mNbHits;            //!< Number of early exits due to coherence
    // Internal methods
    void CheckResize(uint32_t nb);
    bool Resize(uint32_t nb);
};

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_RADIXSORT_H
