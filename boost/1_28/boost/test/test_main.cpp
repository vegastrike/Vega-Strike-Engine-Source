//  boost/test_main.cpp (header or not, as you like it) ----------------------//

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
//   26 Feb 01  Numerous changes suggested during formal review. (Beman)
//   22 Jan 01 Use boost/cpp_main.hpp as framework. (Beman)
//    5 Nov 00 Initial boost version (Beman Dawes)

//  header dependencies are deliberately kept minimal to reducing coupling.
#include <boost/cstdlib.hpp>  // for exit codes
#include <iostream>
#include <stdexcept>

namespace boost
{
  namespace test
  {
    int test_tools_errors = 0;  // count of errors detected

    class test_tools_exception : public std::runtime_error
    {
     public: 
      test_tools_exception() : std::runtime_error("fatal test error") {};
    };
  }

  void report_error( const char * msg, const char * file, int line )
  {
    ++test::test_tools_errors;
    std::cout << "\n**** test failed: " << msg << ", file: " << file
              << ", line: " << line << std::endl;
  }

  void report_critical_error( const char * msg, const char * file, int line )
  {
    report_error( msg, file, line );
    throw test::test_tools_exception();
  }
}

//  cpp_main()  --------------------------------------------------------------//

//  See the boost/cpp_main.cpp comments; they apply to cpp_main and test_main.

int test_main( int argc, char * argv[] );  // prototype for user's test_main()

int cpp_main( int argc, char * argv[] )
{
  int result = 0;               // quiet compiler warnings

  try
  {
    result = test_main( argc, argv );
  }

  // The rules for catch & arguments are a bit different from function 
  // arguments (ISO 15.3 paragraphs 18 & 19). Apparently const isn't
  // required, but it doesn't hurt and some programmers ask for it.

  catch ( const boost::test::test_tools_exception & )
  { 
    std::cout << "\n**** previous test error is fatal" << std::endl;
    boost::test::test_tools_errors = 0;  // reset so we don't get two messages
    result = boost::exit_test_failure;
  }

  if ( boost::test::test_tools_errors )
  {
    std::cout << "\n**** " << boost::test::test_tools_errors
              << " test errors detected" << std::endl;
    result = boost::exit_test_failure;
  }

  return result;
}
