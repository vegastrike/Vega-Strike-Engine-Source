///////////////////////////////////////////////////////////////////////////////
// as_independent.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INDEPENDENT_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INDEPENDENT_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform/arg.hpp>

namespace boost { namespace xpressive { namespace detail
{
    struct keeper_tag
    {};

    struct lookahead_tag
    {};

    struct lookbehind_tag
    {};

    template<typename Grammar>
    struct as_lookahead
      : Grammar
    {
        as_lookahead();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef lookahead_matcher<
                typename Grammar::template apply<Expr, true_xpression, Visitor>::type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, true_xpression(), visitor)
              , false
            );
        }
    };

    template<typename Grammar>
    struct as_lookbehind
      : Grammar
    {
        as_lookbehind();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef lookbehind_matcher<
                typename Grammar::template apply<Expr, true_xpression, Visitor>::type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            typename Grammar::template apply<Expr, true_xpression, Visitor>::type const &
                expr2 = Grammar::call(expr, true_xpression(), visitor);
            std::size_t width = expr2.get_width().value();
            return typename apply<Expr, State, Visitor>::type(expr2, width, false);
        }
    };

    template<typename Grammar>
    struct as_keeper
      : Grammar
    {
        as_keeper();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef keeper_matcher<
                typename Grammar::template apply<Expr, true_xpression, Visitor>::type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, true_xpression(), visitor)
            );
        }
    };

}}}

#endif
