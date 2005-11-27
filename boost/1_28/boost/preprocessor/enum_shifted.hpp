#ifndef BOOST_PREPROCESSOR_ENUM_SHIFTED_HPP
#define BOOST_PREPROCESSOR_ENUM_SHIFTED_HPP

/* Copyright (C) 2001
 * Housemarque Oy
 * http://www.housemarque.com
 *
 * Permission to copy, use, modify, sell and distribute this software is
 * granted provided this copyright notice appears in all copies. This
 * software is provided "as is" without express or implied warranty, and
 * with no claim as to its suitability for any purpose.
 *
 * See http://www.boost.org for most recent version.
 */

#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/inc.hpp>

/** <p>Generates a comma separated shifted list.</p>

<p>In other words, expands to the sequence:</p>

<pre>
  MACRO(1,DATA), MACRO(2,DATA), ..., MACRO(BOOST_PP_DEC(COUNT),DATA)
</pre>

<p>For example,</p>

<pre>
  #define TYPED_PARAM(INDEX,DATA)\
    BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,0,DATA),INDEX) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,DATA),INDEX)
  BOOST_PP_ENUM_SHIFTED(3,TYPED_PARAM,(X,x))
</pre>

<p>expands to:</p>

<pre>
  X1 x1, X2 x2
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_REPEAT()</li>
</ul>
*/
#define BOOST_PP_ENUM_SHIFTED(COUNT,MACRO,DATA) BOOST_PP_ENUM(BOOST_PP_DEC(COUNT),BOOST_PP_ENUM_SHIFTED_F,(MACRO,DATA))

#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_ENUM_SHIFTED_F(I,FP) BOOST_PP_TUPLE_ELEM(2,0,FP)(BOOST_PP_INC(I),BOOST_PP_TUPLE_ELEM(2,1,FP))
#else
#  define BOOST_PP_ENUM_SHIFTED_F(I,FP) BOOST_PP_TUPLE2_ELEM0 FP(BOOST_PP_INC(I),BOOST_PP_TUPLE2_ELEM1 FP)
#endif
#endif
