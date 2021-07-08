///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains FPU related code.
 *	\file		IceFPU.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 *
 *  Updated by Stephen G. Tuggy 2021-07-07
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEFPU_H__
#define __ICEFPU_H__

	#define	SIGN_BITMASK			0x80000000

    union ice_union
    {
        int32_t sint32_member;
        uint32_t uint32_member;
        float      float_member;
    };

    // 2021-07-07 SGT: It seems that the below macros cannot be converted to inline functions, 
    //                 because they need to be assignable in some contexts.
    //! Integer representation of a floating-point value.
    #define IR(x)                   (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->uint32_member)

	//! Signed integer representation of a floating-point value.
    #define SIR(x)                  (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->sint32_member)

	//! Absolute integer representation of a floating-point value
	#define AIR(x)					(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
    #define FR(x)                   (static_cast<ice_union*>(const_cast<void*>(static_cast<const void*>(&(x))))->float_member)
	
	//! Is the float valid ?
	inline_ bool IsNAN(float value)				{ return (IR(value)&0x7f800000) == 0x7f800000;	}
	inline_ bool IsIndeterminate(float value)	{ return IR(value) == 0xffc00000;				}
	inline_ bool IsPlusInf(float value)			{ return IR(value) == 0x7f800000;				}
	inline_ bool IsMinusInf(float value)		{ return IR(value) == 0xff800000;				}

	inline_	bool IsValidFloat(float value)
	{
		if(IsNAN(value))			return false;
		if(IsIndeterminate(value))	return false;
		if(IsPlusInf(value))		return false;
		if(IsMinusInf(value))		return false;
		return true;
	}

	#define CHECK_VALID_FLOAT(x)	OPASSERT(IsValidFloat(x));


#endif // __ICEFPU_H__
