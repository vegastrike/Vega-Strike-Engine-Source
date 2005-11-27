#ifndef BOOST_PREPROCESSOR_TUPLE_HPP
#define BOOST_PREPROCESSOR_TUPLE_HPP

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

/** <p>Includes all tuple headers.</p>

<p>A tuple is a fixed size collection of elements.</p>

<p>In the preprocessor library, tuples are represented like macro parameter
lists. Thus an element of a tuple can be any sequence of tokens that
constitutes a single macro parameter.</p>

<p>Examples of tuples:</p>

<pre>
  (const, volatile)    // 2-tuple
  (*, /, %)            // 3-tuple
  (1, "2", '3', (4,5)) // 4-tuple
</pre>

<p>Tuples can be used for representing structured data.</p>
*/

#include <boost/preprocessor/tuple/eat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/reverse.hpp>
#include <boost/preprocessor/tuple/to_list.hpp>
#endif
