#ifndef BOOST_PREPROCESSOR_LIST_SIZE_HPP
#define BOOST_PREPROCESSOR_LIST_SIZE_HPP

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

#include <boost/preprocessor/list/fold_left.hpp>
#include <boost/preprocessor/inc.hpp>

/** <p>Expands to the number of elements in the list.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_SIZE(BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)))
</pre>

<p>expands to <code>3</code>.</p>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_LIST_FOLD_LEFT()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_SIZE(LIST) BOOST_PP_LIST_SIZE_D(0,LIST)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_SIZE_D(D,LIST) BOOST_PP_LIST_FOLD_LEFT_D(D,BOOST_PP_LIST_SIZE_F,0,LIST)
#define BOOST_PP_LIST_SIZE_F(D,P,H) BOOST_PP_INC(P)
#endif
