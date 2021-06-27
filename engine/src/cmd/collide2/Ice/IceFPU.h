///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains FPU related code.
 *	\file		IceFPU.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEFPU_H__
#define __ICEFPU_H__

	#define	SIGN_BITMASK			0x80000000

    union ice_union
    {
        ice_sdword sdword_member;
        ice_udword udword_member;
        float      float_member;
    };

	//! Integer representation of a floating-point value.
	#define IR(x)					((static_cast<ice_union*>((void *)&(x)))->udword_member)

	//! Signed integer representation of a floating-point value.
	#define SIR(x)					((static_cast<ice_union*>((void *)&(x)))->sdword_member)

	//! Absolute integer representation of a floating-point value
	#define AIR(x)					(IR(x)&0x7fffffff)

	//! Floating-point representation of an integer value.
	#define FR(x)					((static_cast<ice_union*>((void *)&(x)))->float_member)
	
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
