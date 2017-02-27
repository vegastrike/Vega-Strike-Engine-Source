#ifndef BOOST_PREPROCESSOR_ARITHMETIC_MOD_HPP
#define BOOST_PREPROCESSOR_ARITHMETIC_MOD_HPP

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

#include <boost/preprocessor/comparison/less_equal.hpp>

/** <p>Expands to the remainder of <code>X</code> and <code>Y</code>.</p>

<p>Both <code>X</code> and <code>Y</code> must expand to integer literals
in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_MOD(4,3)</code> expands to <code>1</code> (a
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
#define BOOST_PP_MOD(X,Y) BOOST_PP_MOD_D(0,X,Y)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_MOD_D(D,X,Y) BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_WHILE##D(BOOST_PP_MOD_C,BOOST_PP_MOD_F,(X,Y)))
#define BOOST_PP_MOD_C(D,P) BOOST_PP_LESS_EQUAL_D(D,BOOST_PP_TUPLE2_ELEM1 P,BOOST_PP_TUPLE2_ELEM0 P)
#define BOOST_PP_MOD_F(D,P) (BOOST_PP_SUB_D(D,BOOST_PP_TUPLE2_ELEM0 P,BOOST_PP_TUPLE2_ELEM1 P),BOOST_PP_TUPLE2_ELEM1 P)

/* <p>Obsolete. Use BOOST_PP_MOD().</p> */
#define BOOST_PREPROCESSOR_MOD(X,Y) BOOST_PP_MOD(X,Y)
#endif
