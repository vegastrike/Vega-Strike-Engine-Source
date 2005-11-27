#ifndef BOOST_PREPROCESSOR_REPEAT_FROM_TO_2ND_HPP
#define BOOST_PREPROCESSOR_REPEAT_FROM_TO_2ND_HPP

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

#include <boost/preprocessor/repeat_2nd.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>

/** <p>Same as BOOST_PP_REPEAT_FROM_TO(), but implemented independently.</p> */
#define BOOST_PP_REPEAT_FROM_TO_2ND(FIRST,LAST,MACRO,DATA) BOOST_PP_REPEAT_2ND(BOOST_PP_SUB(LAST,FIRST),BOOST_PP_REPEAT_FROM_TO_2ND_F,(FIRST,MACRO,DATA))
#define BOOST_PP_REPEAT_FROM_TO_2ND_F(I,SMP) BOOST_PP_TUPLE_ELEM(3,1,SMP)(BOOST_PP_ADD(I,BOOST_PP_TUPLE_ELEM(3,0,SMP)),BOOST_PP_TUPLE_ELEM(3,2,SMP))
#endif
