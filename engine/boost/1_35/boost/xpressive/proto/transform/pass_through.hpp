#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file pass_through.hpp
    /// TODO
    //
    //  Copyright 2007 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_PASS_THROUGH_HPP_EAN_12_26_2006
    #define BOOST_PROTO_TRANSFORM_PASS_THROUGH_HPP_EAN_12_26_2006

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/enum.hpp>
    #include <boost/preprocessor/iterate.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/args.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto { namespace transform
    {
        namespace detail
        {
            template<typename Grammar, typename Expr, typename State, typename Visitor, long Arity = Expr::proto_arity::value>
            struct pass_through_impl {};

            #define BOOST_PROTO_DEFINE_TRANSFORM_TYPE(z, n, data)\
                typename Grammar::BOOST_PP_CAT(proto_arg, n)\
                    ::template apply<typename Expr::BOOST_PP_CAT(proto_arg, n)::proto_base_expr, State, Visitor>\
                ::type

            #define BOOST_PROTO_DEFINE_TRANSFORM(z, n, data)\
                Grammar::BOOST_PP_CAT(proto_arg, n)::call(\
                    expr.BOOST_PP_CAT(arg, n).proto_base(), state, visitor\
                )

            #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/pass_through.hpp>))
            #include BOOST_PP_ITERATE()

            #undef BOOST_PROTO_DEFINE_TRANSFORM
            #undef BOOST_PROTO_DEFINE_TRANSFORM_TYPE

            template<typename Grammar, typename Expr, typename State, typename Visitor>
            struct pass_through_impl<Grammar, Expr, State, Visitor, 0>
            {
                typedef Expr type;

                static Expr const &call(Expr const &expr, State const &, Visitor &)
                {
                    return expr;
                }
            };
        } // namespace detail

        template<typename Grammar>
        struct pass_through
          : Grammar
        {
            pass_through() {}

            template<typename Expr, typename State, typename Visitor>
            struct apply
              : detail::pass_through_impl<
                    Grammar
                  , typename Expr::proto_base_expr
                  , State
                  , Visitor
                  , Expr::proto_arity::value
                >
            {};

            template<typename Expr, typename State, typename Visitor>
            static typename apply<Expr, State, Visitor>::type
            call(Expr const &expr, State const &state, Visitor &visitor)
            {
                return apply<Expr, State, Visitor>::call(expr.proto_base(), state, visitor);
            }
        };
    } // namespace transform

    template<typename Grammar>
    struct is_transform<transform::pass_through<Grammar> >
      : mpl::true_
    {};

    namespace has_transformns_
    {
        template<typename Grammar>
        struct has_pass_through_transform
        {
            has_pass_through_transform() {}

            template<typename Expr, typename State, typename Visitor>
            struct apply
              : transform::detail::pass_through_impl<
                    Grammar
                  , typename Expr::proto_base_expr
                  , State
                  , Visitor
                  , Expr::proto_arity::value
                >
            {};

            template<typename Expr, typename State, typename Visitor>
            static typename apply<Expr, State, Visitor>::type
            call(Expr const &expr, State const &state, Visitor &visitor)
            {
                return apply<Expr, State, Visitor>::call(expr.proto_base(), state, visitor);
            }
        };

    } // namespace has_transformns_

    }} // namespace boost::proto

    #endif

#else

    #define N BOOST_PP_ITERATION()

            template<typename Grammar, typename Expr, typename State, typename Visitor>
            struct pass_through_impl<Grammar, Expr, State, Visitor, N>
            {
                typedef expr<
                    typename Expr::proto_tag
                  , BOOST_PP_CAT(args, N)<
                        BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_TRANSFORM_TYPE, ~)
                    >
                > type;

                #if BOOST_WORKAROUND(BOOST_MSVC, == 1310)
                template<typename Expr2, typename State2, typename Visitor2>
                static type call(Expr2 const &expr, State2 const &state, Visitor2 &visitor)
                #else
                static type call(Expr const &expr, State const &state, Visitor &visitor)
                #endif
                {
                    type that = {
                        BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_TRANSFORM, ~)
                    };
                    #if BOOST_WORKAROUND(BOOST_MSVC, BOOST_TESTED_AT(1400))
                    // Without this, MSVC complains that "that" is uninitialized,
                    // and it actually triggers a runtime check in debug mode when
                    // built with VC8.
                    &that;
                    #endif
                    return that;
                }
            };

    #undef N

#endif
