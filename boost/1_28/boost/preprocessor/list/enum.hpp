#ifndef BOOST_PREPROCESSOR_LIST_ENUM_HPP
#define BOOST_PREPROCESSOR_LIST_ENUM_HPP

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

#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/list/for_each_i.hpp>

/** <p>Converts the list to a comma separated list.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_ENUM(BOOST_PP_TUPLE_TO_LIST(3,(A,B,C)))
</pre>

<p>expands to:</p>

<pre>
  A,B,C
</pre>

<h3>Uses</h3>
<ul>
  <li>BOOST_PP_LIST_FOR_EACH_I()</li>
</ul>
*/
#define BOOST_PP_LIST_ENUM(LIST) BOOST_PP_LIST_ENUM_R(0,LIST)

/** <p>Can be used inside BOOST_PP_FOR().</p> */
#define BOOST_PP_LIST_ENUM_R(R,LIST) BOOST_PP_LIST_FOR_EACH_I_R(R,BOOST_PP_LIST_ENUM_F,_,LIST)
#define BOOST_PP_LIST_ENUM_F(R,_,I,X) BOOST_PP_COMMA_IF(I) X
#endif
