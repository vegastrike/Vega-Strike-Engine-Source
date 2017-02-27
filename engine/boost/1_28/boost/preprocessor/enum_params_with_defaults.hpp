#ifndef BOOST_PREPROCESSOR_ENUM_PARAMS_WITH_DEFAULTS_HPP
#define BOOST_PREPROCESSOR_ENUM_PARAMS_WITH_DEFAULTS_HPP

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
#include <boost/preprocessor/enum.hpp>

/** <p>Generates a comma separated list of parameters with defaults.</p>

<p>In other words, expands to the sequence:</p>

<pre>
  BOOST_PP_CAT(PARAM,0) = BOOST_PP_CAT(DEFAULT,0),
  BOOST_PP_CAT(PARAM,1) = BOOST_PP_CAT(DEFAULT,1),
  ...,
  BOOST_PP_CAT(PARAM,BOOST_PP_DEC(COUNT)) = BOOST_PP_CAT(DEFAULT,BOOST_PP_DEC(COUNT))
</pre>

<p>For example,</p>

<pre>
  BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS(3,x,y)
</pre>

<p>expands to:</p>

<pre>
  x0 = y0, x1 = y1, x2 = y2
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_REPEAT()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/repeat_test.cpp">repeat_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS(COUNT,PARAM,DEFAULT) BOOST_PP_ENUM(COUNT,BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS_F,(PARAM,DEFAULT))

#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS_F(I,PD) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,0,PD),I)=BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,PD),I)
#else
#  define BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS_F(I,PD) BOOST_PP_CAT(BOOST_PP_TUPLE2_ELEM0 PD,I)=BOOST_PP_CAT(BOOST_PP_TUPLE2_ELEM1 PD,I)
#endif

/* <p>Obsolete. Use BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS().</p> */
#define BOOST_PREPROCESSOR_ENUM_PARAMS_WITH_DEFAULTS(N,P,D) BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS(N,P,D)
#endif
