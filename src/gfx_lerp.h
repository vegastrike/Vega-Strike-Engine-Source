#ifndef _GFX_LERP_H_
#define _GFX_LERP_H_

#include <assert.h>
#include "gfx_transform_vector.h"

struct AffineTransform { // lerp between these
  Vector p,q,r;
  Vector position;
  
  AffineTransform(const Vector &p, const Vector &q, const Vector &r,
		  const Vector &position) : p(p), q(q), r(r), position(position) { }
  
};

AffineTransform interpolate(const AffineTransform &a, const AffineTransform &b, float slerp_direction);

#endif

