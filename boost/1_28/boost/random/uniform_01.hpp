/* boost random/uniform_01.hpp header file
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

#ifndef BOOST_RANDOM_UNIFORM_01_HPP
#define BOOST_RANDOM_UNIFORM_01_HPP

#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>

namespace boost {

// Because it is so commonly used: uniform distribution on the real [0..1)
// range.  This allows for specializations to avoid a costly FP division
template<class UniformRandomNumberGenerator, class RealType = double>
class uniform_01
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef RealType result_type;
  BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);

  explicit uniform_01(base_type & rng) : _rng(rng) { 

#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    BOOST_STATIC_ASSERT(!std::numeric_limits<RealType>::is_integer);
#endif
  }
  // compiler-generated copy ctor is fine
  // assignment is disallowed because there is a reference member

  result_type operator()() {
    return static_cast<result_type>(_rng() - _rng.min()) /
      (static_cast<result_type>(_rng.max()-_rng.min()) +
       (std::numeric_limits<base_result>::is_integer ? 1.0 : 0.0));
  }
  result_type min() const { return 0.0; }
  result_type max() const { return 1.0; }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const uniform_01& x, const uniform_01& y)
  { return x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const uniform_01& rhs) const
  { return _rng == rhs._rng;  }
#endif
private:
  typedef typename base_type::result_type base_result;
  base_type & _rng;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<class UniformRandomNumberGenerator, class RealType>
const bool uniform_01<UniformRandomNumberGenerator, RealType>::has_fixed_range;
#endif

} // namespace boost

#endif // BOOST_RANDOM_UNIFORM_01_HPP
