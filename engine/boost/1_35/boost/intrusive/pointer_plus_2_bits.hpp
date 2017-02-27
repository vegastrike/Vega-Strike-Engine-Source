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

#ifndef BOOST_INTRUSIVE_POINTER_PLUS_2_BIT_HPP
#define BOOST_INTRUSIVE_POINTER_PLUS_2_BIT_HPP

namespace boost {
namespace intrusive {

//!This trait class is used to know if a pointer
//!can embed 2 extra bits of information if
//!it's going to be used to point to objects
//!with an alignment of "Alignment" bytes.
template<class VoidPointer, std::size_t Alignment>
struct has_pointer_plus_2_bits
{
   static const bool value = false;
};

//!This is an specialization for raw pointers.
//!Raw pointers can embed two extra bits in the lower bits
//!if the alignment is multiple of 4.
template<std::size_t N>
struct has_pointer_plus_2_bits<void*, N>
{
   static const bool value = (N % 4u == 0);
};

//!This is class that is supposed to have static methods
//!to embed 2 extra bits of information in a pointer.
//!
//!This is a declaration and there is no default implementation,
//!because operations to embed bits change with every pointer type.
//!
//!An implementation that detects that a pointer type whose
//!has_pointer_plus_2_bits<>::value is non-zero can make use of these
//!operations to embed bits in the pointer.
template<class Pointer>
struct pointer_plus_2_bits
{
   static const bool value = false;
};

//!This is the specialization to embed 2 extra bits of information
//!in a raw pointer. Extra bits are stored in the lower bits of the pointer.
template<class T>
struct pointer_plus_2_bits<T*>
{
   typedef T*        pointer;

   static pointer get_pointer(pointer n)
   {  return pointer(std::size_t(n) & ~std::size_t(3u));  }

   static void set_pointer(pointer &n, pointer p)
   {
      assert(0 == (std::size_t(p) & std::size_t(3u)));
      n = pointer(std::size_t(p) | (std::size_t(n) & std::size_t(3u))); 
   }

   static std::size_t get_bits(pointer n)
   {  return (std::size_t(n) & std::size_t(3u));  }

   static void set_bits(pointer &n, std::size_t c)
   {
      assert(c < 4);
      n = pointer(std::size_t(get_pointer(n)) | c);
   }
};

} //namespace intrusive 
} //namespace boost 

#endif //BOOST_INTRUSIVE_POINTER_PLUS_2_BIT_HPP
