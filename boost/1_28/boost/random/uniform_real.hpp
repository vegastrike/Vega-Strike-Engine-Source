/* boost random/uniform_real.hpp header file
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
 *  2001-04-08  added min<max assertion (N. Becker)
 *  2001-02-18  moved to individual header files
 */

#ifndef BOOST_RANDOM_UNIFORM_REAL_HPP
#define BOOST_RANDOM_UNIFORM_REAL_HPP

#include <cassert>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/random/uniform_01.hpp>

namespace boost {

// uniform distribution on a real range
template<class UniformRandomNumberGenerator, class RealType = double>
class uniform_real
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef RealType result_type;
  BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);

  uniform_real(base_type & rng, RealType min, RealType max) 
    : _rng(rng), _min(min), _max(max)
  {
#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    BOOST_STATIC_ASSERT(!std::numeric_limits<RealType>::is_integer);
#endif
    assert(min < max);
  }
  // compiler-generated copy ctor is fine
  // uniform_01 cannot be assigned, neither can this class

  result_type operator()() { return _rng() * (_max - _min) + _min; }
  result_type min() const { return _min; }
  result_type max() const { return _max; }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const uniform_real& x, const uniform_real& y)
  { return x._min == y._min && x._max == y._max && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const uniform_real& rhs) const
  { return _min == rhs._min && _max == rhs._max && _rng == rhs._rng;  }
#endif
private:
  uniform_01<base_type, result_type> _rng;
  RealType _min, _max;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<class UniformRandomNumberGenerator, class RealType>
const bool uniform_real<UniformRandomNumberGenerator, RealType>::has_fixed_range;
#endif

} // namespace boost

#endif // BOOST_RANDOM_UNIFORM_REAL_HPP
