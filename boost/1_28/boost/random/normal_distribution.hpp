/* boost random/normal_distribution.hpp header file
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

#ifndef BOOST_RANDOM_NORMAL_DISTRIBUTION_HPP
#define BOOST_RANDOM_NORMAL_DISTRIBUTION_HPP

#include <cmath>
#include <cassert>
#include <boost/random/uniform_01.hpp>

namespace boost {

// deterministic polar method, uses trigonometric functions
template<class UniformRandomNumberGenerator, class RealType = double>
class normal_distribution
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef RealType result_type;

  explicit normal_distribution(base_type & rng, const result_type& mean = 0,
                               const result_type& sigma = 1)
    : _rng(rng), _mean(mean), _sigma(sigma), _valid(false)
  {
    assert(sigma >= 0);
  }

  // compiler-generated copy constructor is NOT fine, need to purge cache
  normal_distribution(const normal_distribution& other)
    : _rng(other._rng), _mean(other._mean), _sigma(other._sigma), _valid(false)
  {
  }
  // uniform_01 cannot be assigned, neither can this class

  result_type operator()()
  {
#ifndef BOOST_NO_STDC_NAMESPACE
    // allow for Koenig lookup
    using std::sqrt; using std::log; using std::sin; using std::cos;
#endif
    if(!_valid) {
      _r1 = _rng();
      _r2 = _rng();
      _cached_rho = sqrt(-2 * log(1.0-_r2));
      _valid = true;
    } else {
      _valid = false;
    }
    // Can we have a boost::mathconst please?
    const double pi = 3.14159265358979323846;
    
    return _cached_rho * (_valid ? cos(2*pi*_r1) : sin(2*pi*_r1)) * _sigma +
      _mean;
  }
#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const normal_distribution& x, 
                         const normal_distribution& y)
  {
    return x._mean == y._mean && x._sigma == y._sigma && 
      x._valid == y._valid && x._rng == y._rng;
  }
#else
  // Use a member function
  bool operator==(const normal_distribution& rhs) const
  {
    return _mean == rhs._mean && _sigma == rhs._sigma && 
      _valid == rhs._valid && _rng == rhs._rng;
  }
#endif
private:
  uniform_01<base_type, RealType> _rng;
  const result_type _mean, _sigma;
  result_type _r1, _r2, _cached_rho;
  bool _valid;
};

} // namespace boost

#endif // BOOST_RANDOM_NORMAL_DISTRIBUTION_HPP
