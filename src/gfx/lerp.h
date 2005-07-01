#ifndef _GFX_LERP_H_
#define _GFX_LERP_H_

#include <assert.h>
#include "vec.h"
#include "quaternion.h"

// Interpolates between the transformations A and B.  Caps the blending factor at 1.0, but not at 0.0
Transformation linear_interpolate(const Transformation &a, const Transformation &b, double blend);

// Interpolates A and B, but does not cap the interpolation to less than 1.0
Transformation linear_interpolate_uncapped(const Transformation &a, const Transformation &b, double blend);

#endif

