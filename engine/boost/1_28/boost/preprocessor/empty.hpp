#ifndef BOOST_PREPROCESSOR_EMPTY_HPP
#define BOOST_PREPROCESSOR_EMPTY_HPP

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

/** <p>Expands to nothing.</p>

<p>For example,</p>

<pre>
  BOOST_PP_IF(0,BOOST_PP_COMMA,BOOST_PP_EMPTY)()
</pre>

<p>expands to nothing.</p>

<h3>Example</h3>
<ul>
  <li><a href="../../example/subscript_layer.cpp">subscript_layer.cpp</a></li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/preprocessor_test.cpp">preprocessor_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_EMPTY()

/* <p>Obsolete. Use BOOST_PP_EMPTY().</p> */
#define BOOST_PREPROCESSOR_EMPTY()
#endif
