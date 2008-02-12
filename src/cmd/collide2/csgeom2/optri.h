  /*
    Copyright (C) 2003 by Jorrit Tyberghein

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_TRIANGLE_H__
#define __CS_TRIANGLE_H__

/**\file 
 * Triangle.
 */
/**
 * \addtogroup geom_utils
 * @{ */

#include "opcodeextern.h"

namespace CS
{
  /**
   * A templated triangle. Note that this structure is only sensible if used
   * in combination with a vertex or edge table. 'a', 'b', and 'c' are then
   * indices in that table (either vertices or edges).
   */
  template<typename T>
  struct TriangleT
  {
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
    union
    {
      struct 
      {
#endif
        //@{
        /// Triangle vertices or edges.
        T a, b, c;
        //@}
#if !defined(__STRICT_ANSI__) && !defined(SWIG)
      };
      /// All components
      T components[3];
    };
#endif

    /// Empty default constructor
    TriangleT () {}

    /// Convenience constructor, builds a triangle with initializers
    TriangleT (const T& _a, const T& _b, const T& _c) : a(_a), b(_b), c(_c) {}

    /// Copy constructor.
    TriangleT (const TriangleT& t)
    {
      a = t.a;
      b = t.b;
      c = t.c;
    }

    /// Assignment.
    TriangleT& operator= (const TriangleT& t)
    {
      a = t.a;
      b = t.b;
      c = t.c;
      return *this;
    }

    /// Set the values.
    void Set (const T& _a, const T& _b, const T& _c)
    {
      a = _a;
      b = _b;
      c = _c;
    }

    /// Returns n-th component of the triangle.
#ifdef __STRICT_ANSI__
    inline const T& operator[] (size_t n) const { return !n?a:n&1?b:c; }
#else
    inline const T& operator[] (size_t n) const { return components[n]; }
#endif

    /// Returns n-th component of the triangle.
#ifdef __STRICT_ANSI__
    inline T& operator[] (size_t n) { return !n?a:n&1?b:c; }
#else
    inline T& operator[] (size_t n) { return components[n]; }
#endif

  };
}

/**
 * A triangle. Note that this structure is only valid if used
 * in combination with a vertex or edge table. 'a', 'b', and 'c' are then
 * indices in that table (either vertices or edges).
 */
struct csTriangle : public CS::TriangleT<int>
{
  csTriangle () {}

  /// Convenience constructor, builds a triangle with initializers
  csTriangle (int _a, int _b, int _c) : CS::TriangleT<int> (_a, _b, _c) {}

  /// Copy constructor.
  csTriangle (const CS::TriangleT<int>& t) : CS::TriangleT<int> (t) {}
};

/** @} */

#endif // __CS_TRIANGLE_H__

