/*
 * Vega Strike
 * Copyright (C) 2002 David Jeffery
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

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
	// get the exponent and remove its offset
	tmpexp = (EXP_MASK & tmp.u32_val) >> SIGNIF_BITS;
	tmpexp -= 127;
	// get the significand but leave one extra bit for use while performing
	// rounding later
	tmpsig = (SIGNIF_MASK & tmp.u32_val) >> (SIGNIF_BITS - significand - 1);
	// if this will be a normalized number, perform rounding
	if(tmpexp > 0 && tmpsig & 1){
		tmpsig++;
		if(tmpsig & (1 << (significand + 1))){
			tmpexp++;
			tmpsig -=  1 << (significand + 1);
		}
	}
	// remove the bit saved for rounding
	tmpsig >>= 1;
	if(tmpexp == 0){
		if(tmpsig & 1) // more rounding fun
			tmpsig++;
		if(tmpsig & (1 << significand)){
			tmpexp++;
			tmpsig -= 1 << significand;
		}else{
			tmpsig |= 1 << significand;
			tmpsig >>= 1;
		}
	} else if( tmpexp < 0){
		tmpsig |= 1 << significand;
		tmpsig >>= -tmpexp;
		if(tmpsig & 1)
			tmpsig++;
		tmpsig >>=1;
		tmpexp = 0;
	// if the number is too big, clamp it to the maximum represented value
	} else if(tmpexp >= 1 << exponent){
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
	// if a denormalized value, rebuild to normalized form
	else if( tmpexp == 0){
		tmpsig <<=1;
		while(!(tmpsig & (1 << significand))){
			tmpexp -=1;
			tmpsig <<= 1;
		}
		tmpsig -= 1 << significand;
	}
	tmpexp += 127;
	tmpsig <<= SIGNIF_BITS - significand;
	
	newval.u32_val |= tmpexp << SIGNIF_BITS;
	newval.u32_val |= tmpsig;
	
	return newval.fl_val;
}

