#ifndef BOOST_PREPROCESSOR_CAT_HPP
#define BOOST_PREPROCESSOR_CAT_HPP

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

/** <p>Concatenates <code>X</code> and <code>Y</code> after they are macro
expanded.</p>

<p>For example, <code>BOOST_PP_CAT(A,BOOST_PP_CAT(_,B))</code> expands to <code>A_B</code>.</p>

<h3>Example</h3>
<ul>
  <li><a href="../../example/static_assert.c">static_assert.c</a></li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/preprocessor_test.cpp">preprocessor_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_CAT(X,Y) BOOST_PP_CAT_DELAY(X,Y)

#define BOOST_PP_CAT_DELAY(X,Y) BOOST_PP_DO_CAT(X,Y)
#define BOOST_PP_DO_CAT(X,Y) X##Y

/* <p>Obsolete. Use BOOST_PP_CAT().</p> */
#define BOOST_PREPROCESSOR_CAT(X,Y) BOOST_PP_CAT(X,Y)
#endif
