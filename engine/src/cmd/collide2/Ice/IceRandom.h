///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for random generators.
 *	\file		IceRandom.h
 *	\author		Pierre Terdiman
 *	\date		August, 9, 2001
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICERANDOM_H__
#define __ICERANDOM_H__

	ICECORE_API	void	SRand(ice_udword seed);
	 ice_udword	Rand();

	//! Returns a unit random floating-point value
	inline_ float UnitRandomFloat()	{ return float(Rand()) * ONE_OVER_RAND_MAX;	}

	//! Returns a random index so that 0<= index < max_index
	ICECORE_API	ice_udword GetRandomIndex(ice_udword max_index);

	class ICECORE_API BasicRandom
	{
		public:

		//! Constructor
		inline_				BasicRandom(ice_udword seed=0)	: mRnd(seed)	{}
		//! Destructor
		inline_				~BasicRandom()								{}

		inline_	void		SetSeed(ice_udword seed)		{ mRnd = seed;											}
		inline_	ice_udword		GetCurrentValue()	const	{ return mRnd;											}
		inline_	ice_udword		Randomize()					{ mRnd = mRnd * 2147001325 + 715136305; return mRnd;	}

		private:
				ice_udword		mRnd;
	};

#endif // __ICERANDOM_H__

