#ifndef BOOST_PREPROCESSOR_TUPLE_EAT_HPP
#define BOOST_PREPROCESSOR_TUPLE_EAT_HPP

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

/** <p>Expands to a macro that eats a tuple of the specified size.</p>

<p>BOOST_PP_TUPLE_EAT() is designed to be used with BOOST_PP_IF() like
BOOST_PP_EMPTY().</p>

<p>For example,</p>

<pre>
  BOOST_PP_IF(0,BOOST_PP_ENUM_PARAMS,BOOST_PP_TUPLE_EAT(2))(10,P)
</pre>

<p>expands to nothing.</p>
*/
#define BOOST_PP_TUPLE_EAT(SIZE_OF_TUPLE) BOOST_PP_TUPLE_EAT_DELAY(SIZE_OF_TUPLE)

#define BOOST_PP_TUPLE_EAT_DELAY(N) BOOST_PP_TUPLE##N##_EAT
#define BOOST_PP_TUPLE0_EAT()
#define BOOST_PP_TUPLE1_EAT(A)
#define BOOST_PP_TUPLE2_EAT(A,B)
#define BOOST_PP_TUPLE3_EAT(A,B,C)
#define BOOST_PP_TUPLE4_EAT(A,B,C,D)
#define BOOST_PP_TUPLE5_EAT(A,B,C,D,E)
#define BOOST_PP_TUPLE6_EAT(A,B,C,D,E,F)
#define BOOST_PP_TUPLE7_EAT(A,B,C,D,E,F,G)
#define BOOST_PP_TUPLE8_EAT(A,B,C,D,E,F,G,H)
#define BOOST_PP_TUPLE9_EAT(A,B,C,D,E,F,G,H,I)
#define BOOST_PP_TUPLE10_EAT(A,B,C,D,E,F,G,H,I,J)
#define BOOST_PP_TUPLE11_EAT(A,B,C,D,E,F,G,H,I,J,K)
#define BOOST_PP_TUPLE12_EAT(A,B,C,D,E,F,G,H,I,J,K,L)
#define BOOST_PP_TUPLE13_EAT(A,B,C,D,E,F,G,H,I,J,K,L,M)
#define BOOST_PP_TUPLE14_EAT(A,B,C,D,E,F,G,H,I,J,K,L,M,N)
#define BOOST_PP_TUPLE15_EAT(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O)
#define BOOST_PP_TUPLE16_EAT(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P)
#endif
