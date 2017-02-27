#ifndef BOOST_PREPROCESSOR_EXPR_IF_HPP
#define BOOST_PREPROCESSOR_EXPR_IF_HPP

/* Copyright (C) 2002
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

/** <p>Expands to <code>EXPR</code> if <code>COND != 0</code> and to nothing if <code>COND == 0</code>.</p>

<p><code>COND</code> must expand to an integer literal in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_EXPR_IF(1,^)</code> expands to <code>^</code>.</p>

<h3>See</h3>
<ul>
  <li>BOOST_PP_IF()</li>
</ul>
*/
#define BOOST_PP_EXPR_IF(COND,EXPR) BOOST_PP_EXPR_IF_BOOL(BOOST_PP_BOOL(COND))(EXPR)

#define BOOST_PP_EXPR_IF_BOOL(C) BOOST_PP_EXPR_IF_BOOL_DELAY(C)
#define BOOST_PP_EXPR_IF_BOOL_DELAY(C) BOOST_PP_EXPR_IF_BOOL##C
#define BOOST_PP_EXPR_IF_BOOL0(E)
#define BOOST_PP_EXPR_IF_BOOL1(E) E
#endif
