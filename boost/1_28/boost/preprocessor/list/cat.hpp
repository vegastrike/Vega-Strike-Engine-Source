#ifndef BOOST_PREPROCESSOR_LIST_CAT_HPP
#define BOOST_PREPROCESSOR_LIST_CAT_HPP

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

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/list/fold_left.hpp>

/** <p>Catenates all elements of the list.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_CAT(BOOST_PP_TUPLE_TO_LIST(3,(1,2,3)))
</pre>

<p>expands to:</p>

<pre>
  123
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_LIST_FOLD_LEFT()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_CAT(LIST) BOOST_PP_LIST_CAT_D(0,LIST)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_CAT_D(D,LIST) BOOST_PP_LIST_FOLD_LEFT_D(D,BOOST_PP_LIST_CAT_F,BOOST_PP_TUPLE3_ELEM0 LIST,BOOST_PP_TUPLE3_ELEM1 LIST)
#define BOOST_PP_LIST_CAT_F(D,P,H) BOOST_PP_CAT(P,H)
#endif
