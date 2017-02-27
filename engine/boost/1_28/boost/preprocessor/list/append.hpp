#ifndef BOOST_PREPROCESSOR_LIST_APPEND_HPP
#define BOOST_PREPROCESSOR_LIST_APPEND_HPP

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

#include <boost/preprocessor/list/fold_right.hpp>

/** <p>Catenates two lists together.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_APPEND
  ( BOOST_PP_TUPLE_TO_LIST(2,(1,2))
  , BOOST_PP_TUPLE_TO_LIST(2,(3,4))
  )
</pre>

<p>expands to the same as:</p>

<pre>
  BOOST_PP_TUPLE_TO_LIST(4,(1,2,3,4))
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_LIST_FOLD_RIGHT()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_APPEND(LIST_1ST,LIST_2ND) BOOST_PP_LIST_APPEND_D(0,LIST_1ST,LIST_2ND)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_APPEND_D(D,LIST_1ST,LIST_2ND) BOOST_PP_LIST_FOLD_RIGHT_D(D,BOOST_PP_LIST_APPEND_F,LIST_1ST,LIST_2ND)
#define BOOST_PP_LIST_APPEND_F(D,H,P) (H,P,1)
#endif
