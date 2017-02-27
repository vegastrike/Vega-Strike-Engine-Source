//  test tools header  -------------------------------------------------------//

//  (C) Copyright Beman Dawes 2000. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright notice
//  appears in all copies. This software is provided "as is" without express or
//  implied warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for updates, documentation, and revision history.

//  Revision History
//   26 Feb 01  Numerous changes suggested during formal review. (Beman)
//    7 Feb 01  #include <boost/test/test_main.cpp> if requested. (Beman)
//   22 Jan 01  Remove all header dependencies. (Beman)
//    3 Dec 00  Remove function definitions. (Ed Brey)
//    5 Nov 00  Initial boost version (Beman Dawes)

#ifndef BOOST_TEST_TOOLS_HPP
#define BOOST_TEST_TOOLS_HPP

//  header dependencies eliminated to reducing coupling.

//  macros (gasp!) ease use of reporting functions

#define BOOST_TEST(exp) ((exp) ? static_cast<void>(0) : boost::report_error(#exp,__FILE__,__LINE__))
//  Effects: if (!exp) call report_error().

#define BOOST_CRITICAL_TEST(exp) ((exp) ? static_cast<void>(0) : boost::report_critical_error(#exp,__FILE__,__LINE__))
//  Effects: if (!exp) call report_critical_error().

#define BOOST_ERROR(msg) boost::report_error((msg),__FILE__,__LINE__)

#define BOOST_CRITICAL_ERROR(msg) boost::report_critical_error((msg),__FILE__,__LINE__)

namespace boost
{
  //  Function implementations are not inline because it is better design to
  //  decouple implementation, and because space is more important than speed
  //  since error functions get called relatively infrequently.  Note that
  //  separating implementatiion means that this header could be useful
  //  without using the boost/test_main.hpp header for a main() function,
  //  and/or a different implementation could be supplied at link time.

  void report_error( const char * msg, const char * file, int line );
  //  Effects: increment test_tools_error counter, write error message to cout.

  void report_critical_error( const char * msg, const char * file, int line );
  //  Effects: report_error(msg,file,line), throw test_tools_exception.
}

//  for convenience, allow the user to request inclusion of lower-level layers
#ifdef BOOST_INCLUDE_MAIN
#include <boost/test/cpp_main.cpp>
#include <boost/test/test_main.cpp>
#endif

#endif // BOOST_TEST_TOOLS_HPP
