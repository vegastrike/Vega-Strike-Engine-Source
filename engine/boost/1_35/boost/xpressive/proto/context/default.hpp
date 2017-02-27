#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file default.hpp
    /// Definintion of default_context, a default evaluation context for
    /// proto::eval() that uses Boost.Typeof to deduce return types
    /// of the built-in operators.
    //
    //  Copyright 2007 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_CONTEXT_DEFAULT_HPP_EAN_01_08_2007
    #define BOOST_PROTO_CONTEXT_DEFAULT_HPP_EAN_01_08_2007

    #include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
    #include <boost/config.hpp>
    #include <boost/detail/workaround.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/facilities/intercept.hpp>
    #include <boost/preprocessor/repetition/enum_shifted.hpp>
    #include <boost/preprocessor/repetition/enum_params.hpp>
    #include <boost/preprocessor/repetition/enum_trailing.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/tuple/elem.hpp>
    #include <boost/mpl/if.hpp>
    #include <boost/typeof/typeof.hpp>
    #include <boost/utility/result_of.hpp>
    #include <boost/type_traits/is_const.hpp>
    #include <boost/type_traits/is_function.hpp>
    #include <boost/type_traits/remove_cv.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/tags.hpp>
    #include <boost/xpressive/proto/eval.hpp>
    #include <boost/xpressive/proto/traits.hpp> // for proto::arg_c()
    #include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

    // If we're generating doxygen documentation, hide all the nasty
    // Boost.Typeof gunk.
    #ifndef BOOST_PROTO_DOXYGEN_INVOKED
        #define BOOST_PROTO_DECLTYPE_NESTED_TYPEDEF_TPL_(Nested, Expr)\
            BOOST_TYPEOF_NESTED_TYPEDEF_TPL(BOOST_PP_CAT(nested_and_hidden_, Nested), Expr)\
            static int const sz = sizeof(detail::check_reference(Expr));  \
            struct Nested\
              : mpl::if_c<\
                    1==sz\
                  , typename BOOST_PP_CAT(nested_and_hidden_, Nested)::type &\
                  , typename BOOST_PP_CAT(nested_and_hidden_, Nested)::type\
                >\
            {};

        #define BOOST_PROTO_DECLTYPE_(Expr, Type)\
            BOOST_PROTO_DECLTYPE_NESTED_TYPEDEF_TPL_(BOOST_PP_CAT(nested_, Type), (Expr))\
            typedef typename BOOST_PP_CAT(nested_, Type)::type Type;
    #else
        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_DECLTYPE_NESTED_TYPEDEF_TPL_(Nested, Expr)
        /// INTERNAL ONLY
        ///
        #define BOOST_PROTO_DECLTYPE_(Expr, Type)\
            typedef detail::unspecified Type;
    #endif

    namespace boost { namespace proto
    {
        namespace detail
        {
            template<typename T> T make();

            template<typename T>
            char check_reference(T &);

            template<typename T>
            char (&check_reference(T const &))[2];

            template<typename A0, typename A1>
            struct comma_result
            {
                BOOST_PROTO_DECLTYPE_((detail::make<A0>(), detail::make<A1>()), type)
            };

            template<typename A0>
            struct comma_result<A0, void>
            {
                typedef void type;
            };

            template<typename A1>
            struct comma_result<void, A1>
            {
                typedef A1 type;
            };

            template<>
            struct comma_result<void, void>
            {
                typedef void type;
            };

            template<typename T, typename U = T>
            struct result_of_fixup
              : mpl::if_<is_function<T>, T *, U>
            {};

            template<typename T, typename U>
            struct result_of_fixup<T &, U>
              : result_of_fixup<T, T>
            {};

            template<typename T, typename U>
            struct result_of_fixup<T *, U>
              : result_of_fixup<T, U>
            {};

            template<typename T, typename U>
            struct result_of_fixup<T const, U>
              : result_of_fixup<T, U>
            {};

            //// Tests for result_of_fixup
            //struct bar {};
            //BOOST_MPL_ASSERT((is_same<bar,        result_of_fixup<bar>::type>));
            //BOOST_MPL_ASSERT((is_same<bar const,  result_of_fixup<bar const>::type>));
            //BOOST_MPL_ASSERT((is_same<bar,        result_of_fixup<bar &>::type>));
            //BOOST_MPL_ASSERT((is_same<bar const,  result_of_fixup<bar const &>::type>));
            //BOOST_MPL_ASSERT((is_same<void(*)(),  result_of_fixup<void(*)()>::type>));
            //BOOST_MPL_ASSERT((is_same<void(*)(),  result_of_fixup<void(* const)()>::type>));
            //BOOST_MPL_ASSERT((is_same<void(*)(),  result_of_fixup<void(* const &)()>::type>));
            //BOOST_MPL_ASSERT((is_same<void(*)(),  result_of_fixup<void(&)()>::type>));

        #if BOOST_WORKAROUND(BOOST_MSVC, >= 1400)
            template<typename T> T &make_ref_(T &t);
            template<typename T> T const &make_ref_(T const &t);
            #define BOOST_PROTO_REF(x) detail::make_ref_(x)
        #else
            #define BOOST_PROTO_REF(x) x
        #endif
        }

        namespace context
        {
            template<typename Expr, typename Context, typename Tag, long Arity>
            struct default_eval
            {};

            /// INTERNAL ONLY
            ///
        #define BOOST_PROTO_UNARY_OP_RESULT(Op, Tag)                                                \
            template<typename Expr, typename Context>                                               \
            struct default_eval<Expr, Context, Tag, 1>                                              \
            {                                                                                       \
            private:                                                                                \
                static Expr &sexpr;                                                                 \
                static Context &sctx;                                                               \
            public:                                                                                 \
                BOOST_PROTO_DECLTYPE_(Op proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx), result_type)\
                result_type operator()(Expr &expr, Context &ctx) const                              \
                {                                                                                   \
                    return Op proto::eval(proto::arg_c<0>(expr), ctx);                              \
                }                                                                                   \
            };                                                                                      \
            /**/

            /// INTERNAL ONLY
            ///
        #define BOOST_PROTO_BINARY_OP_RESULT(Op, Tag)                                               \
            template<typename Expr, typename Context>                                               \
            struct default_eval<Expr, Context, Tag, 2>                                              \
            {                                                                                       \
            private:                                                                                \
                static Expr &sexpr;                                                                 \
                static Context &sctx;                                                               \
            public:                                                                                 \
                BOOST_PROTO_DECLTYPE_(proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx) Op proto::eval(BOOST_PROTO_REF(proto::arg_c<1>(sexpr)), sctx), result_type)\
                result_type operator()(Expr &expr, Context &ctx) const                              \
                {                                                                                   \
                    return proto::eval(proto::arg_c<0>(expr), ctx) Op proto::eval(proto::arg_c<1>(expr), ctx);\
                }                                                                                   \
            };                                                                                      \
            /**/

            BOOST_PROTO_UNARY_OP_RESULT(+, proto::tag::posit)
            BOOST_PROTO_UNARY_OP_RESULT(-, proto::tag::negate)
            BOOST_PROTO_UNARY_OP_RESULT(*, proto::tag::dereference)
            BOOST_PROTO_UNARY_OP_RESULT(~, proto::tag::complement)
            BOOST_PROTO_UNARY_OP_RESULT(&, proto::tag::address_of)
            BOOST_PROTO_UNARY_OP_RESULT(!, proto::tag::logical_not)
            BOOST_PROTO_UNARY_OP_RESULT(++, proto::tag::pre_inc)
            BOOST_PROTO_UNARY_OP_RESULT(--, proto::tag::pre_dec)

            BOOST_PROTO_BINARY_OP_RESULT(<<, proto::tag::shift_left)
            BOOST_PROTO_BINARY_OP_RESULT(>>, proto::tag::shift_right)
            BOOST_PROTO_BINARY_OP_RESULT(*, proto::tag::multiplies)
            BOOST_PROTO_BINARY_OP_RESULT(/, proto::tag::divides)
            BOOST_PROTO_BINARY_OP_RESULT(%, proto::tag::modulus)
            BOOST_PROTO_BINARY_OP_RESULT(+, proto::tag::plus)
            BOOST_PROTO_BINARY_OP_RESULT(-, proto::tag::minus)
            BOOST_PROTO_BINARY_OP_RESULT(<, proto::tag::less)
            BOOST_PROTO_BINARY_OP_RESULT(>, proto::tag::greater)
            BOOST_PROTO_BINARY_OP_RESULT(<=, proto::tag::less_equal)
            BOOST_PROTO_BINARY_OP_RESULT(>=, proto::tag::greater_equal)
            BOOST_PROTO_BINARY_OP_RESULT(==, proto::tag::equal_to)
            BOOST_PROTO_BINARY_OP_RESULT(!=, proto::tag::not_equal_to)
            BOOST_PROTO_BINARY_OP_RESULT(||, proto::tag::logical_or)
            BOOST_PROTO_BINARY_OP_RESULT(&&, proto::tag::logical_and)
            BOOST_PROTO_BINARY_OP_RESULT(&, proto::tag::bitwise_and)
            BOOST_PROTO_BINARY_OP_RESULT(|, proto::tag::bitwise_or)
            BOOST_PROTO_BINARY_OP_RESULT(^, proto::tag::bitwise_xor)
            BOOST_PROTO_BINARY_OP_RESULT(->*, proto::tag::mem_ptr)

            BOOST_PROTO_BINARY_OP_RESULT(=, proto::tag::assign)
            BOOST_PROTO_BINARY_OP_RESULT(<<=, proto::tag::shift_left_assign)
            BOOST_PROTO_BINARY_OP_RESULT(>>=, proto::tag::shift_right_assign)
            BOOST_PROTO_BINARY_OP_RESULT(*=, proto::tag::multiplies_assign)
            BOOST_PROTO_BINARY_OP_RESULT(/=, proto::tag::divides_assign)
            BOOST_PROTO_BINARY_OP_RESULT(%=, proto::tag::modulus_assign)
            BOOST_PROTO_BINARY_OP_RESULT(+=, proto::tag::plus_assign)
            BOOST_PROTO_BINARY_OP_RESULT(-=, proto::tag::minus_assign)
            BOOST_PROTO_BINARY_OP_RESULT(&=, proto::tag::bitwise_and_assign)
            BOOST_PROTO_BINARY_OP_RESULT(|=, proto::tag::bitwise_or_assign)
            BOOST_PROTO_BINARY_OP_RESULT(^=, proto::tag::bitwise_xor_assign)

            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::terminal, 0>
            {
                typedef
                    typename mpl::if_<
                        is_const<Expr>
                      , typename proto::result_of::arg<Expr>::const_reference
                      , typename proto::result_of::arg<Expr>::reference
                    >::type
                result_type;

                result_type operator()(Expr &expr, Context &) const
                {
                    return proto::arg(expr);
                }
            };

            // Handle post-increment specially.
            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::post_inc, 1>
            {
            private:
                static Expr &sexpr;
                static Context &sctx;
            public:
                BOOST_PROTO_DECLTYPE_(proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx) ++, result_type)
                result_type operator()(Expr &expr, Context &ctx) const
                {
                    return proto::eval(proto::arg_c<0>(expr), ctx) ++;
                }
            };

            // Handle post-decrement specially.
            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::post_dec, 1>
            {
            private:
                static Expr &sexpr;
                static Context &sctx;
            public:
                BOOST_PROTO_DECLTYPE_(proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx) --, result_type)
                result_type operator()(Expr &expr, Context &ctx) const
                {
                    return proto::eval(proto::arg_c<0>(expr), ctx) --;
                }
            };

            // Handle subscript specially.
            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::subscript, 2>
            {
            private:
                static Expr &sexpr;
                static Context &sctx;
            public:
                BOOST_PROTO_DECLTYPE_(proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx)[proto::eval(BOOST_PROTO_REF(proto::arg_c<1>(sexpr)), sctx)], result_type)
                result_type operator()(Expr &expr, Context &ctx) const
                {
                    return proto::eval(proto::arg_c<0>(expr), ctx)[proto::eval(proto::arg_c<1>(expr), ctx)];
                }
            };

            // Handle if_else_ specially.
            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::if_else_, 3>
            {
            private:
                static Expr &sexpr;
                static Context &sctx;
            public:
                BOOST_PROTO_DECLTYPE_(
                    proto::eval(BOOST_PROTO_REF(proto::arg_c<0>(sexpr)), sctx)
                  ? proto::eval(BOOST_PROTO_REF(proto::arg_c<1>(sexpr)), sctx)
                  : proto::eval(BOOST_PROTO_REF(proto::arg_c<2>(sexpr)), sctx)
                  , result_type
                )
                result_type operator()(Expr &expr, Context &ctx) const
                {
                    return proto::eval(proto::arg_c<0>(expr), ctx)
                         ? proto::eval(proto::arg_c<1>(expr), ctx)
                         : proto::eval(proto::arg_c<2>(expr), ctx);
                }
            };

            // Handle comma specially.
            template<typename Expr, typename Context>
            struct default_eval<Expr, Context, proto::tag::comma, 2>
            {
                typedef typename proto::result_of::eval<typename proto::result_of::arg_c<Expr, 0>::type, Context>::type proto_arg0;
                typedef typename proto::result_of::eval<typename proto::result_of::arg_c<Expr, 1>::type, Context>::type proto_arg1;
                typedef typename detail::comma_result<proto_arg0, proto_arg1>::type result_type;
                result_type operator()(Expr &expr, Context &ctx) const
                {
                    return proto::eval(proto::arg_c<0>(expr), ctx), proto::eval(proto::arg_c<1>(expr), ctx);
                }
            };

        #define BOOST_PROTO_EVAL_N_TYPE(Z, N, Data)\
            typename proto::result_of::eval<\
                typename proto::result_of::arg_c<BOOST_PP_TUPLE_ELEM(2, 0, Data), N>::type\
              , BOOST_PP_TUPLE_ELEM(2, 1, Data)\
            >::type

        #define BOOST_PROTO_EVAL_N(Z, N, Data)\
            proto::eval(proto::arg_c<N>(BOOST_PP_TUPLE_ELEM(2, 0, Data)), BOOST_PP_TUPLE_ELEM(2, 1, Data))

        #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/context/default.hpp>))
        #include BOOST_PP_ITERATE()

        #undef BOOST_PROTO_EVAL_N_TYPE
        #undef BOOST_PROTO_EVAL_N

            /// default_context
            ///
            struct default_context
            {
                /// default_context::eval
                ///
                template<typename Expr, typename ThisContext = default_context const>
                struct eval
                  : default_eval<Expr, ThisContext>
                {};
            };

        } // namespace context

    }} // namespace boost::proto

    #undef BOOST_PROTO_DECLTYPE_NESTED_TYPEDEF_TPL_
    #undef BOOST_PROTO_DECLTYPE_

    #endif

#else

    #define N BOOST_PP_ITERATION()

        // Handle function specially
        template<typename Expr, typename Context>
        struct default_eval<Expr, Context, proto::tag::function, N>
        {
            typedef
                typename detail::result_of_fixup<
                    BOOST_PROTO_EVAL_N_TYPE(1, 0, (Expr, Context))
                >::type
            function_type;

            typedef
                typename boost::result_of<
                    function_type(
                        BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_EVAL_N_TYPE, (Expr, Context))
                    )
                >::type
            result_type;

            result_type operator ()(Expr &expr, Context &context) const
            {
                return BOOST_PROTO_EVAL_N(1, 0, (expr, context))(
                    BOOST_PP_ENUM_SHIFTED(N, BOOST_PROTO_EVAL_N, (expr, context))
                );
            }
        };

    #undef N

#endif
