// Boost.Function library
//
// Copyright (C) 2001 Doug Gregor (gregod@cs.rpi.edu)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.
 
// For more information, see http://www.boost.org

#ifndef BOOST_FUNCTION_FUNCTION2_HEADER
#define BOOST_FUNCTION_FUNCTION2_HEADER

#define BOOST_FUNCTION_NUM_ARGS 2
#define BOOST_FUNCTION_TEMPLATE_PARMS typename T0, typename T1
#define BOOST_FUNCTION_TEMPLATE_ARGS T0, T1
#define BOOST_FUNCTION_PARMS T0 a0, T1 a1
#define BOOST_FUNCTION_ARGS a0, a1
#define BOOST_FUNCTION_NOT_0_PARMS T1 a1
#define BOOST_FUNCTION_NOT_0_ARGS a1

#include <boost/function/function_template.hpp>

#undef BOOST_FUNCTION_NOT_0_ARGS
#undef BOOST_FUNCTION_NOT_0_PARMS
#undef BOOST_FUNCTION_ARGS
#undef BOOST_FUNCTION_PARMS
#undef BOOST_FUNCTION_TEMPLATE_ARGS
#undef BOOST_FUNCTION_TEMPLATE_PARMS
#undef BOOST_FUNCTION_NUM_ARGS

#endif // BOOST_FUNCTION_FUNCTION2_HEADER
