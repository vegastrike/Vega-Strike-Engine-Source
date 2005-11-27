//  (C) Copyright Gennadiy Rozental 2005.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : wraps io savers staff to be provide workaround for classic iostreams
// ***************************************************************************

#ifndef BOOST_WRAP_IO_SAVER_HPP_011605GER
#define BOOST_WRAP_IO_SAVER_HPP_011605GER

#if defined(BOOST_STANDARD_IOSTREAMS)
#include <boost/io/ios_state.hpp>
#endif

namespace boost {

namespace unit_test {

#if defined(BOOST_STANDARD_IOSTREAMS)

typedef ::boost::io::ios_base_all_saver io_saver_type;

#else

struct io_saver_type {
    explicit io_saver_type( std::ostream& ) {}
    void     restore() {}
};

#endif

} // namespace unit_test

} // namespace boost

// ***************************************************************************
//  Revision History :
//  
//  $Log$
//  Revision 1.1  2005/11/27 04:18:28  balloyd1
//  First commit of a more streamlined and maintainable autotools build system.
//  Added OGRE support for mesher (optional)
//  Integrated a number of disparate build systems into the master Makefile.
//  Added boost 1.33 support and streamlined addition of new boost versions.
//
//  Revision 1.1  2005/04/30 16:48:21  rogeeff
//  io saver warkaround for classic io is shared
//
//  Revision 1.1  2005/02/20 08:27:06  rogeeff
//  This a major update for Boost.Test framework. See release docs for complete list of fixes/updates
//
// ***************************************************************************

#endif // BOOST_WRAP_IO_SAVER_HPP_011605GER

