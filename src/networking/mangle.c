#include "mangle.h"

union type_conv{
	float fl_val;
	unsigned int u32_val;
};


unsigned int float_to_uint32(float value, char exponent, char significand){
	union type_conv tmp;
	unsigned int newval, tmpsig;
	int tmpexp;
	
	tmp.fl_val = value;
	// set the sign bit
	newval = SIGN_MASK & tmp.u32_val;
	newval >>= EXP_BITS + SIGNIF_BITS - (exponent + significand);
	// get the exponent
	tmpexp = (EXP_MASK & tmp.u32_val) >> SIGNIF_BITS;
	// get the significand and round properly
	tmpsig = (SIGNIF_MASK & tmp.u32_val) >> (SIGNIF_BITS - significand - 1);
	if(tmpsig & 1){ //time to have fun rounding
		tmpsig++;
		if(tmpsig & (1 << (significand + 1))){
			tmpexp++;
			tmpsig -=  1 << (significand + 1);
		}
	}
	tmpsig >>= 1;
	//reset the base offset of the exponent
	tmpexp -= 127;
	tmpexp += (1 << (exponent - 1)) - 1;
	// if the number would be zero or too small to be represented,
	// return 0 or -0
	if(tmpexp < 0 || ( tmpexp == 0 && tmpsig == 0))
		return newval;
	// if the number is too big, clamp it to the maximum represented value
	else if(tmpexp >= 1 << exponent){
		tmpexp = BITMASK(exponent);
		tmpsig = BITMASK(significand);
	}
	newval |= (tmpexp << significand) | tmpsig;
	return newval;
}

float uint32_to_float(unsigned int value, char exponent, char significand){
	union type_conv newval;
	unsigned int tmpsig;
	int tmpexp;
	
	newval.u32_val = (1 << (exponent + significand)) & value;
	newval.u32_val <<= 31 - (exponent + significand);
	tmpsig = value & BITMASK(significand);
	tmpexp = value & (BITMASK(exponent) << significand);
	tmpexp >>= significand;
	// if the number should be 0 or -0, go ahead and return
	if(tmpexp == 0 && tmpsig == 0)
		return newval.fl_val;
	tmpexp -= (1 << (exponent - 1)) - 1;
	tmpexp += 127;
	tmpsig <<= SIGNIF_BITS - significand;
	
	newval.u32_val |= tmpexp << SIGNIF_BITS;
	newval.u32_val |= tmpsig;
	
	return newval.fl_val;
}

