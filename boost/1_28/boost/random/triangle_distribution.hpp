/* boost random/triangle_distribution.hpp header file
 *
 * Copyright Jens Maurer 2000-2001
 * Permission to use, copy, modify, sell, and distribute this software
 * is hereby granted without fee provided that the above copyright notice
 * appears in all copies and that both that copyright notice and this
 * permission notice appear in supporting documentation,
 *
 * Jens Maurer makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 *
 * Revision history
 *  2001-02-18  moved to individual header files
 */

#ifndef BOOST_RANDOM_TRIANGLE_DISTRIBUTION_HPP
#define BOOST_RANDOM_TRIANGLE_DISTRIBUTION_HPP

#include <cmath>
#include <cassert>
#include <boost/random/uniform_01.hpp>

namespace boost {

// triangle distribution, with a smallest, b most probable, and c largest
// value.
template<class UniformRandomNumberGenerator, class RealType = double>
class triangle_distribution
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef RealType result_type;
  triangle_distribution(base_type & rng, result_type a, result_type b,
                        result_type c)
    : _rng(rng), _a(a), _b(b), _c(c),
      d1(_b-_a), d2(_c-_a), d3(_c-_b), q1(d1/d2), p1(d1*d2)
  {
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::sqrt;
#endif
    d3 = sqrt(d3);
    p1 = sqrt(p1);
    assert(_a <= _b && _b <= _c);
  }
  // compiler-generated copy ctor is fine
  // uniform_01 cannot be assigned, neither can this class
  result_type operator()()
  {
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::sqrt;
#endif
    result_type u = _rng();
    if( u <= q1 )
      return _a + p1*sqrt(u);
    else
      return _c - d3*sqrt(d2*u-d1);
  }
#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const triangle_distribution& x, 
                         const triangle_distribution& y)
  { return x._a == y._a && x._b == y._b && x._c == y._c && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const triangle_distribution& rhs) const
  { return _a == rhs._a && _b == rhs._b && _c == rhs._c && _rng == rhs._rng;  }
#endif
private:
  uniform_01<base_type, result_type> _rng;
  result_type _a, _b, _c;
  result_type d1, d2, d3, q1, p1;
};

} // namespace boost

#endif // BOOST_RANDOM_TRIANGLE_DISTRIBUTION_HPP
