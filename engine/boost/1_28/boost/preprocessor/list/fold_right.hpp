#ifndef BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
#define BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP

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

#include <boost/preprocessor/list/reverse.hpp>

/** <p>Iterates <code>OP(D,X,STATE)</code> for each element <code>X</code> of the
list <code>LIST</code> (from the right or the end of the list).</p>

<p>In other words, expands to:</p>

<pre>
  OP
  ( D
  , BOOST_PP_LIST_AT(LIST,0)
  , ... OP
        ( D
        , BOOST_PP_LIST_AT(LIST,BOOST_PP_SUB(BOOST_PP_LIST_SIZE(LIST),2))
        , OP
          ( D
          , BOOST_PP_LIST_AT(LIST,BOOST_PP_SUB(BOOST_PP_LIST_SIZE(LIST),1))
          , STATE
          )
        ) ...
  )
</pre>

<p>For example,</p>

<pre>
  #define TEST(D,X,STATE) BOOST_PP_CAT(STATE,X)
  BOOST_PP_LIST_FOLD_RIGHT(TEST,_,BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)))
</pre>

<p>expands to:</p>

<pre>
  _CBA
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_WHILE() (see for explanation of the D parameter)</li>
  <li>BOOST_PP_LIST_FOLD_LEFT()</li>
</ul>

<h3>Test</h3>
<ul>
  <li><a href="../../test/list_test.cpp">list_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_LIST_FOLD_RIGHT(OP,LIST,STATE) BOOST_PP_LIST_FOLD_RIGHT_D(0,OP,LIST,STATE)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_FOLD_RIGHT_D(D,OP,LIST,STATE) BOOST_PP_TUPLE_ELEM(2,1,BOOST_PP_LIST_FOLD_LEFT_D(D,BOOST_PP_LIST_FOLD_RIGHT_F,(OP,STATE),BOOST_PP_LIST_REVERSE_D(D,LIST)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_FOLD_RIGHT_F(D,P,H) (BOOST_PP_TUPLE_ELEM(2,0,P),BOOST_PP_TUPLE_ELEM(2,0,P)(D,H,BOOST_PP_TUPLE_ELEM(2,1,P)))
#elif !defined(BOOST_NO_COMPILER_CONFIG) && defined(_MSC_VER)
#  define BOOST_PP_LIST_FOLD_RIGHT_F(D,P,H) (BOOST_PP_TUPLE_ELEM(2,0,P),BOOST_PP_TUPLE_ELEM(2,0,P)(D,H,BOOST_PP_TUPLE2_ELEM1 P))
#else
#  define BOOST_PP_LIST_FOLD_RIGHT_F(D,P,H) (BOOST_PP_TUPLE2_ELEM0 P,BOOST_PP_TUPLE2_ELEM0 P(D,H,BOOST_PP_TUPLE2_ELEM1 P))
#endif
#endif
