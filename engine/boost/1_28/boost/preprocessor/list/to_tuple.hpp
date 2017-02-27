#ifndef BOOST_PREPROCESSOR_LIST_TO_TUPLE_HPP
#define BOOST_PREPROCESSOR_LIST_TO_TUPLE_HPP

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

#include <boost/preprocessor/list/enum.hpp>

/** <p>Converts the list to a tuple.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_TO_TUPLE(BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)))
</pre>

<p>expands to <code>(A,B,C)</code>.</p>

<h3>Note</h3>
<ul>
  <li>The supported size of the list being converted to a tuple is limited by
      BOOST_PP_LIMIT_MAG rather than BOOST_PP_LIMIT_TUPLE.</li>
</ul>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_FOR()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_TO_TUPLE(LIST) BOOST_PP_LIST_TO_TUPLE_R(0,LIST)

/** <p>Can be used inside BOOST_PP_FOR().</p> */
#define BOOST_PP_LIST_TO_TUPLE_R(R,LIST) (BOOST_PP_LIST_ENUM_R(R,LIST))
#endif
