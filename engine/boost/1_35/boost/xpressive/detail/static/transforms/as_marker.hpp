///////////////////////////////////////////////////////////////////////////////
// as_marker.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MARKER_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_MARKER_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/proto.hpp>

//#include <boost/xpressive/proto/transform/construct.hpp>

namespace boost { namespace xpressive { namespace detail
{

    //template<typename Grammar>
    //struct as_marker
    //  : proto::transform::construct<
    //        proto::transform::identity<Grammar>
    //      , proto::shift_right<
    //            proto::terminal<mark_begin_matcher>::type
    //          , proto::shift_right<
    //                proto::transform::right<proto::_>
    //              , proto::terminal<mark_end_matcher>::type
    //            >
    //        >(
    //            proto::terminal<mark_begin_matcher>::type(
    //                mark_begin_matcher(proto::transform::arg<proto::transform::left<proto::_> > )
    //            )
    //          , proto::shift_right<
    //                proto::transform::right<proto::_>
    //              , proto::terminal<mark_end_matcher>::type
    //            >(
    //                proto::transform::right<proto::_>
    //              , proto::terminal<mark_end_matcher>::type(
    //                    mark_end_matcher(proto::transform::arg<proto::transform::left<proto::_> > )
    //                )
    //            )
    //        )
    //    >
    //{};

    ///////////////////////////////////////////////////////////////////////////////
    // as_marker
    //   Insert mark tags before and after the expression
    template<typename Grammar>
    struct as_marker
      : Grammar
    {
        as_marker();

        template<typename Expr, typename, typename>
        struct apply
          : proto::shift_right<
                proto::terminal<mark_begin_matcher>::type
              , typename proto::shift_right<
                    typename proto::result_of::right<Expr>::type
                  , proto::terminal<mark_end_matcher>::type
                >::type
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &)
        {
            int mark_nbr = get_mark_number(proto::left(expr));
            mark_begin_matcher begin(mark_nbr);
            mark_end_matcher end(mark_nbr);

            typename apply<Expr, State, Visitor>::type that = {{begin}, {proto::right(expr), {end}}};
            return that;
        }
    };

}}}

#endif
