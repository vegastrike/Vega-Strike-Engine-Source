///////////////////////////////////////////////////////////////////////////////
// as_alternate.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ALTERNATE_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ALTERNATE_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/config.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/proto/traits.hpp>
#include <boost/xpressive/proto/matches.hpp>
#include <boost/xpressive/proto/transform/fold.hpp>
#include <boost/xpressive/proto/transform/branch.hpp>
#include <boost/xpressive/proto/transform/fold_tree.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/detail/core/matcher/alternate_matcher.hpp>
#include <boost/xpressive/detail/utility/cons.hpp>

namespace boost { namespace xpressive { namespace detail
{

    ///////////////////////////////////////////////////////////////////////////////
    // alternates_list
    //   a fusion-compatible sequence of alternate expressions, that also keeps
    //   track of the list's width and purity.
    template<typename Head, typename Tail>
    struct alternates_list
      : fusion::cons<Head, Tail>
    {
        BOOST_STATIC_CONSTANT(std::size_t, width = Head::width == Tail::width ? Head::width : unknown_width::value);
        BOOST_STATIC_CONSTANT(bool, pure = Head::pure && Tail::pure);

        alternates_list(Head const &head, Tail const &tail)
          : fusion::cons<Head, Tail>(head, tail)
        {
        }
    };

    template<typename Head>
    struct alternates_list<Head, fusion::nil>
      : fusion::cons<Head, fusion::nil>
    {
        BOOST_STATIC_CONSTANT(std::size_t, width = Head::width);
        BOOST_STATIC_CONSTANT(bool, pure = Head::pure);

        alternates_list(Head const &head, fusion::nil const &tail)
          : fusion::cons<Head, fusion::nil>(head, tail)
        {
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // in_alternate
    template<typename Grammar>
    struct in_alternate
      : Grammar
    {
        in_alternate();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef alternates_list<
                typename Grammar::template apply<Expr, alternate_end_xpression, Visitor>::type
              , State
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, alternate_end_xpression(), visitor)
              , state
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_alternate_matcher
    template<typename Grammar>
    struct as_alternate_matcher
      : Grammar
    {
        as_alternate_matcher();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef alternate_matcher<
                typename Grammar::template apply<Expr, State, Visitor>::type
              , typename Visitor::traits_type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, state, visitor)
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_alternate
    template<typename Grammar>
    struct as_alternate
      : as_alternate_matcher<
            proto::transform::reverse_fold_tree<
                typename Grammar::proto_tag
              , in_alternate<typename Grammar::proto_arg0>
              , fusion::nil
            >
        >
    {
        BOOST_MPL_ASSERT((
            is_same<
                typename Grammar::proto_arg0
              , typename Grammar::proto_arg1
            >
        ));
    };

}}}

#endif
