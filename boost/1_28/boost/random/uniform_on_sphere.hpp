/* boost random/uniform_on_sphere.hpp header file
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

#ifndef BOOST_RANDOM_UNIFORM_ON_SPHERE_HPP
#define BOOST_RANDOM_UNIFORM_ON_SPHERE_HPP

#include <vector>
#include <algorithm>     // std::transform
#include <functional>    // std::bind2nd, std::divides
#include <boost/random/normal_distribution.hpp>

namespace boost {

template<class UniformRandomNumberGenerator, class RealType = double,
   class Cont = std::vector<RealType> >
class uniform_on_sphere
{
public:
  typedef UniformRandomNumberGenerator base_type;
  typedef Cont result_type;

  explicit uniform_on_sphere(base_type & rng, int dim = 2)
    : _rng(rng), _container(dim), _dim(dim) { }
  // compiler-generated copy ctor is fine
  // normal_distribution cannot be assigned, neither can this class
  const result_type & operator()()
  {
    RealType sqsum = 0;
    for(typename Cont::iterator it = _container.begin();
        it != _container.end();
        ++it) {
      RealType val = _rng();
      *it = val;
      sqsum += val * val;
    }
#ifndef BOOST_NO_STDC_NAMESPACE
    using std::sqrt;
#endif
    // for all i: result[i] /= sqrt(sqsum)
    std::transform(_container.begin(), _container.end(), _container.begin(),
                   std::bind2nd(std::divides<RealType>(), sqrt(sqsum)));
    return _container;
  }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend bool operator==(const uniform_on_sphere& x, 
                         const uniform_on_sphere& y)
  { return x._dim == y._dim && x._rng == y._rng; }
#else
  // Use a member function
  bool operator==(const uniform_on_sphere& rhs) const
  { return _dim == rhs._dim && _rng == rhs._rng; }
#endif
private:
  normal_distribution<base_type, RealType> _rng;
  result_type _container;
  const int _dim;
};

} // namespace boost

#endif // BOOST_RANDOM_UNIFORM_ON_SPHERE_HPP
