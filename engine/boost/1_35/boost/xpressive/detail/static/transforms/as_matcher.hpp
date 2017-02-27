///////////////////////////////////////////////////////////////////////////////
// as_matcher.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MATCHER_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MATCHER_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>

namespace boost { namespace xpressive { namespace detail
{

    template<typename Grammar>
    struct as_matcher
      : Grammar
    {
        as_matcher();

        template<typename Expr, typename State, typename Visitor>
        struct apply
          : Visitor::template apply<
                typename proto::result_of::arg<Expr>::type
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            return visitor.call(proto::arg(expr));
        }
    };

}}}

#endif
