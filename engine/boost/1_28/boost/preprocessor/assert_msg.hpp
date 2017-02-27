#ifndef BOOST_PREPROCESSOR_ASSERT_MSG_HPP
#define BOOST_PREPROCESSOR_ASSERT_MSG_HPP

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

#include <boost/preprocessor/expr_if.hpp>
#include <boost/preprocessor/logical/not.hpp>

/** <p>Expands to nothing if <code>COND != 0</code> and to <code>MSG</code> if
<code>COND == 0</code>.</p>

<p><code>COND</code> must expand to an integer literal in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_ASSERT_MSG(1,A BUG!)</code> expands to <code>A BUG!</code>.</p>
*/
#define BOOST_PP_ASSERT_MSG(COND,MSG) BOOST_PP_EXPR_IF(BOOST_PP_NOT(COND),MSG)

/* <p>Obsolete. Use BOOST_PP_ASSERT_MSG().</p> */
#define BOOST_PREPROCESSOR_ASSERT_MSG(C,MSG) BOOST_PP_ASSERT_MSG(C,MSG)
#endif
