#include "lerp.h"
#include <iostream>
//Portions adapted from Tuxracer
using std::ostream;

static ostream &operator<<(ostream &os, const Quaternion &v) {
  os << "(" << v.s << ", <" << v.v.i << ", " << v.v.j << ", " << v.v.k << ">)";
  return os;
}

static ostream &operator<<(ostream &os, const Transformation &v) {
  os << "[" << v.orientation << ", <" << v.position.i << ", " << v.position.j << ", " << v.position.k << ">]";
  return os;
}


Transformation linear_interpolate(const Transformation &A, const Transformation &B, double blend) {
  if(blend>1.0) blend = 1.0;

  Quaternion result, a = A.orientation, b = B.orientation;
  double	f = blend, f0, f1;
  double	cos_omega = a.v * b.v + a.s * b.s;
  
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
  
	
  Transformation res(result, A.position + (B.position - A.position) * blend);

  //clog << "Interpolate from " << a << " to " << b << ", factor " << blend << " = " << res << "\n";

  return res;
}
