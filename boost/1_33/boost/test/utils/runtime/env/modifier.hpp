//  (C) Copyright Gennadiy Rozental 2005.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision$
//
//  Description : defines variable modifiers
// ***************************************************************************

#ifndef BOOST_RT_ENV_MODIFIER_HPP_062604GER
#define BOOST_RT_ENV_MODIFIER_HPP_062604GER

// Boost.Runtime.Parameter
#include <boost/test/utils/runtime/config.hpp>

// Boost.Test
#include <boost/test/utils/named_params.hpp>

namespace boost {

namespace BOOST_RT_PARAM_NAMESPACE {

namespace environment {

// ************************************************************************** //
// **************         environment variable modifiers       ************** //
// ************************************************************************** //

namespace {

nfp::typed_keyword<cstring,struct global_id_t>   global_id;
nfp::keyword<struct default_value_t>             default_value;
nfp::keyword<struct interpreter_t>               interpreter;

} // local namespace
} // namespace environment

} // namespace BOOST_RT_PARAM_NAMESPACE

} // namespace boost

// ************************************************************************** //
//   Revision History:
//
//   $Log$
//   Revision 1.1  2005/11/27 04:18:28  balloyd1
//   First commit of a more streamlined and maintainable autotools build system.
//   Added OGRE support for mesher (optional)
//   Integrated a number of disparate build systems into the master Makefile.
//   Added boost 1.33 support and streamlined addition of new boost versions.
//
//   Revision 1.1  2005/04/12 06:42:43  rogeeff
//   Runtime.Param library initial commit
//
// ************************************************************************** //

#endif // BOOST_RT_ENV_MODIFIER_HPP_062604GER
