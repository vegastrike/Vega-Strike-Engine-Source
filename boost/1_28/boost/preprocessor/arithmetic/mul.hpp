#ifndef BOOST_PREPROCESSOR_ARITHMETIC_MUL_HPP
#define BOOST_PREPROCESSOR_ARITHMETIC_MUL_HPP

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

#include <boost/preprocessor/arithmetic/add.hpp>

/** <p>Expands to the product of <code>X</code> and <code>Y</code>.</p>

<p>Both <code>X</code> and <code>Y</code> must expand to integer literals
in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_MUL(4,3)</code> expands to <code>12</code> (a
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
#define BOOST_PP_MUL(X,Y) BOOST_PP_MUL_D(0,X,Y)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_MUL_D(D,X,Y) BOOST_PP_TUPLE_ELEM(3,0,BOOST_PP_WHILE##D(BOOST_PP_MUL_C,BOOST_PP_MUL_F,(0,X,Y)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_MUL_C(D,P) BOOST_PP_TUPLE_ELEM(3,2,P)
#  define BOOST_PP_MUL_F(D,P) (BOOST_PP_ADD_D(D,BOOST_PP_TUPLE_ELEM(3,0,P),BOOST_PP_TUPLE_ELEM(3,1,P)),BOOST_PP_TUPLE_ELEM(3,1,P),BOOST_PP_DEC(BOOST_PP_TUPLE_ELEM(3,2,P)))
#else
#  define BOOST_PP_MUL_C(D,P) BOOST_PP_TUPLE3_ELEM2 P
#  define BOOST_PP_MUL_F(D,P) (BOOST_PP_ADD_D(D,BOOST_PP_TUPLE3_ELEM0 P,BOOST_PP_TUPLE3_ELEM1 P),BOOST_PP_TUPLE3_ELEM1 P,BOOST_PP_DEC(BOOST_PP_TUPLE3_ELEM2 P))
#endif

/* <p>Obsolete. Use BOOST_PP_MUL().</p> */
#define BOOST_PREPROCESSOR_MUL(X,Y) BOOST_PP_MUL(X,Y)
#endif
