#ifndef BOOST_PREPROCESSOR_LIST_REST_N_HPP
#define BOOST_PREPROCESSOR_LIST_REST_N_HPP

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

#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/while.hpp>

/** <p>Expands to a list of all but the first <code>COUNT</code> elements of the
list <code>LIST</code>.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_REST_N(2,BOOST_PP_TUPLE_TO_LIST(4,(+,-,*,/)))
</pre>

<p>expands to the same as:</p>

<pre>
  BOOST_PP_TUPLE_TO_LIST(2,(*,/))
</pre>

<h3>See</h3>
<ul>
  <li>BOOST_PP_LIST_FIRST_N()</li>
</ul>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_WHILE()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_REST_N(COUNT,LIST) BOOST_PP_LIST_REST_N_D(0,COUNT,LIST)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_REST_N_D(D,COUNT,LIST) BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_WHILE##D(BOOST_PP_LIST_REST_N_C,BOOST_PP_LIST_REST_N_F,(LIST,COUNT)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_REST_N_C(D,X) BOOST_PP_TUPLE_ELEM(2,1,X)
#  define BOOST_PP_LIST_REST_N_F(D,X) (BOOST_PP_TUPLE_ELEM(3,1,BOOST_PP_TUPLE_ELEM(2,0,X)),BOOST_PP_DEC(BOOST_PP_TUPLE_ELEM(2,1,X)))
#else
#  define BOOST_PP_LIST_REST_N_C(D,X) BOOST_PP_TUPLE2_ELEM1 X
#  define BOOST_PP_LIST_REST_N_F(D,X) (BOOST_PP_TUPLE3_ELEM1 BOOST_PP_TUPLE2_ELEM0 X,BOOST_PP_DEC(BOOST_PP_TUPLE2_ELEM1 X))
#endif
#endif
