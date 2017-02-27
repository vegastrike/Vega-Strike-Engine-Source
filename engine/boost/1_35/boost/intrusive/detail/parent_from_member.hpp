/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_INTRUSIVE_PARENT_FROM_MEMBER_HPP
#define BOOST_INTRUSIVE_PARENT_FROM_MEMBER_HPP

#include <boost/intrusive/detail/config_begin.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>

namespace boost {
namespace intrusive {
namespace detail {

template<class Parent, class Member>
inline std::size_t offset_from_pointer_to_member(const Member Parent::* ptr_to_member)
{
   //BOOST_STATIC_ASSERT(( sizeof(std::ptrdiff_t) == sizeof(ptr_to_member) ));
   //The implementation of a pointer to member is compiler dependent.
   #if defined(BOOST_MSVC) || (defined (BOOST_WINDOWS) && defined(BOOST_INTEL))
   //This works with gcc, msvc, ac++, ibmcpp
   return *(const std::ptrdiff_t*)(void*)&ptr_to_member;
   #elif defined(__GNUC__) || defined(__HP_aCC) || defined(BOOST_INTEL) || defined (__IBMCPP__) || defined (__DECCXX)
   const Parent * const parent = 0;
   const char *const member = reinterpret_cast<const char*>(&(parent->*ptr_to_member));
   return std::size_t(member - reinterpret_cast<const char*>(parent));
   #else
   //This is the traditional C-front approach: __MWERKS__, __DMC__, __SUNPRO_CC
   return (*(const std::ptrdiff_t*)(void*)&ptr_to_member) - 1;
   #endif
}

template<class Parent, class Member>
inline Parent *parent_from_member(Member *member, const Member Parent::* ptr_to_member)
{
   return (Parent*)((char*)member - 
      offset_from_pointer_to_member(ptr_to_member));
}

template<class Parent, class Member>
inline const Parent *parent_from_member(const Member *member, const Member Parent::* ptr_to_member)
{
   return (const Parent*)((const char*)member - 
      offset_from_pointer_to_member(ptr_to_member));
}

}  //namespace detail {
}  //namespace intrusive {
}  //namespace boost {

#include <boost/intrusive/detail/config_end.hpp>

#endif   //#ifndef BOOST_INTRUSIVE_PARENT_FROM_MEMBER_HPP
