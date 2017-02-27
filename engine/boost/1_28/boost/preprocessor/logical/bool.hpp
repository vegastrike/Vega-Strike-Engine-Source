#ifndef BOOST_PREPROCESSOR_LOGICAL_BOOL_HPP
#define BOOST_PREPROCESSOR_LOGICAL_BOOL_HPP

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

/** <p>Expands to <code>0</code> if <code>X == 0</code> and <code>1</code> if <code>X != 0</code>.</p>

<p><code>X</code> must be an integer literal in the range [0, BOOST_PP_LIMIT_MAG].</p>

<p>For example, <code>BOOST_PP_BOOL(3)</code> expands to <code>1</code>.</p>
*/
#define BOOST_PP_BOOL(X) BOOST_PP_BOOL_DELAY(X)

/* BOOL can be implemented in O(1) tokens using saturated ADD & SUB.
 * Unfortunately, it would result in significantly slower preprocessing.
 */
#define BOOST_PP_BOOL_DELAY(X) BOOST_PP_BOOL##X
#define BOOST_PP_BOOL0 0
#define BOOST_PP_BOOL1 1
#define BOOST_PP_BOOL2 1
#define BOOST_PP_BOOL3 1
#define BOOST_PP_BOOL4 1
#define BOOST_PP_BOOL5 1
#define BOOST_PP_BOOL6 1
#define BOOST_PP_BOOL7 1
#define BOOST_PP_BOOL8 1
#define BOOST_PP_BOOL9 1
#define BOOST_PP_BOOL10 1
#define BOOST_PP_BOOL11 1
#define BOOST_PP_BOOL12 1
#define BOOST_PP_BOOL13 1
#define BOOST_PP_BOOL14 1
#define BOOST_PP_BOOL15 1
#define BOOST_PP_BOOL16 1
#define BOOST_PP_BOOL17 1
#define BOOST_PP_BOOL18 1
#define BOOST_PP_BOOL19 1
#define BOOST_PP_BOOL20 1
#define BOOST_PP_BOOL21 1
#define BOOST_PP_BOOL22 1
#define BOOST_PP_BOOL23 1
#define BOOST_PP_BOOL24 1
#define BOOST_PP_BOOL25 1
#define BOOST_PP_BOOL26 1
#define BOOST_PP_BOOL27 1
#define BOOST_PP_BOOL28 1
#define BOOST_PP_BOOL29 1
#define BOOST_PP_BOOL30 1
#define BOOST_PP_BOOL31 1
#define BOOST_PP_BOOL32 1
#define BOOST_PP_BOOL33 1
#define BOOST_PP_BOOL34 1
#define BOOST_PP_BOOL35 1
#define BOOST_PP_BOOL36 1
#define BOOST_PP_BOOL37 1
#define BOOST_PP_BOOL38 1
#define BOOST_PP_BOOL39 1
#define BOOST_PP_BOOL40 1
#define BOOST_PP_BOOL41 1
#define BOOST_PP_BOOL42 1
#define BOOST_PP_BOOL43 1
#define BOOST_PP_BOOL44 1
#define BOOST_PP_BOOL45 1
#define BOOST_PP_BOOL46 1
#define BOOST_PP_BOOL47 1
#define BOOST_PP_BOOL48 1
#define BOOST_PP_BOOL49 1
#define BOOST_PP_BOOL50 1
#define BOOST_PP_BOOL51 1
#define BOOST_PP_BOOL52 1
#define BOOST_PP_BOOL53 1
#define BOOST_PP_BOOL54 1
#define BOOST_PP_BOOL55 1
#define BOOST_PP_BOOL56 1
#define BOOST_PP_BOOL57 1
#define BOOST_PP_BOOL58 1
#define BOOST_PP_BOOL59 1
#define BOOST_PP_BOOL60 1
#define BOOST_PP_BOOL61 1
#define BOOST_PP_BOOL62 1
#define BOOST_PP_BOOL63 1
#define BOOST_PP_BOOL64 1
#define BOOST_PP_BOOL65 1
#define BOOST_PP_BOOL66 1
#define BOOST_PP_BOOL67 1
#define BOOST_PP_BOOL68 1
#define BOOST_PP_BOOL69 1
#define BOOST_PP_BOOL70 1
#define BOOST_PP_BOOL71 1
#define BOOST_PP_BOOL72 1
#define BOOST_PP_BOOL73 1
#define BOOST_PP_BOOL74 1
#define BOOST_PP_BOOL75 1
#define BOOST_PP_BOOL76 1
#define BOOST_PP_BOOL77 1
#define BOOST_PP_BOOL78 1
#define BOOST_PP_BOOL79 1
#define BOOST_PP_BOOL80 1
#define BOOST_PP_BOOL81 1
#define BOOST_PP_BOOL82 1
#define BOOST_PP_BOOL83 1
#define BOOST_PP_BOOL84 1
#define BOOST_PP_BOOL85 1
#define BOOST_PP_BOOL86 1
#define BOOST_PP_BOOL87 1
#define BOOST_PP_BOOL88 1
#define BOOST_PP_BOOL89 1
#define BOOST_PP_BOOL90 1
#define BOOST_PP_BOOL91 1
#define BOOST_PP_BOOL92 1
#define BOOST_PP_BOOL93 1
#define BOOST_PP_BOOL94 1
#define BOOST_PP_BOOL95 1
#define BOOST_PP_BOOL96 1
#define BOOST_PP_BOOL97 1
#define BOOST_PP_BOOL98 1
#define BOOST_PP_BOOL99 1
#define BOOST_PP_BOOL100 1
#define BOOST_PP_BOOL101 1
#define BOOST_PP_BOOL102 1
#define BOOST_PP_BOOL103 1
#define BOOST_PP_BOOL104 1
#define BOOST_PP_BOOL105 1
#define BOOST_PP_BOOL106 1
#define BOOST_PP_BOOL107 1
#define BOOST_PP_BOOL108 1
#define BOOST_PP_BOOL109 1
#define BOOST_PP_BOOL110 1
#define BOOST_PP_BOOL111 1
#define BOOST_PP_BOOL112 1
#define BOOST_PP_BOOL113 1
#define BOOST_PP_BOOL114 1
#define BOOST_PP_BOOL115 1
#define BOOST_PP_BOOL116 1
#define BOOST_PP_BOOL117 1
#define BOOST_PP_BOOL118 1
#define BOOST_PP_BOOL119 1
#define BOOST_PP_BOOL120 1
#define BOOST_PP_BOOL121 1
#define BOOST_PP_BOOL122 1
#define BOOST_PP_BOOL123 1
#define BOOST_PP_BOOL124 1
#define BOOST_PP_BOOL125 1
#define BOOST_PP_BOOL126 1
#define BOOST_PP_BOOL127 1
#define BOOST_PP_BOOL128 1

/* <p>Obsolete. Use BOOST_PP_BOOL().</p> */
#define BOOST_PREPROCESSOR_BOOL(X) BOOST_PP_BOOL(X)
#endif
