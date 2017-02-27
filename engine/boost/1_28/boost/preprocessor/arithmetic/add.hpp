#ifndef BOOST_PREPROCESSOR_ARITHMETIC_ADD_HPP
#define BOOST_PREPROCESSOR_ARITHMETIC_ADD_HPP

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

#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/while.hpp>

/** <p>Expands to the sum of <code>X</code> and <code>Y</code>.</p>

<p>Both <code>X</code> and <code>Y</code> must expand to integer literals
in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_ADD(4,3)</code> expands to <code>7</code> (a
single token).</p>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_WHILE()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/arithmetic_test.cpp">arithmetic_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_ADD(X,Y) BOOST_PP_ADD_D(0,X,Y)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_ADD_D(D,X,Y) BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_WHILE##D(BOOST_PP_ADD_C,BOOST_PP_ADD_F,(X,Y)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_ADD_C(D,P) BOOST_PP_TUPLE_ELEM(2,1,P)
#  define BOOST_PP_ADD_F(D,P) (BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(2,0,P)),BOOST_PP_DEC(BOOST_PP_TUPLE_ELEM(2,1,P)))
#else
#  define BOOST_PP_ADD_C(D,P) BOOST_PP_TUPLE2_ELEM1 P
#  define BOOST_PP_ADD_F(D,P) (BOOST_PP_INC(BOOST_PP_TUPLE2_ELEM0 P),BOOST_PP_DEC(BOOST_PP_TUPLE2_ELEM1 P))
#endif

/* <p>Obsolete. Use BOOST_PP_ADD().</p> */
#define BOOST_PREPROCESSOR_ADD(X,Y) BOOST_PP_ADD(X,Y)
#endif
