#include "lerp.h"


Transformation linear_interpolate_uncapped(const Transformation &A, const Transformation &B, double blend) {
  
  Quaternion result;
  if (A.orientation==B.orientation) {
    result = A.orientation;
  }else {
    Quaternion a = A.orientation;
    Quaternion b = B.orientation;
    double	f = blend, f0, f1;
    double	cos_omega = DotProduct (a.v, b.v) + a.s * b.s;
    
    // Adjust signs if necessary.
    if (cos_omega < 0) {
      cos_omega = -cos_omega;
      b.v = -b.v;
      b.s = -b.s;
    }
    
    if (cos_omega < 0.99) {
      // Do the spherical interp.
      double	omega = acos(cos_omega);
      double	sin_omega = sin(omega);
      f0 = sin((1 - f) * omega) / sin_omega;
      f1 = sin(f * omega) / sin_omega;
    }
    else {
      // Quaternions are close; just do straight lerp and avoid division by near-zero.
      f0 = 1 - f;
      f1 = f;
    }
    result.s = a.s * f0 + b.s * f1;
    result.v = a.v * f0 + b.v * f1;
    result.Normalize();
  }
  Transformation res(result, (A.position*(1-blend)) + (B.position * blend));

  return res;
}
