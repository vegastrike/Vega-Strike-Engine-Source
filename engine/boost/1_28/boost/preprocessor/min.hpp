#ifndef BOOST_PREPROCESSOR_MIN_HPP
#define BOOST_PREPROCESSOR_MIN_HPP

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
#include <boost/preprocessor/if.hpp>

/** <p>Expands to the minimum of <code>X</code> and <code>Y</code>.</p>

<p>Both <code>X</code> and <code>Y</code> must expand to integer literals
in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_MIN(5,7)</code> expands to <code>5</code> (a
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
#define BOOST_PP_MIN(X,Y) BOOST_PP_MIN_D(0,X,Y)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_MIN_D(D,X,Y) BOOST_PP_IF(BOOST_PP_LESS_EQUAL_D(D,Y,X),Y,X)

/* <p>Obsolete. Use BOOST_PP_MIN().</p> */
#define BOOST_PREPROCESSOR_MIN(X,Y) BOOST_PP_MIN(X,Y)
#endif
