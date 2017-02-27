#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file deep_copy.hpp
    /// Replace all nodes stored by reference by nodes stored by value.
    //
    //  Copyright 2007 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_DEEP_COPY_HPP_EAN_11_21_2006
    #define BOOST_PROTO_DEEP_COPY_HPP_EAN_11_21_2006

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/enum.hpp>
    #include <boost/preprocessor/iterate.hpp>
    #include <boost/call_traits.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/expr.hpp>
    #include <boost/xpressive/proto/generate.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto
    {
        namespace detail
        {
            template<typename Expr, long Arity = Expr::proto_arity::value>
            struct deep_copy_impl;

            template<typename Expr>
            struct deep_copy_impl<Expr, 0>
            {
                typedef typename terminal<typename result_of::arg<Expr>::type>::type expr_type;
                typedef typename Expr::proto_domain::template apply<expr_type>::type type;

                template<typename Expr2>
                static type call(Expr2 const &expr)
                {
                    return Expr::proto_domain::make(expr_type::make(proto::arg(expr)));
                }
            };
        }

        namespace result_of
        {
            template<typename Expr>
            struct deep_copy
              : detail::deep_copy_impl<Expr>
            {};
        }

        namespace functional
        {
            struct deep_copy
            {
                template<typename Sig>
                struct result;

                template<typename This, typename Expr>
                struct result<This(Expr)>
                  : result_of::deep_copy<typename detail::remove_cv_ref<Expr>::type>
                {};

                template<typename Expr>
                typename result_of::deep_copy<Expr>::type
                operator()(Expr const &expr) const
                {
                    return result_of::deep_copy<Expr>::call(expr);
                }
            };
        }

        functional::deep_copy const deep_copy = {};

        namespace detail
        {
        #define BOOST_PROTO_DEFINE_DEEP_COPY_TYPE(z, n, data)\
            typename deep_copy_impl<typename Expr::BOOST_PP_CAT(proto_arg, n)>::type

        #define BOOST_PROTO_DEFINE_DEEP_COPY_FUN(z, n, data)\
            proto::deep_copy(expr.proto_base().BOOST_PP_CAT(arg, n))

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/deep_copy.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_DEFINE_DEEP_COPY_FUN
        #undef BOOST_PROTO_DEFINE_DEEP_COPY_TYPE
        }

    }}

    #endif // BOOST_PROTO_COMPILER_DEEP_COPY_HPP_EAN_11_21_2006

#else

    #define N BOOST_PP_ITERATION()

            template<typename Expr>
            struct deep_copy_impl<Expr, N>
            {
                typedef expr<typename Expr::proto_tag, BOOST_PP_CAT(args, N)<
                    BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_DEEP_COPY_TYPE, ~)
                > > expr_type;
                typedef typename Expr::proto_domain::template apply<expr_type>::type type;

                template<typename Expr2>
                static type call(Expr2 const &expr)
                {
                    expr_type that = {
                        BOOST_PP_ENUM(N, BOOST_PROTO_DEFINE_DEEP_COPY_FUN, ~)
                    };
                    return Expr::proto_domain::make(that);
                }
            };

    #undef N

#endif
