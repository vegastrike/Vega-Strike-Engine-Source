#ifndef BOOST_PREPROCESSOR_IF_HPP
#define BOOST_PREPROCESSOR_IF_HPP

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

#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

/** <p>Expands to <code>THEN</code> if <code>COND != 0</code> and <code>ELSE</code> if
<code>COND == 0</code>.</p>

<p><code>COND</code> must expand to an integer literal in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_IF(0,1,2)</code> expands to <code>2</code>.</p>

<h3>See</h3>
<ul>
  <li>BOOST_PP_EXPR_IF()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/preprocessor_test.cpp">preprocessor_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_IF(COND,THEN,ELSE) BOOST_PP_IF_BOOL(BOOST_PP_BOOL(COND))(ELSE,THEN)

#define BOOST_PP_IF_BOOL(C) BOOST_PP_IF_BOOL_DELAY(C)
#define BOOST_PP_IF_BOOL_DELAY(C) BOOST_PP_TUPLE2_ELEM##C

/* <p>Obsolete. Use BOOST_PP_IF().</p> */
#define BOOST_PREPROCESSOR_IF(C,T,E) BOOST_PP_IF(C,T,E)
#endif
