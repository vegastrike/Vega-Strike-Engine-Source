#ifndef BOOST_PREPROCESSOR_IDENTITY_HPP
#define BOOST_PREPROCESSOR_IDENTITY_HPP

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

#include <boost/preprocessor/empty.hpp>

/** <p>Expands to <code>X</code> once invoked.</p>

<p>Designed to be used with BOOST_PP_IF(), when one of the clauses need to be
invoked.</p>

<p>For example,</p>

<pre>
  BOOST_PP_IF(1,BOOST_PP_IDENTITY(X),BOOST_PP_EMPTY)<b>()</b>
</pre>

<p>expands to:</p>

<pre>
  X
</pre>

<h3>Test</h3>
<ul>
  <li><a href="../../test/preprocessor_test.cpp">preprocessor_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_IDENTITY(X) X BOOST_PP_EMPTY

/* <p>Obsolete. Use BOOST_PP_IDENTITY().</p> */
#define BOOST_PREPROCESSOR_IDENTITY(X) BOOST_PP_IDENTITY(X)
#endif
