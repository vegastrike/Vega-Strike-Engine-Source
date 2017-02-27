///////////////////////////////////////////////////////////////////////////////
// as_action.hpp
//
//  Copyright 2007 Eric Niebler.
//  Copyright 2007 David Jenkins.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ACTION_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_ACTION_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/min_max.hpp>
#include <boost/mpl/apply_wrap.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/core/matcher/attr_end_matcher.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/detail/static/transforms/as_quantifier.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform/arg.hpp>
#include <boost/xpressive/proto/transform/fold.hpp>

namespace boost { namespace xpressive { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////////
    // FindAttr
    //  Look for patterns like (a1= terminal<RHS>) and return the type of the RHS.
    template<typename Nbr>
    struct FindAttr
      : proto::or_<
            proto::transform::state< proto::terminal<proto::_> >
            // Ignore nested actions, because attributes are scoped:
          , proto::transform::state< proto::subscript<proto::_, proto::_> >
          , proto::transform::arg<
                proto::transform::right<
                    proto::assign<
                        proto::terminal<xpressive::detail::attribute_placeholder<Nbr> >
                      , proto::_
                    >
                >
            >
          , proto::transform::fold<proto::nary_expr<proto::_, proto::vararg<FindAttr<Nbr> > > >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // by_value
    //  Store all terminals within an action by value to avoid dangling references.
    template<typename Terminal>
    struct by_value
      : Terminal
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : proto::result_of::as_expr<typename proto::result_of::arg<Expr>::type>
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &)
        {
            return proto::as_expr(proto::arg(expr));
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // read_attr
    //  Placeholder that knows the slot number of an attribute as well as the type
    //  of the object stored in it.
    template<typename Nbr, typename Matcher>
    struct read_attr
    {
        typedef Nbr nbr_type;
        typedef Matcher matcher_type;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_read_attr
    //  For patterns like (a1 = RHS)[ref(i) = a1], transform to
    //  (a1 = RHS)[ref(i) = read_attr<1, RHS>] so that when reading the attribute
    //  we know what type is stored in the attribute slot.
    template<typename Grammar>
    struct as_read_attr
      : Grammar
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : proto::result_of::as_expr<
                read_attr<
                    typename Expr::proto_arg0::nbr_type
                  , typename FindAttr<typename Expr::proto_arg0::nbr_type>
                        ::template apply<State, mpl::void_, int>::type
                >
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &, State const &, Visitor &)
        {
            typename apply<Expr, State, Visitor>::type that = {{}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // DeepCopy
    //  Turn all refs into values, and also bind all attribute placeholders with
    //  the types from which they are being assigned.
    struct DeepCopy
      : proto::or_<
            as_read_attr<proto::terminal<xpressive::detail::attribute_placeholder<proto::_> > >
          , by_value<proto::terminal<proto::_> >
          , proto::nary_expr<proto::_, proto::vararg<DeepCopy> >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // max_attr
    //  Take the maximum of the current attr slot number and the state.
    template<typename Grammar>
    struct max_attr
      : Grammar
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : mpl::max<State, typename Grammar::template apply<Expr, State, Visitor>::type>
        {};
    };

    ///////////////////////////////////////////////////////////////////////////////
    // attr_nbr
    //  For an attribute placeholder, return the attribute's slot number.
    template<typename Grammar>
    struct attr_nbr
      : Grammar
    {
        template<typename Expr, typename State, typename Visitor>
        struct apply
          : Expr::proto_arg0::nbr_type
        {};
    };

    ///////////////////////////////////////////////////////////////////////////////
    // MaxAttr
    //  In an action (rx)[act], find the largest attribute slot being used.
    struct MaxAttr
      : proto::or_<
            attr_nbr< proto::terminal< xpressive::detail::attribute_placeholder<proto::_> > >
          , proto::transform::state< proto::terminal<proto::_> >
            // Ignore nested actions, because attributes are scoped:
          , proto::transform::state< proto::subscript<proto::_, proto::_> >
          , proto::transform::fold<proto::nary_expr<proto::_, max_attr<proto::vararg<MaxAttr> > > >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // as_attr_matcher
    //  turn a1=matcher into attr_matcher<Matcher>(1)
    template<typename Grammar>
    struct as_attr_matcher
      : Grammar
    {
        as_attr_matcher();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef
                attr_matcher<
                    typename proto::result_of::arg<typename Expr::proto_arg1>::type
                  , typename Visitor::traits_type
                  , Visitor::icase_type::value
                >
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            return typename apply<Expr, State, Visitor>::type(
                Expr::proto_arg0::proto_base_expr::proto_arg0::nbr_type::value
              , proto::arg(proto::right(expr))
              , visitor.traits()
            );
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // add_attrs
    //  Wrap an expression in attr_begin_matcher/attr_end_matcher pair
    template<typename Grammar>
    struct add_attrs
      : Grammar
    {
        add_attrs();

        template<typename Expr, typename, typename>
        struct apply
          : proto::shift_right<
                typename proto::terminal<
                    attr_begin_matcher<typename MaxAttr::apply<Expr, mpl::int_<0>, int>::type>
                >::type
              , typename proto::shift_right<
                    Expr
                  , proto::terminal<attr_end_matcher>::type
                >::type
            >
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &)
        {
            attr_begin_matcher<typename MaxAttr::apply<Expr, mpl::int_<0>, int>::type> begin;
            attr_end_matcher end;
            typename apply<Expr, State, Visitor>::type that = {{begin}, {expr, {end}}};
            return that;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // InsertAttrs
    struct InsertAttrs
      : proto::or_<
            add_attrs<proto::if_<mpl::apply_wrap3<MaxAttr, mpl::_, mpl::int_<0>, int> > >
          , proto::_
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // CheckAssertion
    struct CheckAssertion
      : proto::function<
            proto::terminal<check_tag>
          , proto::_
        >
    {};

    ///////////////////////////////////////////////////////////////////////////////
    // action_transform
    //  Turn A[B] into (mark_begin(n) >> A >> mark_end(n) >> action_matcher<B>(n))
    //  If A and B use attributes, wrap the above expression in
    //  a attr_begin_matcher<Count> / attr_end_matcher pair, where Count is
    //  the number of attribute slots used by the pattern/action.
    template<typename Grammar>
    struct as_action
      : Grammar
    {
        as_action();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef typename proto::result_of::left<Expr>::type expr_type;
            typedef typename proto::result_of::right<Expr>::type action_type;
            typedef typename DeepCopy::apply<action_type, expr_type, int>::type action_copy_type;

            typedef
                typename InsertMark::apply<expr_type, State, Visitor>::type
            marked_expr_type;

            typedef
                typename mpl::if_<
                    proto::matches<action_type, CheckAssertion>
                  , predicate_matcher<action_copy_type>
                  , action_matcher<action_copy_type>
                >::type
            matcher_type;

            typedef
                typename proto::shift_right<
                    marked_expr_type
                  , typename proto::terminal<matcher_type>::type
                >::type
            no_attr_type;

            typedef
                typename InsertAttrs::apply<no_attr_type, State, Visitor>::type
            type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            typedef apply<Expr, State, Visitor> apply_type;
            typedef typename apply_type::matcher_type matcher_type;

            int dummy = 0;
            typename apply_type::marked_expr_type marked_expr =
                InsertMark::call(proto::left(expr), state, visitor);

            typename apply_type::no_attr_type that =
            {
                marked_expr
              , {
                    matcher_type
                    (
                        DeepCopy::call(proto::right(expr), proto::left(expr), dummy)
                      , proto::arg(proto::left(marked_expr)).mark_number_
                    )
                }
            };

            return InsertAttrs::call(that, state, visitor);
        }
    };

}}}

#endif
