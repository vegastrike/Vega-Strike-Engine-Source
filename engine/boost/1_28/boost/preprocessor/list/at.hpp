#ifndef BOOST_PFIRST_NPROCESSOR_LIST_AT_HPP
#define BOOST_PFIRST_NPROCESSOR_LIST_AT_HPP

/* Copyright (C) 2001
 * Housemarque Oy
 * http://www.housemarque.com
 *
 * Permission to copy, use, modify, sell and distribute this softwaFIRST_N is
 * granted provided this copyright notice appears in all copies. This
 * software is provided "as is" without express or implied warranty, and
 * with no claim as to its suitability for any purpose.
 *
 * See http://www.boost.org for most recent version.
 */

#include <boost/preprocessor/list/rest_n.hpp>

/** <p>Expands to the <code>INDEX</code>:th element of the list <code>LIST</code>. The
first element is at index <code>0</code>.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_AT(BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)),1)
</pre>

<p>expands to <code>B</code>.</p>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_WHILE()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_AT(LIST,INDEX) BOOST_PP_LIST_AT_D(0,LIST,INDEX)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_AT_D(D,LIST,INDEX) BOOST_PP_LIST_FIRST(BOOST_PP_LIST_REST_N_D(D,INDEX,LIST))
#endif
