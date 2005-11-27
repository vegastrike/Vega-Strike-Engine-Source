/* boost random/exponential_distribution.hpp header file
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

#ifndef BOOST_RANDOM_EXPONENTIAL_DISTRIBUTION_HPP
#define BOOST_RANDOM_EXPONENTIAL_DISTRIBUTION_HPP

#include <cmath>
#include <cassert>
#include <boost/random/uniform_01.hpp>

namespace boost {

// exponential distribution: p(x) = lambda * exp(-lambda * x)
template<class UniformRandomNumberGenerator, class RealType = double>
class exponential_distribution
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef RealType result_type;

  exponential_distribution(base_type& rng, result_type lambda)
    : _rng(rng), _lambda(lambda) { assert(lambda > 0); }
  // compiler-generated copy ctor is fine
  // uniform_01 cannot be assigned, neither can this class
  result_type operator()()
  { 
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::log;
#endif
    return -1.0 / _lambda * log(1-_rng());
  }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const exponential_distribution& x, 
                         const exponential_distribution& y)
  { return x._lambda == y._lambda && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const exponential_distribution& rhs) const
  { return _lambda == rhs._lambda && _rng == rhs._rng;  }
#endif
private:
  uniform_01<base_type, RealType> _rng;
  const result_type _lambda;
};

} // namespace boost

#endif // BOOST_RANDOM_EXPONENTIAL_DISTRIBUTION_HPP
