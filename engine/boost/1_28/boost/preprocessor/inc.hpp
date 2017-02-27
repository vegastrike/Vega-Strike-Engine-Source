#ifndef BOOST_PREPROCESSOR_INC_HPP
#define BOOST_PREPROCESSOR_INC_HPP

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

/** <p>Increments <code>X</code> expanding to a single token.</p>

<p>For example, <code>BOOST_PP_INC(3)</code> expands to <code>4</code> (a
single token).</p>

<p>BOOST_PP_INC() uses saturation arithmetic. Incrementing a
BOOST_PP_LIMIT_MAG yields a BOOST_PP_LIMIT_MAG.</p>

<p>Only decimal integer literals in the range [0,BOOST_PP_LIMIT_MAG] are
supported.</p>
*/
#define BOOST_PP_INC(X) BOOST_PP_INC_DELAY(X)

#define BOOST_PP_INC_DELAY(X) BOOST_PP_INC##X
#define BOOST_PP_INC0 1
#define BOOST_PP_INC1 2
#define BOOST_PP_INC2 3
#define BOOST_PP_INC3 4
#define BOOST_PP_INC4 5
#define BOOST_PP_INC5 6
#define BOOST_PP_INC6 7
#define BOOST_PP_INC7 8
#define BOOST_PP_INC8 9
#define BOOST_PP_INC9 10
#define BOOST_PP_INC10 11
#define BOOST_PP_INC11 12
#define BOOST_PP_INC12 13
#define BOOST_PP_INC13 14
#define BOOST_PP_INC14 15
#define BOOST_PP_INC15 16
#define BOOST_PP_INC16 17
#define BOOST_PP_INC17 18
#define BOOST_PP_INC18 19
#define BOOST_PP_INC19 20
#define BOOST_PP_INC20 21
#define BOOST_PP_INC21 22
#define BOOST_PP_INC22 23
#define BOOST_PP_INC23 24
#define BOOST_PP_INC24 25
#define BOOST_PP_INC25 26
#define BOOST_PP_INC26 27
#define BOOST_PP_INC27 28
#define BOOST_PP_INC28 29
#define BOOST_PP_INC29 30
#define BOOST_PP_INC30 31
#define BOOST_PP_INC31 32
#define BOOST_PP_INC32 33
#define BOOST_PP_INC33 34
#define BOOST_PP_INC34 35
#define BOOST_PP_INC35 36
#define BOOST_PP_INC36 37
#define BOOST_PP_INC37 38
#define BOOST_PP_INC38 39
#define BOOST_PP_INC39 40
#define BOOST_PP_INC40 41
#define BOOST_PP_INC41 42
#define BOOST_PP_INC42 43
#define BOOST_PP_INC43 44
#define BOOST_PP_INC44 45
#define BOOST_PP_INC45 46
#define BOOST_PP_INC46 47
#define BOOST_PP_INC47 48
#define BOOST_PP_INC48 49
#define BOOST_PP_INC49 50
#define BOOST_PP_INC50 51
#define BOOST_PP_INC51 52
#define BOOST_PP_INC52 53
#define BOOST_PP_INC53 54
#define BOOST_PP_INC54 55
#define BOOST_PP_INC55 56
#define BOOST_PP_INC56 57
#define BOOST_PP_INC57 58
#define BOOST_PP_INC58 59
#define BOOST_PP_INC59 60
#define BOOST_PP_INC60 61
#define BOOST_PP_INC61 62
#define BOOST_PP_INC62 63
#define BOOST_PP_INC63 64
#define BOOST_PP_INC64 65
#define BOOST_PP_INC65 66
#define BOOST_PP_INC66 67
#define BOOST_PP_INC67 68
#define BOOST_PP_INC68 69
#define BOOST_PP_INC69 70
#define BOOST_PP_INC70 71
#define BOOST_PP_INC71 72
#define BOOST_PP_INC72 73
#define BOOST_PP_INC73 74
#define BOOST_PP_INC74 75
#define BOOST_PP_INC75 76
#define BOOST_PP_INC76 77
#define BOOST_PP_INC77 78
#define BOOST_PP_INC78 79
#define BOOST_PP_INC79 80
#define BOOST_PP_INC80 81
#define BOOST_PP_INC81 82
#define BOOST_PP_INC82 83
#define BOOST_PP_INC83 84
#define BOOST_PP_INC84 85
#define BOOST_PP_INC85 86
#define BOOST_PP_INC86 87
#define BOOST_PP_INC87 88
#define BOOST_PP_INC88 89
#define BOOST_PP_INC89 90
#define BOOST_PP_INC90 91
#define BOOST_PP_INC91 92
#define BOOST_PP_INC92 93
#define BOOST_PP_INC93 94
#define BOOST_PP_INC94 95
#define BOOST_PP_INC95 96
#define BOOST_PP_INC96 97
#define BOOST_PP_INC97 98
#define BOOST_PP_INC98 99
#define BOOST_PP_INC99 100
#define BOOST_PP_INC100 101
#define BOOST_PP_INC101 102
#define BOOST_PP_INC102 103
#define BOOST_PP_INC103 104
#define BOOST_PP_INC104 105
#define BOOST_PP_INC105 106
#define BOOST_PP_INC106 107
#define BOOST_PP_INC107 108
#define BOOST_PP_INC108 109
#define BOOST_PP_INC109 110
#define BOOST_PP_INC110 111
#define BOOST_PP_INC111 112
#define BOOST_PP_INC112 113
#define BOOST_PP_INC113 114
#define BOOST_PP_INC114 115
#define BOOST_PP_INC115 116
#define BOOST_PP_INC116 117
#define BOOST_PP_INC117 118
#define BOOST_PP_INC118 119
#define BOOST_PP_INC119 120
#define BOOST_PP_INC120 121
#define BOOST_PP_INC121 122
#define BOOST_PP_INC122 123
#define BOOST_PP_INC123 124
#define BOOST_PP_INC124 125
#define BOOST_PP_INC125 126
#define BOOST_PP_INC126 127
#define BOOST_PP_INC127 128
#define BOOST_PP_INC128 128

/* <p>Obsolete. Use BOOST_PP_INC().</p> */
#define BOOST_PREPROCESSOR_INC(X) BOOST_PP_INC(X)
#endif
