#ifndef BOOST_PREPROCESSOR_COMMA_HPP
#define BOOST_PREPROCESSOR_COMMA_HPP

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

/** <p>Expands to a comma.</p>

<p>Commas need special handling in preprocessor code, because commas are used
for separating macro parameters.</p>

<p>For example,</p>

<pre>
  BOOST_PP_IF(1,BOOST_PP_COMMA,BOOST_PP_EMPTY)()
</pre>

<p>expands to a comma.</p>

<h3>See</h3>
<ul>
  <li>BOOST_PP_COMMA_IF()</li>
</ul>
*/
#define BOOST_PP_COMMA() ,

/* <p>Obsolete. Use BOOST_PP_COMMA().</p> */
#define BOOST_PREPROCESSOR_COMMA() ,
#endif
