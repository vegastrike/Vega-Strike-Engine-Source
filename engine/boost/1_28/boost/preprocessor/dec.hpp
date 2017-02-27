#ifndef BOOST_PREPROCESSOR_DEC_HPP
#define BOOST_PREPROCESSOR_DEC_HPP

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

/** <p>Decrements <code>X</code> expanding to a single token.</p>

<p>For example, <code>BOOST_PP_DEC(3)</code> expands to <code>2</code> (a
single token).</p>

<p>BOOST_PP_DEC() uses saturation arithmetic. Decrementing 0 yeilds a 0.</p>

<p>Only decimal integer literals in the range [0,BOOST_PP_LIMIT_MAG] are
supported.</p>
*/
#define BOOST_PP_DEC(X) BOOST_PP_DEC_DELAY(X)

#define BOOST_PP_DEC_DELAY(X) BOOST_PP_DEC##X
#define BOOST_PP_DEC0 0
#define BOOST_PP_DEC1 0
#define BOOST_PP_DEC2 1
#define BOOST_PP_DEC3 2
#define BOOST_PP_DEC4 3
#define BOOST_PP_DEC5 4
#define BOOST_PP_DEC6 5
#define BOOST_PP_DEC7 6
#define BOOST_PP_DEC8 7
#define BOOST_PP_DEC9 8
#define BOOST_PP_DEC10 9
#define BOOST_PP_DEC11 10
#define BOOST_PP_DEC12 11
#define BOOST_PP_DEC13 12
#define BOOST_PP_DEC14 13
#define BOOST_PP_DEC15 14
#define BOOST_PP_DEC16 15
#define BOOST_PP_DEC17 16
#define BOOST_PP_DEC18 17
#define BOOST_PP_DEC19 18
#define BOOST_PP_DEC20 19
#define BOOST_PP_DEC21 20
#define BOOST_PP_DEC22 21
#define BOOST_PP_DEC23 22
#define BOOST_PP_DEC24 23
#define BOOST_PP_DEC25 24
#define BOOST_PP_DEC26 25
#define BOOST_PP_DEC27 26
#define BOOST_PP_DEC28 27
#define BOOST_PP_DEC29 28
#define BOOST_PP_DEC30 29
#define BOOST_PP_DEC31 30
#define BOOST_PP_DEC32 31
#define BOOST_PP_DEC33 32
#define BOOST_PP_DEC34 33
#define BOOST_PP_DEC35 34
#define BOOST_PP_DEC36 35
#define BOOST_PP_DEC37 36
#define BOOST_PP_DEC38 37
#define BOOST_PP_DEC39 38
#define BOOST_PP_DEC40 39
#define BOOST_PP_DEC41 40
#define BOOST_PP_DEC42 41
#define BOOST_PP_DEC43 42
#define BOOST_PP_DEC44 43
#define BOOST_PP_DEC45 44
#define BOOST_PP_DEC46 45
#define BOOST_PP_DEC47 46
#define BOOST_PP_DEC48 47
#define BOOST_PP_DEC49 48
#define BOOST_PP_DEC50 49
#define BOOST_PP_DEC51 50
#define BOOST_PP_DEC52 51
#define BOOST_PP_DEC53 52
#define BOOST_PP_DEC54 53
#define BOOST_PP_DEC55 54
#define BOOST_PP_DEC56 55
#define BOOST_PP_DEC57 56
#define BOOST_PP_DEC58 57
#define BOOST_PP_DEC59 58
#define BOOST_PP_DEC60 59
#define BOOST_PP_DEC61 60
#define BOOST_PP_DEC62 61
#define BOOST_PP_DEC63 62
#define BOOST_PP_DEC64 63
#define BOOST_PP_DEC65 64
#define BOOST_PP_DEC66 65
#define BOOST_PP_DEC67 66
#define BOOST_PP_DEC68 67
#define BOOST_PP_DEC69 68
#define BOOST_PP_DEC70 69
#define BOOST_PP_DEC71 70
#define BOOST_PP_DEC72 71
#define BOOST_PP_DEC73 72
#define BOOST_PP_DEC74 73
#define BOOST_PP_DEC75 74
#define BOOST_PP_DEC76 75
#define BOOST_PP_DEC77 76
#define BOOST_PP_DEC78 77
#define BOOST_PP_DEC79 78
#define BOOST_PP_DEC80 79
#define BOOST_PP_DEC81 80
#define BOOST_PP_DEC82 81
#define BOOST_PP_DEC83 82
#define BOOST_PP_DEC84 83
#define BOOST_PP_DEC85 84
#define BOOST_PP_DEC86 85
#define BOOST_PP_DEC87 86
#define BOOST_PP_DEC88 87
#define BOOST_PP_DEC89 88
#define BOOST_PP_DEC90 89
#define BOOST_PP_DEC91 90
#define BOOST_PP_DEC92 91
#define BOOST_PP_DEC93 92
#define BOOST_PP_DEC94 93
#define BOOST_PP_DEC95 94
#define BOOST_PP_DEC96 95
#define BOOST_PP_DEC97 96
#define BOOST_PP_DEC98 97
#define BOOST_PP_DEC99 98
#define BOOST_PP_DEC100 99
#define BOOST_PP_DEC101 100
#define BOOST_PP_DEC102 101
#define BOOST_PP_DEC103 102
#define BOOST_PP_DEC104 103
#define BOOST_PP_DEC105 104
#define BOOST_PP_DEC106 105
#define BOOST_PP_DEC107 106
#define BOOST_PP_DEC108 107
#define BOOST_PP_DEC109 108
#define BOOST_PP_DEC110 109
#define BOOST_PP_DEC111 110
#define BOOST_PP_DEC112 111
#define BOOST_PP_DEC113 112
#define BOOST_PP_DEC114 113
#define BOOST_PP_DEC115 114
#define BOOST_PP_DEC116 115
#define BOOST_PP_DEC117 116
#define BOOST_PP_DEC118 117
#define BOOST_PP_DEC119 118
#define BOOST_PP_DEC120 119
#define BOOST_PP_DEC121 120
#define BOOST_PP_DEC122 121
#define BOOST_PP_DEC123 122
#define BOOST_PP_DEC124 123
#define BOOST_PP_DEC125 124
#define BOOST_PP_DEC126 125
#define BOOST_PP_DEC127 126
#define BOOST_PP_DEC128 127

/* <p>Obsolete. Use BOOST_PP_DEC().</p> */
#define BOOST_PREPROCESSOR_DEC(X) BOOST_PP_DEC(X)
#endif
