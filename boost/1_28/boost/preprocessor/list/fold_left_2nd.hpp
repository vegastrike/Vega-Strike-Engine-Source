#ifndef BOOST_PREPROCESSOR_LIST_FOLD_LEFT_2ND_HPP
#define BOOST_PREPROCESSOR_LIST_FOLD_LEFT_2ND_HPP

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
#include <boost/preprocessor/while.hpp>

/** <p>Same as BOOST_PP_LIST_FOLD_LEFT(), but implemented independently.</p> */
#define BOOST_PP_LIST_FOLD_LEFT_2ND(OP,STATE,LIST) BOOST_PP_LIST_FOLD_LEFT_2ND_D(0,OP,STATE,LIST)

/** <p>Can be used inside BOOST_PP_WHILE().</p> */
#define BOOST_PP_LIST_FOLD_LEFT_2ND_D(D,OP,STATE,LIST) BOOST_PP_TUPLE_ELEM(3,1,BOOST_PP_WHILE##D(BOOST_PP_LIST_FOLD_LEFT_2ND_C,BOOST_PP_LIST_FOLD_LEFT_2ND_F,(OP,STATE,LIST)))
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_C(D,X) BOOST_PP_TUPLE_ELEM(3,2,BOOST_PP_TUPLE_ELEM(3,2,X))
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_F(D,X) (BOOST_PP_TUPLE_ELEM(3,0,X),BOOST_PP_TUPLE_ELEM(3,0,X)(D,BOOST_PP_TUPLE_ELEM(3,1,X),BOOST_PP_TUPLE_ELEM(3,0,BOOST_PP_TUPLE_ELEM(3,2,X))),BOOST_PP_TUPLE_ELEM(3,1,BOOST_PP_TUPLE_ELEM(3,2,X)))
#elif !defined(BOOST_NO_COMPILER_CONFIG) && defined(_MSC_VER)
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_C(D,X) BOOST_PP_TUPLE3_ELEM2 BOOST_PP_TUPLE3_ELEM2 X
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_F(D,X) (BOOST_PP_TUPLE_ELEM(3,0,X),BOOST_PP_TUPLE3_ELEM0 X(D,BOOST_PP_TUPLE3_ELEM1 X,BOOST_PP_TUPLE3_ELEM0 BOOST_PP_TUPLE3_ELEM2 X),BOOST_PP_TUPLE3_ELEM1 BOOST_PP_TUPLE3_ELEM2 X)
#else
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_C(D,X) BOOST_PP_TUPLE3_ELEM2 BOOST_PP_TUPLE3_ELEM2 X
#  define BOOST_PP_LIST_FOLD_LEFT_2ND_F(D,X) (BOOST_PP_TUPLE3_ELEM0 X,BOOST_PP_TUPLE3_ELEM0 X(D,BOOST_PP_TUPLE3_ELEM1 X,BOOST_PP_TUPLE3_ELEM0 BOOST_PP_TUPLE3_ELEM2 X),BOOST_PP_TUPLE3_ELEM1 BOOST_PP_TUPLE3_ELEM2 X)
#endif
#endif
