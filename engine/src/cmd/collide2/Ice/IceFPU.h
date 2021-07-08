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

	//! Integer representation of a floating-point value.
    inline uint32_t IR(uint32_t x)
    {
        return x;
    }
    inline uint32_t IR(int32_t x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->uint32_member;
    }
    inline uint32_t IR(float x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->uint32_member;
    }

	//! Signed integer representation of a floating-point value.
    inline int32_t SIR(uint32_t x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->sint32_member;
    }
    inline int32_t SIR(int32_t x)
    {
        return x;
    }
    inline int32_t SIR(float x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->sint32_member;
    }

	//! Absolute integer representation of a floating-point value
	#define AIR(x)					(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
    inline float FR(uint32_t x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->float_member;
    }
    inline float FR(int32_t x)
    {
        return static_cast<ice_union*>(static_cast<void*>(&(x)))->float_member;
    }
    inline float FR(float x)
    {
        return x;
    }
	
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
