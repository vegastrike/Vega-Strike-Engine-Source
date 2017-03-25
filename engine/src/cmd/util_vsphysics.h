/**
 * Util Vsphysics
 * 
 * Vega Strike overloads the copysign function from cmath, this 
 * namespace is there to make sure that it is this method that is 
 * preferred.
 */


#ifndef UTIL_VSPHYSICS
#define UTIL_VSPHYSICS

namespace vsphysics
{
	float copysign( float x, float y )
	{
		if (y > 0)
			return x;
		else
			return -x;
	}
}
#endif
