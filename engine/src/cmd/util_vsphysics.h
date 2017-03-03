/**
 * A work around to overrule the cmath copysign function
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
