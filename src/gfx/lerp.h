#ifndef _GFX_LERP_H_
#define _GFX_LERP_H_

#include <assert.h>
#include "vec.h"
#include "quaternion.h"

Transformation linear_interpolate(const Transformation &a, const Transformation &b, double blend);

#endif

