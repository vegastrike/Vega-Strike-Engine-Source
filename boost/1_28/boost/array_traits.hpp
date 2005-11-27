// -*-C++-*- array_traits.hpp
// <!!----------------------------------------------------------------------> 
// <!! Copyright (C) 1998 Dietmar Kuehl, Claas Solutions GmbH > 
// <!!> 
// <!! Permission to use, copy, modify, distribute and sell this > 
// <!! software for any purpose is hereby granted without fee, provided > 
// <!! that the above copyright notice appears in all copies and that > 
// <!! both that copyright notice and this permission notice appear in > 
// <!! supporting documentation. Dietmar Kuehl and Claas Solutions make no > 
// <!! representations about the suitability of this software for any > 
// <!! purpose. It is provided "as is" without express or implied warranty. > 
// <!!----------------------------------------------------------------------> 

// Author: Dietmar Kuehl dietmar.kuehl@claas-solutions.de 
// Title:  STL container support, including support for built-in arrays
// Version: $Id$

// Dec 4, 2000  Added some more typedefs to array_traits including
//              an iterator type to supersede iter_type. -J.Siek

// -------------------------------------------------------------------------- 

#if !defined(BOOST_ARRAY_TRAITS_HPP)
#define BOOST_ARRAY_TRAITS_HPP 1

// -------------------------------------------------------------------------- 

#include <cstddef>
#include <boost/config.hpp>

// -------------------------------------------------------------------------- 

namespace boost
{

  // --- a general version of container traits ------------------------------ 

  template <typename Cont>
    struct array_traits
    {
      typedef typename Cont::iterator  iterator;
      typedef iterator iter_type; // just for backward compatibility
      typedef typename Cont::size_type size_type;
      typedef typename Cont::value_type value_type;
      typedef typename Cont::reference reference;
      typedef typename Cont::pointer pointer;
      static iterator begin(Cont &cont) { return cont.begin(); }
      static iterator end(Cont &cont) { return cont.end(); }
      static size_type size(Cont &cont) { return cont.size(); }
    };

  // --- a version of container traits for constant constainer --------------

  template <typename Cont>
    struct array_traits<Cont const>
    {
      typedef typename Cont::const_iterator iterator;
      typedef iterator iter_type; // just for backward compatibility
      typedef typename Cont::size_type size_type;
      typedef typename Cont::value_type value_type;
      typedef typename Cont::const_reference reference;
      typedef typename Cont::const_pointer pointer;
      static iterator begin(Cont const &cont) { return cont.begin(); }
      static iterator end(Cont const &cont) { return cont.end(); }
      static size_type size(Cont const &cont) { return cont.size(); }
    };

  // --- a special version for non-const built-in arrays -------------------- 

  template <typename T, std::size_t sz>
    struct array_traits<T[sz]>
    {
      typedef T* iterator;
      typedef iterator iter_type; // just for backward compatibility
      typedef T value_type;
      typedef value_type& reference;
      typedef std::size_t size_type;
      static iterator begin(T (&array)[sz]) { return array; }
      static iterator end(T (&array)[sz]) { return array + sz; }
      static size_type size(T (&)[sz]) { return sz; }
    };

  // --- a special version for const built-in arrays ------------------------ 

  template <typename T, std::size_t sz>
    struct array_traits<T const[sz]>
    {
      typedef T const* iterator;
      typedef iterator iter_type; // just for backward compatibility
      typedef std::size_t size_type;
      typedef T const value_type;
      typedef value_type& reference;
      typedef value_type* pointer;
      static iterator begin(T const (&array)[sz]) { return array; }
      static iterator end(T const (&array)[sz]) { return array + sz; }
      static size_type size(T const (&)[sz]) { return sz; }
    };

  template <typename T, int sz>
  inline char (&sizer(T (&)[sz]))[sz];
  
  // --- general version of the global accessor functions --------------------- 

  template <typename Cont>
    inline typename array_traits<Cont>::iterator
    begin(Cont &cont) { return array_traits<Cont>::begin(cont); }

  template <typename Cont>
    inline typename array_traits<Cont>::iterator
    end(Cont &cont) { return array_traits<Cont>::end(cont); }

  template <typename Cont>
    inline typename array_traits<Cont>::size_type
    size(Cont &cont) { return array_traits<Cont>::size(cont); }

  // --- Actually the above should be sufficient but compilers seem -----------
  // --- to welcome some help. So here we go:

  template <typename T, std::size_t sz>
    inline typename array_traits<T[sz]>::iterator
    begin(T (&a)[sz]) { return array_traits<T[sz]>::begin(a); }
  
  template <typename T, std::size_t sz>
    inline typename array_traits<T[sz]>::iterator
    end(T (&a)[sz]) { return array_traits<T[sz]>::end(a); }
  
  template <typename T, std::size_t sz>
    inline typename array_traits<T[sz]>::size_type
    size(T (&a)[sz]) { return array_traits<T[sz]>::size(a); }
  
  // --- Apparently the compilers also need some specific help, ---------------

  // --- EDG-2.39 wants to pass around pointers in some contexts --------------
#ifdef __EDG__
  template <typename T>
    struct array_traits<T*>
    {
      typedef T*     iterator;
      typedef iterator iter_type; // just for backward compatibility
      typedef std::size_t size_type;
    };
#endif

  // --- egcs-1998-11-22 apparently likes an extra const version: -------------
#ifdef __GNUG__
  template <typename T, std::size_t sz>
    inline typename array_traits<T const[sz]>::iterator
    begin(T const(&a)[sz]) { return array_traits<T const[sz]>::begin(a); }
  
  template <typename T, std::size_t sz>
    inline typename array_traits<T const[sz]>::iterator
    end(T const(&a)[sz]) { return array_traits<T const[sz]>::end(a); }
  
  template <typename T, std::size_t sz>
    inline typename array_traits<T const[sz]>::size_type
    size(T const (&a)[sz]) { return array_traits<T const[sz]>::size(a); }
#endif
  
}

// -----------------------------------------------------------------------------

#endif /* BOOST_ARRAY_TRAITS_HPP */
