#ifndef BOOST_PREPROCESSOR_LIST_FOR_EACH_I_HPP
#define BOOST_PREPROCESSOR_LIST_FOR_EACH_I_HPP

/* Copyright (C) 2002
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

#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/for.hpp>

/** <p>Repeats <code>MACRO(R,DATA,INDEX,BOOST_PP_LIST_AT(LIST,INDEX))</code> for each INDEX = [0,
BOOST_PP_LIST_SIZE(LIST)).</p>

<p>In other words, expands to the sequence:</p>

<pre>
  MACRO(R,DATA,0,BOOST_PP_LIST_AT(LIST,0))
  MACRO(R,DATA,1,BOOST_PP_LIST_AT(LIST,1))
  ...
  MACRO(R,DATA,BOOST_PP_DEC(BOOST_PP_LIST_SIZE(LIST)),BOOST_PP_LIST_AT(LIST,BOOST_PP_DEC(BOOST_PP_LIST_SIZE(LIST))))
</pre>

<p>For example,</p>

<pre>
  #define TEST(R,DATA,INDEX,X) BOOST_PP_CAT(DATA,X)(INDEX);
  BOOST_PP_LIST_FOR_EACH_I(TEST,prefix_,BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)))
</pre>

<p>expands to:</p>

<pre>
  prefix_A(0); prefix_B(1); prefix_C(2);
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_FOR() (see for explanation of the R parameter)</li>
</ul>
*/
#define BOOST_PP_LIST_FOR_EACH_I(MACRO,DATA,LIST) BOOST_PP_LIST_FOR_EACH_I_R(0,MACRO,DATA,LIST)

/** <p>Can be used inside BOOST_PP_FOR().</p> */
#define BOOST_PP_LIST_FOR_EACH_I_R(R,MACRO,DATA,LIST) BOOST_PP_FOR##R((MACRO,DATA,LIST,0),BOOST_PP_LIST_FOR_EACH_I_C,BOOST_PP_LIST_FOR_EACH_I_F,BOOST_PP_LIST_FOR_EACH_I_I)
#define BOOST_PP_LIST_FOR_EACH_I_C(R,FPLI) BOOST_PP_TUPLE_ELEM(3,2,BOOST_PP_TUPLE_ELEM(4,2,FPLI))
#define BOOST_PP_LIST_FOR_EACH_I_F(R,FPLI) (BOOST_PP_TUPLE_ELEM(4,0,FPLI),BOOST_PP_TUPLE_ELEM(4,1,FPLI),BOOST_PP_TUPLE_ELEM(3,1,BOOST_PP_TUPLE_ELEM(4,2,FPLI)),BOOST_PP_INC(BOOST_PP_TUPLE_ELEM(4,3,FPLI)))
#define BOOST_PP_LIST_FOR_EACH_I_I(R,FPLI) BOOST_PP_TUPLE_ELEM(4,0,FPLI)(R,BOOST_PP_TUPLE_ELEM(4,1,FPLI),BOOST_PP_TUPLE_ELEM(4,3,FPLI),BOOST_PP_TUPLE_ELEM(3,0,BOOST_PP_TUPLE_ELEM(4,2,FPLI)))
#endif
