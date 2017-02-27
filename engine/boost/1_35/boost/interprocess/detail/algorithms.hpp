//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2008.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP
#define BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP

#if (defined _MSC_VER) && (_MSC_VER >= 1200)
#  pragma once
#endif

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/detail/iterators.hpp>
#include <boost/get_pointer.hpp>
#include <boost/detail/no_exceptions_support.hpp>

namespace boost {
namespace interprocess { 

template<class T>
struct has_own_construct_from_it
{
   static const bool value = false;
};

namespace detail  {

template<class T, class InpIt>
inline void construct_in_place_impl(T* dest, const InpIt &source, detail::true_)
{
   T::construct(dest, *source);
}

template<class T, class InpIt>
inline void construct_in_place_impl(T* dest, const InpIt &source, detail::false_)
{
   new(dest)T(*source);
}

}  //namespace detail   {

template<class T, class InpIt>
inline void construct_in_place(T* dest, InpIt source)
{
   typedef detail::bool_<has_own_construct_from_it<T>::value> boolean_t;
   detail::construct_in_place_impl(dest, source, boolean_t());
}

template<class T, class U, class D>
inline void construct_in_place(T *dest, default_construct_iterator<U, D>)
{
   new(dest)T();
}

template<class InIt, class OutIt>
InIt copy_n(InIt first, typename std::iterator_traits<InIt>::difference_type length, OutIt dest)
{
   for (; length--; ++dest, ++first)
      *dest = *first;
   return first;
}

template<class InIt, class FwdIt> inline
InIt n_uninitialized_copy_n
   (InIt first, 
    typename std::iterator_traits<InIt>::difference_type count,
    FwdIt dest)
{
   typedef typename std::iterator_traits<FwdIt>::value_type value_type;
   //Save initial destination position
   FwdIt dest_init = dest;
   typename std::iterator_traits<InIt>::difference_type new_count = count+1;

   BOOST_TRY{
      //Try to build objects
      for (; --new_count; ++dest, ++first){
         construct_in_place(detail::get_pointer(&*dest), first);
      }
   }
   BOOST_CATCH(...){
      //Call destructors
      new_count = count - new_count;
      for (; new_count--; ++dest_init){
         detail::get_pointer(&*dest_init)->~value_type();
      }
      BOOST_RETHROW
   }
   BOOST_CATCH_END
   return first;
}

// uninitialized_copy_copy
// Copies [first1, last1) into [result, result + (last1 - first1)), and
// copies [first2, last2) into
// [result + (last1 - first1), result + (last1 - first1) + (last2 - first2)).
template <class InpIt1, class InpIt2, class FwdIt>
FwdIt uninitialized_copy_copy
   (InpIt1 first1, InpIt1 last1, InpIt2 first2, InpIt2 last2, FwdIt result)
{
   typedef typename std::iterator_traits<FwdIt>::value_type value_type;
   FwdIt mid = std::uninitialized_copy(first1, last1, result);
   BOOST_TRY {
      return std::uninitialized_copy(first2, last2, mid);
   }
   BOOST_CATCH(...){
      for(;result != mid; ++result){
         result->~value_type();
      }
      BOOST_RETHROW
   }
   BOOST_CATCH_END
}

}  //namespace interprocess { 
}  //namespace boost {

#include <boost/interprocess/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTERPROCESS_DETAIL_ALGORITHMS_HPP

