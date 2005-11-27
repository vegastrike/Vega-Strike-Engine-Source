#ifndef BOOST_PREPROCESSOR_EXPAND_HPP
#define BOOST_PREPROCESSOR_EXPAND_HPP

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

#include <boost/preprocessor/identity.hpp>

/** <p>Essentially macro expands the parameter <code>X</code> twice.</p> */
#define BOOST_PP_EXPAND(X) X
#endif
