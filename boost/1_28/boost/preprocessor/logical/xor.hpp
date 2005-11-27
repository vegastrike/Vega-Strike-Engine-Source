#ifndef BOOST_PREPROCESSOR_LOGICAL_XOR_HPP
#define BOOST_PREPROCESSOR_LOGICAL_XOR_HPP

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

#include <boost/preprocessor/logical/and.hpp>

/** <p>Expands to the logical EXCLUSIVE OR of the operands.</p>

<p>Both <code>X</code> and <code>Y</code> must expand to integer literals
in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_XOR(1,2)</code> expands to <code>0</code> (a single token).</p>

<h3>Test</h3>
<ul>
  <li><a href="../../test/logical_test.cpp">logical_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_XOR(X,Y) BOOST_PP_NOR(BOOST_PP_NOR(X,Y),BOOST_PP_AND(X,Y))

/* <p>Obsolete. Use BOOST_PP_XOR().</p> */
#define BOOST_PREPROCESSOR_XOR(X,Y) BOOST_PP_XOR(X,Y)
#endif
