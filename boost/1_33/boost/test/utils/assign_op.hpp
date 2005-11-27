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
//  Description : overloadable assignment
// ***************************************************************************

#ifndef BOOST_TEST_ASSIGN_OP_033005GER
#define BOOST_TEST_ASSIGN_OP_033005GER

namespace boost {

namespace unit_test {

// ************************************************************************** //
// **************             generic assign operator          ************** //
// ************************************************************************** //

// generic
template<typename T,typename S>
inline void
assign_op( T& t, S const& s, long )
{
    t = s;
}

//____________________________________________________________________________//

} // namespace unit_test

} // namespace boost

// ***************************************************************************
//   Revision History:
//  
//  $Log$
//  Revision 1.1  2005/11/27 04:18:28  balloyd1
//  First commit of a more streamlined and maintainable autotools build system.
//  Added OGRE support for mesher (optional)
//  Integrated a number of disparate build systems into the master Makefile.
//  Added boost 1.33 support and streamlined addition of new boost versions.
//
//  Revision 1.1  2005/04/12 06:48:12  rogeeff
//  Runtime.Param library initial commit
//
// ***************************************************************************

#endif // BOOST_TEST_ASSIGN_OP_033005GER

