#ifndef BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_2ND_HPP
#define BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_2ND_HPP

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

#include <boost/preprocessor/list/fold_left_2nd.hpp>
#include <boost/preprocessor/list/reverse.hpp>

/** <p>Same as BOOST_PP_LIST_FOLD_RIGHT(), but implemented independently.</p> */
#define BOOST_PP_LIST_FOLD_RIGHT_2ND(OP,LIST,STATE) BOOST_PP_LIST_FOLD_RIGHT_2ND_D(0,OP,LIST,STATE)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_FOLD_RIGHT_2ND_D(D,OP,LIST,STATE) BOOST_PP_TUPLE_ELEM(2,1,BOOST_PP_LIST_FOLD_LEFT_2ND_D(D,BOOST_PP_LIST_FOLD_RIGHT_2ND_F,(OP,STATE),BOOST_PP_LIST_FOLD_LEFT_2ND_D(D,BOOST_PP_LIST_REVERSE_F,(_,_,0),LIST)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_FOLD_RIGHT_2ND_F(D,P,H) (BOOST_PP_TUPLE_ELEM(2,0,P),BOOST_PP_TUPLE_ELEM(2,0,P)(D,H,BOOST_PP_TUPLE_ELEM(2,1,P)))
#elif !defined(BOOST_NO_COMPILER_CONFIG) && defined(_MSC_VER)
#  define BOOST_PP_LIST_FOLD_RIGHT_2ND_F(D,P,H) (BOOST_PP_TUPLE_ELEM(2,0,P),BOOST_PP_TUPLE_ELEM(2,0,P)(D,H,BOOST_PP_TUPLE2_ELEM1 P))
#else
#  define BOOST_PP_LIST_FOLD_RIGHT_2ND_F(D,P,H) (BOOST_PP_TUPLE2_ELEM0 P,BOOST_PP_TUPLE2_ELEM0 P(D,H,BOOST_PP_TUPLE2_ELEM1 P))
#endif
#endif
