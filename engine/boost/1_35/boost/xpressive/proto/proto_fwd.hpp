///////////////////////////////////////////////////////////////////////////////
/// \file proto_fwd.hpp
/// Forward declarations of all of proto's public types and functions.
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_FWD_HPP_EAN_04_01_2005
#define BOOST_PROTO_FWD_HPP_EAN_04_01_2005

#include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
#include <cstddef>
#include <climits>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/mpl/long.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>

#ifndef BOOST_PROTO_MAX_ARITY
# define BOOST_PROTO_MAX_ARITY 5
#endif

#ifndef BOOST_PROTO_MAX_LOGICAL_ARITY
# define BOOST_PROTO_MAX_LOGICAL_ARITY 8
#endif

#if BOOST_WORKAROUND(__GNUC__, == 3) \
 || BOOST_WORKAROUND(__EDG_VERSION__, BOOST_TESTED_AT(306))
# define BOOST_PROTO_BROKEN_CONST_OVERLOADS
#endif

#ifdef BOOST_PROTO_BROKEN_CONST_OVERLOADS
# include <boost/utility/enable_if.hpp>
# include <boost/type_traits/is_const.hpp>
# define BOOST_PROTO_DISABLE_IF_IS_CONST(T)\
    , typename boost::disable_if<boost::is_const<T> >::type * = 0
#else
# define BOOST_PROTO_DISABLE_IF_IS_CONST(T)
#endif

#include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

namespace boost { namespace proto
{
    namespace detail
    {
        typedef char yes_type;
        typedef char (&no_type)[2];

        struct dont_care
        {
            dont_care(...);
        };

        template<typename T>
        struct remove_cv_ref
          : remove_cv<typename remove_reference<T>::type>
        {};
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Operator tags
    namespace tag
    {
        struct terminal;
        struct posit;
        struct negate;
        struct dereference;
        struct complement;
        struct address_of;
        struct logical_not;
        struct pre_inc;
        struct pre_dec;
        struct post_inc;
        struct post_dec;

        struct shift_left;
        struct shift_right;
        struct multiplies;
        struct divides;
        struct modulus;
        struct plus;
        struct minus;
        struct less;
        struct greater;
        struct less_equal;
        struct greater_equal;
        struct equal_to;
        struct not_equal_to;
        struct logical_or;
        struct logical_and;
        struct bitwise_and;
        struct bitwise_or;
        struct bitwise_xor;
        struct comma;
        struct mem_ptr;

        struct assign;
        struct shift_left_assign;
        struct shift_right_assign;
        struct multiplies_assign;
        struct divides_assign;
        struct modulus_assign;
        struct plus_assign;
        struct minus_assign;
        struct bitwise_and_assign;
        struct bitwise_or_assign;
        struct bitwise_xor_assign;
        struct subscript;
        struct if_else_;
        struct function;

        // Fusion tags
        struct proto_expr;
        struct proto_ref;
        struct proto_ref_iterator;
    }

    namespace wildcardns_
    {
        struct _;
    }

    using wildcardns_::_;

    namespace generatorns_
    {
        struct default_generator;

        template<template<typename> class Extends>
        struct generator;

        template<template<typename> class Extends>
        struct pod_generator;

        template<typename Generator = default_generator>
        struct by_value_generator;
    }

    using generatorns_::default_generator;
    using generatorns_::generator;
    using generatorns_::pod_generator;
    using generatorns_::by_value_generator;

    namespace domainns_
    {
        template<typename Generator = default_generator, typename Grammar = proto::_>
        struct domain;

        struct default_domain;

        struct deduce_domain;
    }

    using domainns_::domain;
    using domainns_::default_domain;
    using domainns_::deduce_domain;

    namespace exprns_
    {
        template<typename Tag, typename Args, long Arity = Args::size>
        struct expr;

        template<
            typename Expr
          , typename Derived
          , typename Domain = default_domain
          , typename Tag = typename Expr::proto_tag
        >
        struct extends;

        struct is_proto_expr;
    }

    using exprns_::expr;
    using exprns_::extends;
    using exprns_::is_proto_expr;

    namespace refns_
    {
        template<typename Expr>
        struct ref_;
    }

    using refns_::ref_;

    namespace control
    {
        template<
            typename Grammar0
          , typename Grammar1
          , BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(BOOST_PP_SUB(BOOST_PROTO_MAX_LOGICAL_ARITY,2), typename G, void)
        >
        struct or_;

        template<
            typename Grammar0
          , typename Grammar1
          , BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(BOOST_PP_SUB(BOOST_PROTO_MAX_LOGICAL_ARITY,2), typename G, void)
        >
        struct and_;

        template<typename Condition, typename Then = void, typename Else = void>
        struct if_;

        template<typename Cases>
        struct switch_;

        template<typename Grammar>
        struct not_;

        template<typename T>
        struct exact;

        template<typename T>
        struct convertible_to;

        template<typename Grammar>
        struct vararg;

        int const N = INT_MAX;
    }

    using control::if_;
    using control::or_;
    using control::and_;
    using control::not_;
    using control::switch_;
    using control::exact;
    using control::convertible_to;
    using control::vararg;
    using control::N;

    namespace context
    {
        struct null_context;

        template<typename Expr, typename Context, long Arity = Expr::proto_arity::value>
        struct null_eval;

        struct default_context;

        template<typename Expr, typename Context, typename Tag = typename Expr::proto_tag, long Arity = Expr::proto_arity::value>
        struct default_eval;

        template<typename Derived, typename DefaultCtx = default_context>
        struct callable_context;

        template<typename Expr, typename Context, long Arity = Expr::proto_arity::value>
        struct callable_eval;
    }

    using context::null_context;
    using context::null_eval;
    using context::default_context;
    using context::default_eval;
    using context::callable_context;
    using context::callable_eval;

    namespace utility
    {
        template<typename T, typename Domain = default_domain>
        struct literal;
    }

    using utility::literal;

    namespace result_of
    {
        template<typename T, typename Domain = default_domain, typename EnableIf = void>
        struct as_expr;

        template<typename T, typename Domain = default_domain, typename EnableIf = void>
        struct as_arg;

        template<typename Expr, typename N = mpl::long_<0> >
        struct arg;

        template<typename Expr, long N>
        struct arg_c;

        template<typename Expr>
        struct left;

        template<typename Expr>
        struct right;

        template<typename Expr>
        struct deep_copy;

        template<typename T>
        struct unref;

        template<typename Expr, typename Context>
        struct eval;

        template<
            typename Tag
            BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(
                BOOST_PROTO_MAX_ARITY
              , typename A
              , = void BOOST_PP_INTERCEPT
            )
          , typename _1 = void
          , typename _2 = void
        >
        struct make_expr;

        template<typename Tag, typename DomainOrSequence, typename SequenceOrVoid = void, typename _ = void>
        struct unpack_expr;

        template<typename T, typename EnableIf = void>
        struct is_ref;

        template<typename T, typename EnableIf = void>
        struct is_expr;

        template<typename T, typename EnableIf = void>
        struct is_domain;

        template<typename Expr>
        struct tag_of;

        template<typename T, typename EnableIf = void>
        struct domain_of;

        template<typename Expr>
        struct id;

        template<typename Expr, typename Grammar>
        struct matches;
    }

    using proto::result_of::is_ref;
    using proto::result_of::is_expr;
    using proto::result_of::is_domain;
    using proto::result_of::tag_of;
    using proto::result_of::domain_of;
    using proto::result_of::id;
    using proto::result_of::matches;

    namespace op
    {
        // Generic expression generators
        template<typename Tag, typename Arg>
        struct unary_expr;

        template<typename Tag, typename Left, typename Right>
        struct binary_expr;

        template<typename Tag, BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(BOOST_PROTO_MAX_ARITY, typename A, void), typename Dummy = void>
        struct nary_expr;

        // Specific expression generators, for convenience
        template<typename T> struct terminal;
        template<typename T> struct posit;
        template<typename T> struct negate;
        template<typename T> struct dereference;
        template<typename T> struct complement;
        template<typename T> struct address_of;
        template<typename T> struct logical_not;
        template<typename T> struct pre_inc;
        template<typename T> struct pre_dec;
        template<typename T> struct post_inc;
        template<typename T> struct post_dec;

        template<typename T, typename U> struct shift_left;
        template<typename T, typename U> struct shift_right;
        template<typename T, typename U> struct multiplies;
        template<typename T, typename U> struct divides;
        template<typename T, typename U> struct modulus;
        template<typename T, typename U> struct plus;
        template<typename T, typename U> struct minus;
        template<typename T, typename U> struct less;
        template<typename T, typename U> struct greater;
        template<typename T, typename U> struct less_equal;
        template<typename T, typename U> struct greater_equal;
        template<typename T, typename U> struct equal_to;
        template<typename T, typename U> struct not_equal_to;
        template<typename T, typename U> struct logical_or;
        template<typename T, typename U> struct logical_and;
        template<typename T, typename U> struct bitwise_and;
        template<typename T, typename U> struct bitwise_or;
        template<typename T, typename U> struct bitwise_xor;
        template<typename T, typename U> struct comma;
        template<typename T, typename U> struct mem_ptr;

        template<typename T, typename U> struct assign;
        template<typename T, typename U> struct shift_left_assign;
        template<typename T, typename U> struct shift_right_assign;
        template<typename T, typename U> struct multiplies_assign;
        template<typename T, typename U> struct divides_assign;
        template<typename T, typename U> struct modulus_assign;
        template<typename T, typename U> struct plus_assign;
        template<typename T, typename U> struct minus_assign;
        template<typename T, typename U> struct bitwise_and_assign;
        template<typename T, typename U> struct bitwise_or_assign;
        template<typename T, typename U> struct bitwise_xor_assign;
        template<typename T, typename U> struct subscript;
        template<typename T, typename U, typename V> struct if_else_;

        template<BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(BOOST_PROTO_MAX_ARITY, typename A, void), typename Dummy = void>
        struct function;
    }

    using namespace op;

    namespace functional
    {
        struct left;
        struct right;
        struct unref;
        struct deep_copy;

        template<typename Domain = default_domain>
        struct as_expr;

        template<typename Domain = default_domain>
        struct as_arg;

        template<typename N = mpl::long_<0> >
        struct arg;

        template<long N>
        struct arg_c;

        template<typename Tag, typename Domain = deduce_domain>
        struct make_expr;

        template<typename Tag, typename Domain = deduce_domain>
        struct unpack_expr;

        template<typename Tag, typename Domain = deduce_domain>
        struct unfused_expr_fun;

        template<typename Tag, typename Domain = deduce_domain>
        struct unfused_expr;
    }

#define BOOST_PROTO_IDENTITY_TRANSFORM()\
    template<typename Expr_, typename State_, typename Visitor_>\
    static Expr_ const &call(Expr_ const &expr_, State_ const &, Visitor_ &)\
    {\
        return expr_;\
    }\
    template<typename Expr_, typename, typename>\
    struct apply\
    {\
        typedef Expr_ type;\
    }

    namespace transform
    {
        namespace detail
        {
            using proto::detail::yes_type;
            using proto::detail::no_type;
            
            struct default_factory;

            no_type is_wildcard_expression_fun(...);

            template<typename T>
            struct is_wildcard_expression;
        }

        template<typename Grammar, typename N = mpl::long_<0> >
        struct arg;

        template<typename Grammar, long N>
        struct arg_c;

        template<typename Grammar>
        struct left;

        template<typename Grammar>
        struct right;

        template<typename Grammar>
        struct state;

        template<typename Grammar>
        struct visitor;

        template<typename Grammar>
        struct identity;

        template<typename Grammar, typename Always, typename Factory = detail::default_factory>
        struct always;

        template<typename Grammar, typename Lambda, typename Factory = detail::default_factory>
        struct apply1;

        template<typename Grammar, typename Lambda, typename Factory = detail::default_factory>
        struct apply2;

        template<typename Grammar, typename Lambda, typename Factory = detail::default_factory>
        struct apply3;

        template<typename Grammar, typename State>
        struct branch;

        template<typename Grammar, typename State = void>
        struct fold;

        template<typename Grammar, typename State = void>
        struct reverse_fold;

        template<typename Tag, typename Grammar, typename State = void>
        struct fold_tree;

        template<typename Tag, typename Grammar, typename State = void>
        struct reverse_fold_tree;

        template<typename Grammar, typename Function1>
        struct function1;

        template<typename Grammar, typename Function2>
        struct function2;

        template<typename Grammar, typename Function3>
        struct function3;

        template<typename Grammar>
        struct list;

        template<typename Grammar>
        struct tail;

        template<typename Grammar>
        struct pass_through;

        template<typename Grammar, typename ConstructorFun>
        struct construct;

        template<typename Grammar, typename ConstructorFun>
        struct pod_construct;
    }

    namespace has_transformns_
    {
        template<typename Grammar>
        struct has_pass_through_transform;

        struct has_identity_transform
        {
            BOOST_PROTO_IDENTITY_TRANSFORM();
        };
    }

    using has_transformns_::has_identity_transform;
    using has_transformns_::has_pass_through_transform;

    template<typename T>
    struct is_transform;

    template<typename T>
    struct is_extension;

    namespace exops
    {}

}} // namespace boost::proto

#endif
