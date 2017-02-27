#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file callable.hpp
    /// Definintion of callable_context\<\>, an evaluation context for
    /// proto::eval() that explodes each node and calls the derived context
    /// type with the expressions constituents. If the derived context doesn't
    /// have an overload that handles this node, fall-back to the default_context.
    /// TODO: make the fall-back configurable!
    //
    //  Copyright 2007 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_CONTEXT_CALLABLE_HPP_EAN_06_23_2007
    #define BOOST_PROTO_CONTEXT_CALLABLE_HPP_EAN_06_23_2007

    #include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/selection/max.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/mpl/bool.hpp>
    #include <boost/utility/result_of.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp> // for arg_c
    #include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

    namespace boost { namespace proto
    {
        namespace detail
        {
            struct private_type_
            {
                private_type_ const &operator,(int) const;
            };

            template<typename T>
            yes_type check_is_expr_handled(T const &);

            no_type check_is_expr_handled(private_type_ const &);

            template<typename Context, long Arity>
            struct callable_context_wrapper;

            template<typename Expr, typename Context, long Arity = Expr::proto_arity::value>
            struct is_expr_handled;
        }

        namespace context
        {
            /// callable_eval
            ///
            template<typename Expr, typename Context, long Arity>
            struct callable_eval
            {};

            /// callable_context
            ///
            template<typename Context, typename DefaultCtx>
            struct callable_context
            {
                /// callable_context::eval
                ///
                template<typename Expr, typename ThisContext = Context>
                struct eval
                  : mpl::if_<
                        detail::is_expr_handled<Expr, Context>
                      , callable_eval<Expr, ThisContext>
                      , typename DefaultCtx::template eval<Expr, Context>
                    >::type
                {};
            };
        }

    #define BOOST_PROTO_ARG_N_TYPE(Z, N, Expr)                                                      \
        typename proto::result_of::arg_c<Expr, N>::const_reference                                  \
        /**/

    #define BOOST_PROTO_ARG_N(Z, N, expr)                                                           \
        proto::arg_c<N>(expr)                                                                       \
        /**/

    #define BOOST_PP_ITERATION_PARAMS_1                                                             \
        (3, (0, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/context/callable.hpp>))               \
        /**/

    #include BOOST_PP_ITERATE()

    #undef BOOST_PROTO_ARG_N_TYPE
    #undef BOOST_PROTO_ARG_N

    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()
    #define ARG_COUNT BOOST_PP_MAX(1, N)

        namespace detail
        {
            #if N > 0
            template<typename Context>
            struct callable_context_wrapper<Context, N>
              : remove_cv<Context>::type
            {
                callable_context_wrapper();
                typedef private_type_ const &(*pointer_to_function)(BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(ARG_COUNT), detail::dont_care BOOST_PP_INTERCEPT));
                operator pointer_to_function() const;
            };
            #endif

            template<typename Expr, typename Context>
            struct is_expr_handled<Expr, Context, N>
            {
                static callable_context_wrapper<Context, ARG_COUNT> &sctx_;
                static Expr &sexpr_;

                BOOST_STATIC_CONSTANT(bool, value =
                (
                    sizeof(yes_type) ==
                    sizeof(
                        detail::check_is_expr_handled(
                            (sctx_(
                                typename Expr::proto_tag()
                                BOOST_PP_ENUM_TRAILING(ARG_COUNT, BOOST_PROTO_ARG_N, sexpr_)
                            ), 0)
                        )
                )));

                typedef mpl::bool_<value> type;
            };
        }

        namespace context
        {
            template<typename Expr, typename Context>
            struct callable_eval<Expr, Context, N>
            {
                typedef
                    typename boost::result_of<
                        Context(
                            typename Expr::proto_tag
                            BOOST_PP_ENUM_TRAILING(ARG_COUNT, BOOST_PROTO_ARG_N_TYPE, Expr)
                        )
                    >::type
                result_type;

                result_type operator ()(Expr &expr, Context &context) const
                {
                    return context(
                        typename Expr::proto_tag()
                        BOOST_PP_ENUM_TRAILING(ARG_COUNT, BOOST_PROTO_ARG_N, expr)
                    );
                }
            };
        }

    #undef N
    #undef ARG_COUNT

#endif
