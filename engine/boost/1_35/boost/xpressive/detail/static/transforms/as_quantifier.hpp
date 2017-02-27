///////////////////////////////////////////////////////////////////////////////
// as_quantifier.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_QUANTIFIER_HPP_EAN_04_01_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_QUANTIFIER_HPP_EAN_04_01_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/transform/arg.hpp>
#include <boost/xpressive/proto/transform/compose.hpp>

namespace boost { namespace xpressive { namespace detail
{

    ///////////////////////////////////////////////////////////////////////////////
    // generic_quant_tag
    template<uint_t Min, uint_t Max>
    struct generic_quant_tag
    {
        typedef mpl::integral_c<uint_t, Min> min_type;
        typedef mpl::integral_c<uint_t, Max> max_type;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // min_type / max_type
    template<typename Tag>
    struct min_type : Tag::min_type {};

    template<>
    struct min_type<proto::tag::posit> : mpl::integral_c<uint_t, 1> {};

    template<>
    struct min_type<proto::tag::dereference> : mpl::integral_c<uint_t, 0> {};

    template<>
    struct min_type<proto::tag::logical_not> : mpl::integral_c<uint_t, 0> {};

    template<typename Tag>
    struct max_type : Tag::max_type {};

    template<>
    struct max_type<proto::tag::posit> : mpl::integral_c<uint_t, UINT_MAX-1> {};

    template<>
    struct max_type<proto::tag::dereference> : mpl::integral_c<uint_t, UINT_MAX-1> {};

    template<>
    struct max_type<proto::tag::logical_not> : mpl::integral_c<uint_t, 1> {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_simple_quantifier
    template<typename Grammar, bool Greedy>
    struct as_simple_quantifier
      : Grammar
    {
        typedef proto::transform::arg<Grammar> grammar_type;
        as_simple_quantifier();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef typename grammar_type::template apply<Expr, true_xpression, Visitor>::type xpr_type;
            typedef simple_repeat_matcher<xpr_type, Greedy> matcher_type;
            typedef typename proto::terminal<matcher_type>::type type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            typename apply<Expr, State, Visitor>::xpr_type const &xpr =
                grammar_type::call(expr, true_xpression(), visitor);
            return apply<Expr, State, Visitor>::type::make(
                typename apply<Expr, State, Visitor>::matcher_type(
                    xpr
                  , min_type<typename Expr::proto_tag>::value
                  , max_type<typename Expr::proto_tag>::value
                  , xpr.get_width().value()
                )
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // add_hidden_mark
    template<typename Grammar>
    struct add_hidden_mark
      : Grammar
    {
        add_hidden_mark();

        template<typename Expr, typename, typename>
        struct apply
          : proto::shift_right<
                proto::terminal<mark_begin_matcher>::type
              , typename proto::shift_right<
                    Expr
                  , proto::terminal<mark_end_matcher>::type
                >::type
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            // we're inserting a hidden mark ... so grab the next hidden mark number.
            int mark_nbr = visitor.get_hidden_mark();
            mark_begin_matcher begin(mark_nbr);
            mark_end_matcher end(mark_nbr);

            typename apply<Expr, State, Visitor>::type that = {{begin}, {expr, {end}}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // InsertMark
    struct InsertMark
      : proto::or_<
            proto::assign<basic_mark_tag, proto::_>
          , add_hidden_mark<proto::_>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_default_quantifier_impl
    template<bool Greedy, uint_t Min, uint_t Max>
    struct as_default_quantifier_impl
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : proto::shift_right<
                proto::terminal<repeat_begin_matcher>::type
              , typename proto::shift_right<
                    typename InsertMark::apply<typename proto::result_of::arg<Expr>::type, State, Visitor>::type
                  , typename proto::terminal<repeat_end_matcher<Greedy> >::type
                >::type
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            // Ensure this sub-expression is book-ended with mark matchers
            typedef typename proto::result_of::arg<Expr>::type arg_type;
            typename InsertMark::apply<arg_type, State, Visitor>::type const &
                marked_sub = InsertMark::call(proto::arg(expr), state, visitor);

            // Get the mark_number from the begin_mark_matcher
            int mark_number = proto::arg(proto::left(marked_sub)).mark_number_;
            BOOST_ASSERT(0 != mark_number);

            unsigned min_ = min_type<typename Expr::proto_tag>::value;
            unsigned max_ = max_type<typename Expr::proto_tag>::value;

            repeat_begin_matcher begin(mark_number);
            repeat_end_matcher<Greedy> end(mark_number, min_, max_);

            typename apply<Expr, State, Visitor>::type that = {{begin}, {marked_sub, {end}}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // optional_tag
    template<bool Greedy>
    struct optional_tag
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_default_optional
    template<typename Grammar, bool Greedy>
    struct as_default_optional
      : Grammar
    {
        as_default_optional();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef optional_matcher<
                typename Grammar::template apply<Expr, alternate_end_xpression, Visitor>::type
              , Greedy
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, alternate_end_xpression(), visitor)
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_mark_optional
    template<typename Grammar, bool Greedy>
    struct as_mark_optional
      : Grammar
    {
        as_mark_optional();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef optional_mark_matcher<
                typename Grammar::template apply<Expr, alternate_end_xpression, Visitor>::type
              , Greedy
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            int mark_number = proto::arg(proto::left(expr)).mark_number_;
            return typename apply<Expr, State, Visitor>::type(
                Grammar::call(expr, alternate_end_xpression(), visitor)
              , mark_number
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // IsMarkerOrRepeater
    struct IsMarkerOrRepeater
      : proto::or_<
            proto::shift_right<proto::terminal<repeat_begin_matcher>, proto::_>
          , proto::assign<proto::terminal<mark_placeholder>, proto::_>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_optional
    template<typename Grammar, bool Greedy>
    struct as_optional
      : proto::if_<
            proto::matches<mpl::_, IsMarkerOrRepeater>
          , as_mark_optional<Grammar, Greedy>
          , as_default_optional<Grammar, Greedy>
        >
    {
        as_optional();
    };

    ///////////////////////////////////////////////////////////////////////////////
    // make_optional_
    template<bool Greedy>
    struct make_optional_
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : proto::unary_expr<optional_tag<Greedy>, Expr>
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &)
        {
            typename apply<Expr, State, Visitor>::type that = {expr};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_default_quantifier_impl
    template<bool Greedy, uint_t Max>
    struct as_default_quantifier_impl<Greedy, 0, Max>
      : proto::transform::compose<
            as_default_quantifier_impl<Greedy, 1, Max>
          , make_optional_<Greedy>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_default_quantifier_impl
    template<bool Greedy>
    struct as_default_quantifier_impl<Greedy, 0, 1>
      : proto::transform::compose<
            proto::transform::arg<proto::_>
          , make_optional_<Greedy>
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_default_quantifier
    template<typename Grammar, bool Greedy>
    struct as_default_quantifier
      : Grammar
    {
        as_default_quantifier();

        template<typename Expr, typename State, typename Visitor>
        struct apply
          : as_default_quantifier_impl<
                Greedy
              , min_type<typename Expr::proto_tag>::value
              , max_type<typename Expr::proto_tag>::value
            >::template apply<Expr, State, Visitor>
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            return as_default_quantifier_impl<
                Greedy
              , min_type<typename Expr::proto_tag>::value
              , max_type<typename Expr::proto_tag>::value
            >::call(expr, state, visitor);
        }
    };

}}}

#endif
