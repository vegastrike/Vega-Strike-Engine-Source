//  boost/cpp_main.cpp (header or not, as you like it)  ----------------------//

//  (C) Copyright Beman Dawes 1995-2001. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright notice
//  appears in all copies. This software is provided "as is" without express or
//  implied warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for updates, documentation, and revision history.

//  This file may be included as a header file, or may be compiled and placed
//  in a library for traditional linking. It is unusual for non-template
//  non-inline implementation code to be used as a header file, but the user
//  may elect to do so because header-only implementation requires no library
//  build support. (Suggested by Ed Brey)

//  Revision History
//   26 Feb 01 Numerous changes suggested during formal review. (Beman)
//   25 Jan 01 catch_exceptions.hpp code factored out.
//   22 Jan 01 Remove test_tools dependencies to reduce coupling.
//    5 Nov 00 Initial boost version (Beman Dawes)

//  header dependencies are deliberately restricted to reduce coupling.
#include <iostream>
#include <boost/detail/catch_exceptions.hpp>

//  main()  ------------------------------------------------------------------//

//  Uniformly detects and reports the occurrence of several types of errors,
//  reducing the various errors to a uniform return value which is returned
//  to the host environment.
//
//  There are two intended uses:
//
//    * In production programs, which require no further action beyond
//      naming the top-level function cpp_main() instead of main().
//
//    * In test frameworks, which supply cpp_main() to detect (or catch)
//      test specific errors, report them,  and then return a presumably
//      non-zero value. 
//
//  Requires: A user-supplied cpp_main() function with same interface as main().
//
//  Effects:
//
//    Call cpp_main( argc, argv ) in a try block.
//  
//    Treat as errors:
//
//      *  exceptions from cpp_main().
//      *  non-zero return from cpp_main().
//
//    Report errors to both cout (with details) and cerr (summary).
//    Rationale: Detail error reporting goes to cout so that it is properly
//    interlaced with other output, thus aiding error analysis. Summary goes
//    to cerr in case cout is redirected.
//
//  Returns: non-zero if any error was detected.
//
//  Note: this header is designed to work well if boost test tools happen to be
//  used, but there is no requirement that they be used. There are no
//  dependencies on test_tools headers. 

int cpp_main( int argc, char * argv[] );  // prototype for user's cpp_main()

namespace boost
{
  namespace test
  {
    class cpp_main_caller
    {
      int argc;
      char ** argv;
     public:
      cpp_main_caller( int c, char ** v ) : argc(c), argv(v) {}
      int operator()() { return cpp_main( argc, argv ); }
    };
  }
}

int main( int argc, char * argv[] )
{
  return boost::catch_exceptions( boost::test::cpp_main_caller(argc, argv),
                                  std::cout, std::cerr );
}
