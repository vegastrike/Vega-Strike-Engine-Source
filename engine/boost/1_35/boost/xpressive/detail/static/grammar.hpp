///////////////////////////////////////////////////////////////////////////////
// grammar.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_GRAMMAR_HPP_EAN_11_12_2006
#define BOOST_XPRESSIVE_DETAIL_STATIC_GRAMMAR_HPP_EAN_11_12_2006

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/xpressive/detail/static/is_pure.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform/arg.hpp>
#include <boost/xpressive/proto/transform/fold.hpp>
#include <boost/xpressive/proto/transform/compose.hpp>
#include <boost/xpressive/detail/static/transforms/as_matcher.hpp>
#include <boost/xpressive/detail/static/transforms/as_alternate.hpp>
#include <boost/xpressive/detail/static/transforms/as_sequence.hpp>
#include <boost/xpressive/detail/static/transforms/as_quantifier.hpp>
#include <boost/xpressive/detail/static/transforms/as_marker.hpp>
#include <boost/xpressive/detail/static/transforms/as_set.hpp>
#include <boost/xpressive/detail/static/transforms/as_independent.hpp>
#include <boost/xpressive/detail/static/transforms/as_modifier.hpp>
#include <boost/xpressive/detail/static/transforms/as_inverse.hpp>
#include <boost/xpressive/detail/static/transforms/as_action.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>

#define BOOST_XPRESSIVE_CHECK_REGEX(Expr, Char)\
    BOOST_MPL_ASSERT\
    ((\
        typename boost::mpl::if_<\
            boost::xpressive::is_valid_regex<Expr, Char>\
          , boost::mpl::true_\
          , boost::xpressive::INVALID_REGULAR_EXPRESSION\
        >::type\
    ));

//////////////////////////////////////////////////////////////////////////
//**********************************************************************//
//*                            << NOTE! >>                             *//
//*                                                                    *//
//* Whenever you change this grammar, you MUST also make corresponding *//
//* changes to width_of.hpp and is_pure.hpp.                           *//
//*                                                                    *//
//**********************************************************************//
//////////////////////////////////////////////////////////////////////////

namespace boost { namespace xpressive
{
    template<typename Char>
    struct Grammar;

    template<typename Char>
    struct ActionableGrammar;

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////////
        // CharLiteral
        template<typename Char>
        struct CharLiteral;

        ///////////////////////////////////////////////////////////////////////////
        // ListSet
        template<typename Char>
        struct ListSet;

        ///////////////////////////////////////////////////////////////////////////
        // as_repeat
        template<typename Char, typename Gram, typename Tag, bool Greedy>
        struct as_repeat
          : proto::if_<
                use_simple_repeat<proto::result_of::arg<mpl::_>, Char>
              , as_simple_quantifier<proto::unary_expr<Tag, Gram>, Greedy>
              , as_default_quantifier<proto::unary_expr<Tag, Gram>, Greedy>
            >
        {};

        ///////////////////////////////////////////////////////////////////////////
        // NonGreedyRepeatCases
        template<typename Char, typename Gram>
        struct NonGreedyRepeatCases
        {
            template<typename Tag, typename Dummy = void>
            struct case_
              : proto::not_<proto::_>
            {};

            template<typename Dummy>
            struct case_<proto::tag::dereference, Dummy>
              : as_repeat<Char, Gram, proto::tag::dereference, false>
            {};

            template<typename Dummy>
            struct case_<proto::tag::posit, Dummy>
              : as_repeat<Char, Gram, proto::tag::posit, false>
            {};

            template<typename Dummy>
            struct case_<proto::tag::logical_not, Dummy>
              : as_repeat<Char, Gram, proto::tag::logical_not, false>
            {};

            template<uint_t Min, uint_t Max, typename Dummy>
            struct case_<generic_quant_tag<Min, Max>, Dummy>
              : as_repeat<Char, Gram, generic_quant_tag<Min, Max>, false>
            {};
        };

        ///////////////////////////////////////////////////////////////////////////
        // InvertibleCases
        template<typename Char, typename Gram>
        struct InvertibleCases
        {
            template<typename Tag, typename Dummy = void>
            struct case_
              : proto::not_<proto::_>
            {};

            template<typename Dummy>
            struct case_<proto::tag::comma, Dummy>
              : as_list_set<ListSet<Char> >
            {};

            template<typename Dummy>
            struct case_<proto::tag::assign, Dummy>
              : as_list_set<ListSet<Char> >
            {};

            template<typename Dummy>
            struct case_<proto::tag::subscript, Dummy>
              : proto::transform::right<proto::subscript<set_initializer_type, as_set<Gram> > >
            {};

            template<typename Dummy>
            struct case_<lookahead_tag, Dummy>
              : proto::transform::arg<proto::unary_expr<lookahead_tag, as_lookahead<Gram> > >
            {};

            template<typename Dummy>
            struct case_<lookbehind_tag, Dummy>
              : proto::transform::arg<proto::unary_expr<lookbehind_tag, as_lookbehind<Gram> > >
            {};

            template<typename Dummy>
            struct case_<proto::tag::terminal, Dummy>
              : proto::or_<
                    as_matcher<CharLiteral<Char> >
                  , as_matcher<proto::terminal<posix_charset_placeholder> >
                  , as_matcher<proto::terminal<range_placeholder<proto::_> > >
                  , as_matcher<proto::terminal<logical_newline_placeholder> >
                  , as_matcher<proto::terminal<assert_word_placeholder<word_boundary<true> > > >
                >
            {};
        };

        ///////////////////////////////////////////////////////////////////////////
        // Cases
        template<typename Char, typename Gram>
        struct Cases
        {
            template<typename Tag, typename Dummy = void>
            struct case_
              : proto::not_<proto::_>
            {};

            template<typename Dummy>
            struct case_<proto::tag::shift_right, Dummy>
              : proto::transform::reverse_fold<proto::shift_right<Gram, Gram> >
            {};

            template<typename Dummy>
            struct case_<proto::tag::terminal, Dummy>
              : in_sequence<as_matcher<proto::terminal<proto::_> > >
            {};

            template<typename Dummy>
            struct case_<proto::tag::bitwise_or, Dummy>
              : in_sequence<as_alternate<proto::bitwise_or<Gram, Gram> > >
            {};

            template<typename Dummy, bool Greedy>
            struct case_<optional_tag<Greedy> , Dummy>
              : in_sequence<proto::transform::arg<proto::unary_expr<optional_tag<Greedy>, as_optional<Gram, Greedy> > > >
            {};

            template<typename Dummy>
            struct case_<proto::tag::dereference, Dummy>
              : proto::transform::compose<as_repeat<Char, Gram, proto::tag::dereference, true>, Gram>
            {};

            template<typename Dummy>
            struct case_<proto::tag::posit, Dummy>
              : proto::transform::compose<as_repeat<Char, Gram, proto::tag::posit, true>, Gram>
            {};

            template<typename Dummy>
            struct case_<proto::tag::logical_not, Dummy>
              : proto::transform::compose<as_repeat<Char, Gram, proto::tag::logical_not, true>, Gram>
            {};

            template<uint_t Min, uint_t Max, typename Dummy>
            struct case_<generic_quant_tag<Min, Max> , Dummy>
              : proto::transform::compose<as_repeat<Char, Gram, generic_quant_tag<Min, Max>, true>, Gram>
            {};

            template<typename Dummy>
            struct case_<proto::tag::negate, Dummy>
              : proto::transform::compose<
                    proto::transform::arg<proto::negate<proto::switch_<NonGreedyRepeatCases<Char, Gram> > > >
                  , Gram
                >
            {};

            template<typename Dummy>
            struct case_<proto::tag::complement, Dummy>
              : in_sequence<as_inverse<
                    proto::transform::arg<proto::complement<proto::switch_<InvertibleCases<Char, Gram> > > >
                > >
            {};

            template<typename Dummy>
            struct case_<modifier_tag, Dummy>
              : as_modifier<proto::binary_expr<modifier_tag, proto::_, Gram> >
            {};

            template<typename Dummy>
            struct case_<lookahead_tag, Dummy>
              : in_sequence<proto::transform::arg<proto::unary_expr<lookahead_tag, as_lookahead<Gram> > > >
            {};

            template<typename Dummy>
            struct case_<lookbehind_tag, Dummy>
              : in_sequence<proto::transform::arg<proto::unary_expr<lookbehind_tag, as_lookbehind<Gram> > > >
            {};

            template<typename Dummy>
            struct case_<keeper_tag, Dummy>
              : in_sequence<proto::transform::arg<proto::unary_expr<keeper_tag, as_keeper<Gram> > > >
            {};

            template<typename Dummy>
            struct case_<proto::tag::comma, Dummy>
              : in_sequence<as_list_set<ListSet<Char> > >
            {};

            template<typename Dummy>
            struct case_<proto::tag::assign, Dummy>
              : proto::or_<
                    proto::transform::compose<as_marker<proto::assign<basic_mark_tag, Gram> >, Gram>
                  , in_sequence<as_list_set<ListSet<Char> > >
                >
            {};

            template<typename Dummy>
            struct case_<proto::tag::subscript, Dummy>
              : proto::or_<
                    in_sequence<proto::transform::right<proto::subscript<set_initializer_type, as_set<Gram> > > >
                  , proto::transform::compose<as_action<proto::subscript<ActionableGrammar<Char>, proto::_> >, ActionableGrammar<Char> >
                >
            {};
        };

        ///////////////////////////////////////////////////////////////////////////
        // ActionableCases
        template<typename Char, typename Gram>
        struct ActionableCases
        {
            template<typename Tag, typename Dummy = void>
            struct case_
              : Cases<Char, Gram>::template case_<Tag>
            {};

            // Only in sub-expressions with actions attached do we allow attribute assignements
            template<typename Dummy>
            struct case_<proto::tag::assign, Dummy>
              : proto::or_<
                    typename Cases<Char, Gram>::template case_<proto::tag::assign>
                  , in_sequence<as_attr_matcher<proto::assign<proto::terminal<attribute_placeholder<proto::_> >, proto::_> > >
                >
            {};
        };

    } // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    // Grammar
    template<typename Char>
    struct Grammar
      : proto::switch_<detail::Cases<Char, Grammar<Char> > >
    {};

    template<typename Char>
    struct ActionableGrammar
      : proto::switch_<detail::ActionableCases<Char, ActionableGrammar<Char> > >
    {};

    ///////////////////////////////////////////////////////////////////////////
    // INVALID_REGULAR_EXPRESSION
    struct INVALID_REGULAR_EXPRESSION
      : mpl::false_
    {};

    ///////////////////////////////////////////////////////////////////////////
    // is_valid_regex
    template<typename Expr, typename Char>
    struct is_valid_regex
      : proto::matches<Expr, Grammar<Char> >
    {};

}} // namespace boost::xpressive

#endif
