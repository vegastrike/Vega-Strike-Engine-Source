///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains source code from the article "Radix Sort Revisited".
 *	\file		IceRevisitedRadix.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERADIXSORT_H__
#define __ICERADIXSORT_H__

	//! Allocate histograms & offsets locally
	#define RADIX_LOCAL_RAM

	enum RadixHint
	{
		RADIX_SIGNED,		//!< Input values are signed
		RADIX_UNSIGNED,		//!< Input values are unsigned

		RADIX_FORCE_DWORD = 0x7fffffff
	};

	class ICECORE_API RadixSort
	{
		public:
		// Constructor/Destructor
								RadixSort();
								~RadixSort();
		// Sorting methods
				RadixSort&		Sort(const ice_udword* input, ice_udword nb, RadixHint hint=RADIX_SIGNED);
				RadixSort&		Sort(const float* input, ice_udword nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		inline_	const ice_udword*	GetRanks()			const	{ return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		inline_	ice_udword*			GetRecyclable()		const	{ return mRanks2;		}

		// Stats
				ice_udword			GetUsedRam()		const;
		//! Returns the total number of calls to the radix sorter.
		inline_	ice_udword			GetNbTotalCalls()	const	{ return mTotalCalls;	}
		//! Returns the number of eraly exits due to temporal coherence.
		inline_	ice_udword			GetNbHits()			const	{ return mNbHits;		}

		private:
#ifndef RADIX_LOCAL_RAM
				ice_udword*			mHistogram;			//!< Counters for each byte
				ice_udword*			mOffset;			//!< Offsets (nearly a cumulative distribution function)
#endif
				ice_udword			mCurrentSize;		//!< Current size of the indices list
				ice_udword*			mRanks;				//!< Two lists, swapped each pass
				ice_udword*			mRanks2;
		// Stats
				ice_udword			mTotalCalls;		//!< Total number of calls to the sort routine
				ice_udword			mNbHits;			//!< Number of early exits due to coherence
		// Internal methods
				void			CheckResize(ice_udword nb);
				bool			Resize(ice_udword nb);
	};

#endif // __ICERADIXSORT_H__
