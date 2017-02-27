#ifndef BOOST_PREPROCESSOR_STRINGIZE_HPP
#define BOOST_PREPROCESSOR_STRINGIZE_HPP

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

/** <p>Stringizes <code>X</code> after it is macro expanded.</p>

<p>For example, <code>BOOST_PP_STRINGIZE(BOOST_PP_CAT(a,b))</code> expands to <code>"ab"</code>.</p>

<h3>Example</h3>
<ul>
  <li><a href="../../example/note.c">note.c</a></li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/preprocessor_test.cpp">preprocessor_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_STRINGIZE(X) BOOST_PP_STRINGIZE_DELAY(X)

#define BOOST_PP_STRINGIZE_DELAY(X) BOOST_PP_DO_STRINGIZE(X)
#define BOOST_PP_DO_STRINGIZE(X) #X

/* <p>Obsolete. Use BOOST_PP_STRINGIZE().</p> */
#define BOOST_PREPROCESSOR_STRINGIZE(E) BOOST_PP_STRINGIZE(E)
#endif
