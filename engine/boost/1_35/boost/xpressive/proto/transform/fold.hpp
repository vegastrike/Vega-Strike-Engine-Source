#ifndef BOOST_PP_IS_ITERATING
    ///////////////////////////////////////////////////////////////////////////////
    /// \file fold.hpp
    /// A special-purpose proto transform for merging sequences of binary operations.
    /// It transforms the right operand and passes the result as state while transforming
    /// the left. Or, it might do the left first, if you choose.
    //
    //  Copyright 2007 Eric Niebler. Distributed under the Boost
    //  Software License, Version 1.0. (See accompanying file
    //  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    #ifndef BOOST_PROTO_TRANSFORM_FOLD_HPP_EAN_12_16_2006
    #define BOOST_PROTO_TRANSFORM_FOLD_HPP_EAN_12_16_2006

    #include <boost/xpressive/proto/detail/prefix.hpp>
    #include <boost/preprocessor/cat.hpp>
    #include <boost/preprocessor/arithmetic/inc.hpp>
    #include <boost/preprocessor/arithmetic/sub.hpp>
    #include <boost/preprocessor/iteration/iterate.hpp>
    #include <boost/preprocessor/repetition/repeat.hpp>
    #include <boost/xpressive/proto/proto_fwd.hpp>
    #include <boost/xpressive/proto/traits.hpp>
    #include <boost/xpressive/proto/transform/branch.hpp>
    #include <boost/xpressive/proto/detail/suffix.hpp>

    namespace boost { namespace proto { namespace transform
    {

        namespace detail
        {
            template<typename Grammar, typename Expr, typename State, typename Visitor, long Arity = Expr::proto_arity::value>
            struct fold_impl
            {};

            template<typename Grammar, typename Expr, typename State, typename Visitor, long Arity = Expr::proto_arity::value>
            struct reverse_fold_impl
            {};

            #define BOOST_PROTO_ARG_N_TYPE(n)\
                BOOST_PP_CAT(proto_arg, n)\
                /**/

            #define BOOST_PROTO_FOLD_STATE_TYPE(z, n, data)\
                typedef typename Grammar::BOOST_PROTO_ARG_N_TYPE(n)::template\
                    apply<typename Expr::BOOST_PROTO_ARG_N_TYPE(n)::proto_base_expr, BOOST_PP_CAT(state, n), Visitor>::type\
                BOOST_PP_CAT(state, BOOST_PP_INC(n));\
                /**/

            #define BOOST_PROTO_FOLD_STATE(z, n, data)\
                BOOST_PP_CAT(state, BOOST_PP_INC(n)) const &BOOST_PP_CAT(s, BOOST_PP_INC(n)) =\
                    Grammar::BOOST_PROTO_ARG_N_TYPE(n)::call(expr.BOOST_PP_CAT(arg, n).proto_base(), BOOST_PP_CAT(s, n), visitor);\
                /**/

            #define BOOST_PROTO_REVERSE_FOLD_STATE_TYPE(z, n, data)\
                typedef typename Grammar::BOOST_PROTO_ARG_N_TYPE(BOOST_PP_SUB(data, BOOST_PP_INC(n)))::template\
                    apply<typename Expr::BOOST_PROTO_ARG_N_TYPE(BOOST_PP_SUB(data, BOOST_PP_INC(n)))::proto_base_expr, BOOST_PP_CAT(state, BOOST_PP_SUB(data, n)), Visitor>::type\
                BOOST_PP_CAT(state, BOOST_PP_SUB(data, BOOST_PP_INC(n)));\
                /**/

            #define BOOST_PROTO_REVERSE_FOLD_STATE(z, n, data)\
                BOOST_PP_CAT(state, BOOST_PP_SUB(data, BOOST_PP_INC(n))) const &BOOST_PP_CAT(s, BOOST_PP_SUB(data, BOOST_PP_INC(n))) =\
                    Grammar::BOOST_PROTO_ARG_N_TYPE(BOOST_PP_SUB(data, BOOST_PP_INC(n)))::call(expr.BOOST_PP_CAT(arg, BOOST_PP_SUB(data, BOOST_PP_INC(n))).proto_base(), BOOST_PP_CAT(s, BOOST_PP_SUB(data, n)), visitor);\
                /**/

            #define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PROTO_MAX_ARITY, <boost/xpressive/proto/transform/fold.hpp>))
            #include BOOST_PP_ITERATE()

            #undef BOOST_PROTO_REVERSE_FOLD_STATE
            #undef BOOST_PROTO_REVERSE_FOLD_STATE_TYPE
            #undef BOOST_PROTO_FOLD_STATE
            #undef BOOST_PROTO_FOLD_STATE_TYPE
            #undef BOOST_PROTO_ARG_N_TYPE
        }

        // A fold transform that transforms the left sub-tree and
        // uses the result as state while transforming the right.
        template<typename Grammar>
        struct fold<Grammar, void>
          : Grammar
        {
            fold() {}

            template<typename Expr, typename State, typename Visitor>
            struct apply
              : detail::fold_impl<Grammar, typename Expr::proto_base_expr, State, Visitor>
            {};

            template<typename Expr, typename State, typename Visitor>
            static typename apply<Expr, State, Visitor>::type
            call(Expr const &expr, State const &state, Visitor &visitor)
            {
                return apply<Expr, State, Visitor>::call(expr.proto_base(), state, visitor);
            }
        };

        // A reverse_fold compiler that compiles the right sub-tree and
        // uses the result as state while compiling the left.
        template<typename Grammar>
        struct reverse_fold<Grammar, void>
          : Grammar
        {
            reverse_fold() {}

            template<typename Expr, typename State, typename Visitor>
            struct apply
              : detail::reverse_fold_impl<Grammar, typename Expr::proto_base_expr, State, Visitor>
            {};

            template<typename Expr, typename State, typename Visitor>
            static typename apply<Expr, State, Visitor>::type
            call(Expr const &expr, State const &state, Visitor &visitor)
            {
                return apply<Expr, State, Visitor>::call(expr.proto_base(), state, visitor);
            }
        };

        // Causes Doxygen to crash. Sigh.
    #ifndef BOOST_PROTO_DOXYGEN_INVOKED
        template<typename Grammar, typename State>
        struct fold
          : branch<fold<Grammar, void>, State>
        {};

        template<typename Grammar, typename State>
        struct reverse_fold
          : branch<reverse_fold<Grammar, void>, State>
        {};
    #endif

    }}}

    namespace boost { namespace proto
    {
        template<typename Grammar, typename State>
        struct is_transform<transform::fold<Grammar, State> >
          : mpl::true_
        {};

        template<typename Grammar, typename State>
        struct is_transform<transform::reverse_fold<Grammar, State> >
          : mpl::true_
        {};
    }}

    #endif

#else

    #define N BOOST_PP_ITERATION()

            template<typename Grammar, typename Expr, typename state0, typename Visitor>
            struct fold_impl<Grammar, Expr, state0, Visitor, N>
            {
                BOOST_PP_REPEAT(N, BOOST_PROTO_FOLD_STATE_TYPE, N)
                typedef BOOST_PP_CAT(state, N) type;

                static type call(Expr const &expr, state0 const &s0, Visitor &visitor)
                {
                    BOOST_PP_REPEAT(N, BOOST_PROTO_FOLD_STATE, N)
                    return BOOST_PP_CAT(s, N);
                }
            };

            template<typename Grammar, typename Expr, typename BOOST_PP_CAT(state, N), typename Visitor>
            struct reverse_fold_impl<Grammar, Expr, BOOST_PP_CAT(state, N), Visitor, N>
            {
                BOOST_PP_REPEAT(N, BOOST_PROTO_REVERSE_FOLD_STATE_TYPE, N)
                typedef state0 type;

                static type call(Expr const &expr, BOOST_PP_CAT(state, N) const &BOOST_PP_CAT(s, N), Visitor &visitor)
                {
                    BOOST_PP_REPEAT(N, BOOST_PROTO_REVERSE_FOLD_STATE, N)
                    return s0;
                }
            };
    #undef N

#endif
