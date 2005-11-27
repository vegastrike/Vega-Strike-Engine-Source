/* boost random/geometric_distribution.hpp header file
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

#ifndef BOOST_RANDOM_GEOMETRIC_DISTRIBUTION_HPP
#define BOOST_RANDOM_GEOMETRIC_DISTRIBUTION_HPP

#include <cmath>          // std::log
#include <cassert>
#include <boost/random/uniform_01.hpp>

namespace boost {

#if defined(__GNUC__) && (__GNUC__ < 3)
// Special gcc workaround: gcc 2.95.x ignores using-declarations
// in template classes (confirmed by gcc author Martin v. Loewis)
  using std::log;
#endif

// geometric distribution: p(i) = (1-p) * pow(p, i-1)   (integer)
template<class UniformRandomNumberGenerator, class IntType = int>
class geometric_distribution
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef IntType result_type;

  geometric_distribution(base_type & rng, double p)
    : _rng(rng)
  {
    assert(0.0 < p && p < 1.0);
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::log;
#endif
    _log_p = log(p);
  }
  // compiler-generated copy ctor is fine
  // uniform_01 cannot be assigned, neither can this class

  result_type operator()()
  {
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::log;
#endif
    return IntType (log(1-_rng()) / _log_p) + 1;
  }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const geometric_distribution& x, 
                         const geometric_distribution& y)
  { return x._log_p == y._log_p && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const geometric_distribution& rhs) const
  { return _log_p == rhs._log_p && _rng == rhs._rng;  }
#endif
private:
  uniform_01<base_type> _rng;
  typename uniform_01<base_type>::result_type _log_p;
};

} // namespace boost

#endif // BOOST_RANDOM_GEOMETRIC_DISTRIBUTION_HPP

