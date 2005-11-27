/* boost random/uniform_int.hpp header file
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

#ifndef BOOST_RANDOM_UNIFORM_INT_HPP
#define BOOST_RANDOM_UNIFORM_INT_HPP

#include <cassert>
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/detail/signed_unsigned_compare.hpp>

namespace boost {

// uniform integer distribution on [min, max]
template<class UniformRandomNumberGenerator, class IntType = int>
class uniform_int
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef IntType result_type;
  BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);

  uniform_int(base_type & rng, IntType min, IntType max) 
    : _rng(rng), _min(min), _max(max), _range(_max - _min),
      _bmin(_rng.min()), _brange(_rng.max() - _bmin)
  {
#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    BOOST_STATIC_ASSERT(std::numeric_limits<IntType>::is_integer);
#endif
    assert(min < max);
    if(random::equal_signed_unsigned(_brange, _range))
      _range_comparison = 0;
    else if(random::lessthan_signed_unsigned(_brange, _range))
      _range_comparison = -1;
    else
      _range_comparison = 1;
  }
  result_type operator()();
  result_type min() const { return _min; }
  result_type max() const { return _max; }
#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const uniform_int& x, const uniform_int& y)
  { return x._min == y._min && x._max == y._max && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const uniform_int& rhs) const
  { return _min == rhs._min && _max == rhs._max && _rng == rhs._rng;  }
#endif
private:
  typedef typename base_type::result_type base_result;
  base_type & _rng;
  result_type _min, _max, _range;
  base_result _bmin, _brange;
  int _range_comparison;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<class UniformRandomNumberGenerator, class IntType>
const bool uniform_int<UniformRandomNumberGenerator, IntType>::has_fixed_range;
#endif

template<class UniformRandomNumberGenerator, class IntType>
inline IntType uniform_int<UniformRandomNumberGenerator, IntType>::operator()()
{
  if(_range_comparison == 0) {
    // this will probably never happen in real life
    // basically nothing to do; just take care we don't overflow / underflow
    return static_cast<result_type>(_rng() - _bmin) + _min;
  } else if(_range_comparison < 0) {
    // use rejection method to handle things like 0..3 --> 0..4
    for(;;) {
      // concatenate several invocations of the base RNG
      // take extra care to avoid overflows
      result_type limit;
      if(_range == std::numeric_limits<result_type>::max()) {
        limit = _range/(static_cast<result_type>(_brange)+1);
        if(_range % static_cast<result_type>(_brange)+1 == static_cast<result_type>(_brange))
          ++limit;
      } else {
        limit = (_range+1)/(static_cast<result_type>(_brange)+1);
      }
      // we consider "result" as expressed to base (_brange+1)
      // for every power of (_brange+1), we determine a random factor
      result_type result = 0;
      result_type mult = 1;
      while(mult <= limit) {
        result += (_rng() - _bmin) * mult;
        mult *= static_cast<result_type>(_brange)+1;
      }
      if(mult == limit)
        // _range+1 is an integer power of _brange+1: no rejections required
        return result;
      // _range/mult < _brange+1  -> no endless loop
      result += uniform_int<base_type,result_type>(_rng, 0, _range/mult)() * mult;
      if(result <= _range)
        return result + _min;
    }
  } else {                   // brange > range
    if(_brange / _range > 4 /* quantization_cutoff */ ) {
      // the new range is vastly smaller than the source range,
      // so quantization effects are not relevant
      return uniform_smallint<base_type,result_type>(_rng, _min, _max)();
    } else {
      // use rejection method to handle things like 0..5 -> 0..4
      for(;;) {
        base_result result = _rng() - _bmin;
        // result and range are non-negative, and result is possibly larger
        // than range, so the cast is safe
        if(result <= static_cast<base_result>(_range))
          return result + _min;
      }
    }
  }
}

} // namespace boost

#endif // BOOST_RANDOM_UNIFORM_INT_HPP
