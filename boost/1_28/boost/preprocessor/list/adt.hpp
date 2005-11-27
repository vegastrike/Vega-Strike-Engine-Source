#ifndef BOOST_PREPROCESSOR_LIST_ADT_HPP
#define BOOST_PREPROCESSOR_LIST_ADT_HPP

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

/** <p>This header defines the fundamental list operations.</p>

<h3>Note</h3>
<ul>
  <li>The internal representation of lists is hidden. Although there aren't
      compelling reasons to change the representation, you should avoid
      writing code that depends on the internal representation details.</li>
</ul>
*/

#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/logical/not.hpp>

/** <p>List constructor.</p>

<p>Lists are build using list constructors BOOST_PP_LIST_NIL and
BOOST_PP_LIST_CONS(). For example,</p>

<pre>
  BOOST_PP_LIST_CONS(1,
  BOOST_PP_LIST_CONS(2,
  BOOST_PP_LIST_CONS(3,
  BOOST_PP_LIST_CONS(4,
  BOOST_PP_LIST_CONS(5,
  BOOST_PP_LIST_NIL)))))
</pre>

<p>Short lists can also be build from tuples:</p>

<pre>
  BOOST_PP_TUPLE_TO_LIST(5,(1,2,3,4,5))
</pre>

<p>Both of the above lists contain 5 elements: 1, 2, 3, 4 and 5.</p>

<p>Longer lists can be built from short lists with BOOST_PP_LIST_APPEND_D()
and BOOST_PP_LIST_FOLD_RIGHT_2ND():</p>

<pre>
  BOOST_PP_LIST_FOLD_RIGHT_2ND
  ( BOOST_PP_LIST_APPEND_D
  , BOOST_PP_TUPLE_TO_LIST
    ( N
    , BOOST_PP_TUPLE_TO_LIST(M, (E11, E12, ..., E1M) )
    , BOOST_PP_TUPLE_TO_LIST(M, (E21, E22, ..., E2M) )
    , ...
    , BOOST_PP_TUPLE_TO_LIST(M, (EN1, EN2, ..., ENM) )
    )
  )
</pre>
*/
#define BOOST_PP_LIST_CONS(FIRST,REST) (FIRST,REST,1)

/** <p>List nil constructor.</p> */
#define BOOST_PP_LIST_NIL (_,_,0)

/** <p>Expands to 1 if the list is not nil and 0 otherwise.</p> */
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_IS_CONS(LIST) BOOST_PP_TUPLE_ELEM(3,2,LIST)
#else
#  define BOOST_PP_LIST_IS_CONS(LIST) BOOST_PP_TUPLE3_ELEM2 LIST
#endif

/** <p>Expands to 1 if the list is nil and 0 otherwise.</p> */
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_IS_NIL(LIST) BOOST_PP_NOT(BOOST_PP_TUPLE_ELEM(3,2,LIST))
#else
#  define BOOST_PP_LIST_IS_NIL(LIST) BOOST_PP_NOT(BOOST_PP_TUPLE3_ELEM2 LIST)
#endif

/** <p>Expands to the first element of the list. The list must not be nil.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_FIRST(BOOST_PP_TUPLE_TO_LIST(5,(1,2,3,4,5)))
</pre>

<p>expands to 1.</p>
*/
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_FIRST(LIST) BOOST_PP_TUPLE_ELEM(3,0,LIST)
#else
#  define BOOST_PP_LIST_FIRST(LIST) BOOST_PP_TUPLE3_ELEM0 LIST
#endif

/** <p>Expands to a list of all but the first element of the list.</p>

<p>The list must not be nil.</p>

<p>For example,</p>

<pre>
  BOOST_PP_LIST_REST(BOOST_PP_TUPLE_TO_LIST(5,(1,2,3,4,5)))
</pre>

<p>expands to the same as:</p>

<pre>
  BOOST_PP_TUPLE_TO_LIST(4,(2,3,4,5))
</pre>
*/
#if !defined(BOOST_NO_COMPILER_CONFIG) && defined(__MWERKS__)
#  define BOOST_PP_LIST_REST(LIST) BOOST_PP_TUPLE_ELEM(3,1,LIST)
#else
#  define BOOST_PP_LIST_REST(LIST) BOOST_PP_TUPLE3_ELEM1 LIST
#endif
#endif
