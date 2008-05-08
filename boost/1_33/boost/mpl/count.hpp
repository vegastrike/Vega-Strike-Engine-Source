
#ifndef BOOST_MPL_COUNT_HPP_INCLUDED
#define BOOST_MPL_COUNT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source$
// $Date: 2005-11-26 23:18:48 -0500 (Sat, 26 Nov 2005) $
// $Revision: 9977 $

#include <boost/mpl/count_fwd.hpp>
#include <boost/mpl/sequence_tag.hpp>
#include <boost/mpl/aux_/count_impl.hpp>
#include <boost/mpl/aux_/na_spec.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>

namespace boost { namespace mpl {

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(T)
    >
struct count
    : count_impl< typename sequence_tag<Sequence>::type >
        ::template apply<Sequence,T>
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(2,count,(Sequence,T))
};

BOOST_MPL_AUX_NA_SPEC(2, count)

}}

#endif // BOOST_MPL_COUNT_HPP_INCLUDED
