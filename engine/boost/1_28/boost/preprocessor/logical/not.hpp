#ifndef BOOST_PREPROCESSOR_LOGICAL_NOT_HPP
#define BOOST_PREPROCESSOR_LOGICAL_NOT_HPP

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

#include <boost/preprocessor/logical/nor.hpp>

/** <p>Expands to the logical NOT of the operand.</p>

<p><code>X</code> must be an integer literal in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_NOT(0)</code> expands to <code>1</code> (a single token).</p>

<h3>Test</h3>
<ul>
  <li><a href="../../test/logical_test.cpp">logical_test.cpp</a></li>
</ul>
*/
#define BOOST_PP_NOT(X) BOOST_PP_NOR(X,X)

/* <p>Obsolete. Use BOOST_PP_NOT().</p> */
#define BOOST_PREPROCESSOR_NOT(X) BOOST_PP_NOT(X)
#endif
