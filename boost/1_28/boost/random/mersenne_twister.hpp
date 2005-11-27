/* boost random/mersenne_twister.hpp header file
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

#ifndef BOOST_RANDOM_MERSENNE_TWISTER_HPP
#define BOOST_RANDOM_MERSENNE_TWISTER_HPP

#include <iostream>
#include <algorithm>     // std::copy
#include <boost/config.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer_traits.hpp>
#include <boost/cstdint.hpp>
#include <boost/random/linear_congruential.hpp>

namespace boost {
namespace random {

// http://www.math.keio.ac.jp/matumoto/emt.html
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
class mersenne_twister
{
public:
  typedef DataType result_type;
  BOOST_STATIC_CONSTANT(int, state_size = n);
  BOOST_STATIC_CONSTANT(int, shift_size = m);
  BOOST_STATIC_CONSTANT(int, mask_bits = r);
  BOOST_STATIC_CONSTANT(DataType, parameter_a = a);
  BOOST_STATIC_CONSTANT(int, output_u = u);
  BOOST_STATIC_CONSTANT(int, output_s = s);
  BOOST_STATIC_CONSTANT(DataType, output_b = b);
  BOOST_STATIC_CONSTANT(int, output_t = t);
  BOOST_STATIC_CONSTANT(DataType, output_c = c);
  BOOST_STATIC_CONSTANT(int, output_l = l);
#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
  static const bool has_fixed_range = true;
  static const result_type min_value = integer_traits<result_type>::const_min;
  static const result_type max_value = integer_traits<result_type>::const_max;
#else
  BOOST_STATIC_CONSTANT(bool, has_fixed_range = false);
#endif
  result_type min() const { return std::numeric_limits<result_type>::min(); }
  result_type max() const { return std::numeric_limits<result_type>::max(); }
  
  mersenne_twister() { seed(); }

#if defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x520)
  // Work around overload resolution problem (Gennadiy E. Rozental)
  explicit mersenne_twister(const DataType& value)
#else
  explicit mersenne_twister(DataType value)
#endif
  { seed(value); }

  template<class Generator>
  explicit mersenne_twister(Generator & gen) { seed(gen); }

  // compiler-generated copy ctor and assignment operator are fine

  void seed() { seed(DataType(4357)); }

#if defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x520)
  // Work around overload resolution problem (Gennadiy E. Rozental)
  void seed(const DataType& value)
#else
  void seed(DataType value)
#endif
  {
    random::linear_congruential<uint32_t, 69069, 0, 0, /* unknown */ 0> 
      gen(value);
    seed(gen);
  }

  // For GCC, moving this function out-of-line prevents inlining, which may
  // reduce overall object code size.  However, MSVC does not grok
  // out-of-line definitions of member function templates.
  template<class Generator>
  void seed(Generator & gen)
  {
#ifndef BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS
    BOOST_STATIC_ASSERT(!std::numeric_limits<result_type>::is_signed);
#endif
    // I could have used std::generate_n, but it takes "gen" by value
    for(int j = 0; j < n; j++)
      x[j] = gen();
    i = n;
  }
  
  result_type operator()();
  bool validation(result_type v) const { return val == v; }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
  friend std::ostream& operator<<(std::ostream& os, const mersenne_twister& mt)
  {
    os << mt.i << " ";
    std::copy(mt.x, mt.x+n, std::ostream_iterator<data_type>(os, " "));
    return os;
  }
  friend std::istream& operator>>(std::istream& is, mersenne_twister& mt)
  {
    is >> mt.i >> std::ws;
    for(int i = 0; i < mt.state_size; ++i)
      is >> mt.x[i] >> std::ws;
    return is;
  }
  friend bool operator==(const mersenne_twister& x, const mersenne_twister& y)
  { return x.i == y.i && std::equal(x.x, x.x+n, y.x); }
#else
  // Use a member function; Streamable concept not supported.
  bool operator==(const mersenne_twister& rhs) const
  { return i == rhs.i && std::equal(x, x+n, rhs.x); }
#endif

private:
  typedef DataType data_type;
  void twist();
  int i;
  data_type x[n];
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
//  A definition is required even for integral static constants
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const bool mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::has_fixed_range;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const typename mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::result_type mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::min_value;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const typename mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::result_type mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::max_value;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::state_size;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::shift_size;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::mask_bits;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const DataType mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::parameter_a;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_u;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_s;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const DataType mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_b;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_t;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const DataType mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_c;
template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
const int mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::output_l;
#endif

template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
void mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::twist()
{
  const data_type upper_mask = (~0u) << r;
  const data_type lower_mask = ~upper_mask;
  /*
  for(int j = 0; j < n; j++) {
    // Step 2
    data_type y = (x[j] & upper_mask) | (x[(j+1)%n] & lower_mask);
    // Step 3
    x[j] = x[(j+m)%n] ^ (y >> 1) ^ (y&1 ? a : 0);
  }
  */
  // split loop to avoid costly modulo operations
  {  // extra scope for MSVC brokenness w.r.t. for scope
  for(int j = 0; j < n-m; j++) {
    data_type y = (x[j] & upper_mask) | (x[j+1] & lower_mask);
    x[j] = x[j+m] ^ (y >> 1) ^ (y&1 ? a : 0);
  }
  }

  for(int j = n-m; j < n-1; j++) {
    data_type y = (x[j] & upper_mask) | (x[j+1] & lower_mask);
    x[j] = x[j+m-n] ^ (y >> 1) ^ (y&1 ? a : 0);
  }
  // last iteration
  data_type y = (x[n-1] & upper_mask) | (x[0] & lower_mask);
  x[n-1] = x[n-1+m-n] ^ (y >> 1) ^ (y&1 ? a : 0);
  
  i = 0;
}

template<class DataType, int n, int m, int r, DataType a, int u,
  int s, DataType b, int t, DataType c, int l, DataType val>
inline typename mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::result_type
mersenne_twister<DataType,n,m,r,a,u,s,b,t,c,l,val>::operator()()
{
  if(i >= n)
    twist();
  // Step 4
  data_type z = x[i];
  ++i;
  z ^= (z >> u);
  z ^= ((z << s) & b);
  z ^= ((z << t) & c);
  z ^= (z >> l);
  return z;
}

} // namespace random


typedef random::mersenne_twister<uint32_t,351,175,19,0xccab8ee7,11,
  7,0x31b6ab00,15,0xffe50000,17, /* unknown */ 0> mt11213b;

// validation by experiment from mt19937.c
typedef random::mersenne_twister<uint32_t,624,397,31,0x9908b0df,11,
  7,0x9d2c5680,15,0xefc60000,18, 3346425566U> mt19937;

} // namespace boost

#endif // BOOST_RANDOM_MERSENNE_TWISTER_HPP
