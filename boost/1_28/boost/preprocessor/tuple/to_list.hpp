#ifndef BOOST_PREPROCESSOR_TUPLE_TO_LIST_HPP
#define BOOST_PREPROCESSOR_TUPLE_TO_LIST_HPP

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

#include <boost/preprocessor/list/adt.hpp>

/** <p>Converts a tuple to a list.</p>

<p>For example,</p>

<pre>
  BOOST_PP_TUPLE_TO_LIST(3,(A,B,C))
</pre>

<p>expands to the same as</p>

<pre>
  BOOST_PP_LIST_CONS(A,
  BOOST_PP_LIST_CONS(B,
  BOOST_PP_LIST_CONS(C,
  BOOST_PP_LIST_NIL)))
</pre>

<h3>See</h3>
<ul>
  <li>BOOST_PP_LIMIT_TUPLE</li>
</ul>
*/
#define BOOST_PP_TUPLE_TO_LIST(SIZE_OF_TUPLE,TUPLE) BOOST_PP_TUPLE_TO_LIST_DELAY(SIZE_OF_TUPLE,TUPLE)

#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
/* This is a workaround for a CodeWarrior PP bug. Strictly speaking
 * this workaround invokes undefined behavior, but it works as desired.
 */
#  define BOOST_PP_TUPLE_TO_LIST_DELAY(N,T) BOOST_PP_TUPLE##N##_TO_LIST##T
#else
#  define BOOST_PP_TUPLE_TO_LIST_DELAY(N,T) BOOST_PP_TUPLE##N##_TO_LIST T
#endif
#define BOOST_PP_TUPLE0_TO_LIST() (_,_,0)
#define BOOST_PP_TUPLE1_TO_LIST(A) (A,(_,_,0),1)
#define BOOST_PP_TUPLE2_TO_LIST(A,B) (A,(B,(_,_,0),1),1)
#define BOOST_PP_TUPLE3_TO_LIST(A,B,C) (A,(B,(C,(_,_,0),1),1),1)
#define BOOST_PP_TUPLE4_TO_LIST(A,B,C,D) (A,(B,(C,(D,(_,_,0),1),1),1),1)
#define BOOST_PP_TUPLE5_TO_LIST(A,B,C,D,E) (A,(B,(C,(D,(E,(_,_,0),1),1),1),1),1)
#define BOOST_PP_TUPLE6_TO_LIST(A,B,C,D,E,F) (A,(B,(C,(D,(E,(F,(_,_,0),1),1),1),1),1),1)
#define BOOST_PP_TUPLE7_TO_LIST(A,B,C,D,E,F,G) (A,(B,(C,(D,(E,(F,(G,(_,_,0),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE8_TO_LIST(A,B,C,D,E,F,G,H) (A,(B,(C,(D,(E,(F,(G,(H,(_,_,0),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE9_TO_LIST(A,B,C,D,E,F,G,H,I) (A,(B,(C,(D,(E,(F,(G,(H,(I,(_,_,0),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE10_TO_LIST(A,B,C,D,E,F,G,H,I,J) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(_,_,0),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE11_TO_LIST(A,B,C,D,E,F,G,H,I,J,K) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(_,_,0),1),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE12_TO_LIST(A,B,C,D,E,F,G,H,I,J,K,L) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(L,(_,_,0),1),1),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE13_TO_LIST(A,B,C,D,E,F,G,H,I,J,K,L,M) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(L,(M,(_,_,0),1),1),1),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE14_TO_LIST(A,B,C,D,E,F,G,H,I,J,K,L,M,N) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(L,(M,(N,(_,_,0),1),1),1),1),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE15_TO_LIST(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(L,(M,(N,(O,(_,_,0),1),1),1),1),1),1),1),1),1),1),1),1),1),1),1)
#define BOOST_PP_TUPLE16_TO_LIST(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (A,(B,(C,(D,(E,(F,(G,(H,(I,(J,(K,(L,(M,(N,(O,(P,(_,_,0),1),1),1),1),1),1),1),1),1),1),1),1),1),1),1),1)
#endif
