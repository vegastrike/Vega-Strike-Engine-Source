#ifndef BOOST_PREPROCESSOR_ENUM_SHIFTED_PARAMS_HPP
#define BOOST_PREPROCESSOR_ENUM_SHIFTED_PARAMS_HPP

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
#include <boost/preprocessor/enum_shifted.hpp>

/** <p>Generates a comma separated list of shifted actual parameters.</p>

<p>In other words, expands to the sequence:</p>

<pre>
  BOOST_PP_CAT(PARAM,1), BOOST_PP_CAT(PARAM,2), ..., BOOST_PP_CAT(PARAM,BOOST_PP_DEC(COUNT))
</pre>
<p>For example,</p>

<pre>
  BOOST_PP_ENUM_SHIFTED_PARAMS(3,x)
</pre>

<p>expands to:</p>

<pre>
  x1, x2
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_REPEAT()</li>
</ul>

<h3>Rationale</h3>
<ul>
  <li>This macro facilitates a typical usage of the library. Shifted parameter
      lists are common in template metaprograms.</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/repeat_2nd_test.cpp">repeat_2nd_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_ENUM_SHIFTED_PARAMS(COUNT,PARAM) BOOST_PP_ENUM_SHIFTED(COUNT,BOOST_PP_ENUM_SHIFTED_PARAMS_F,PARAM)

#define BOOST_PP_ENUM_SHIFTED_PARAMS_F(I,P) BOOST_PP_CAT(P,I)

/* <p>Obsolete. Use BOOST_PP_ENUM_SHIFTED_PARAMS().</p> */
#define BOOST_PREPROCESSOR_ENUM_SHIFTED_PARAMS(N,P) BOOST_PP_ENUM_SHIFTED_PARAMS(N,P)
#endif
