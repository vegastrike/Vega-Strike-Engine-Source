///////////////////////////////////////////////////////////////////////////////
/// \file eval.hpp
/// Contains the eval() expression evaluator.
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_EVAL_HPP_EAN_03_29_2007
#define BOOST_PROTO_EVAL_HPP_EAN_03_29_2007

#include <boost/xpressive/proto/detail/prefix.hpp> // must be first include
#include <boost/type_traits/remove_reference.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp> // must be last include

namespace boost { namespace proto
{

    namespace result_of
    {
        template<typename Expr, typename Context>
        struct eval
        {
            typedef typename Context::template eval<Expr>::result_type type;
        };
    }

    namespace functional
    {
        struct eval
        {
            template<typename Sig>
            struct result;

            template<typename This, typename Expr, typename Context>
            struct result<This(Expr, Context)>
              : proto::result_of::eval<
                    typename remove_reference<Expr>::type
                  , typename remove_reference<Context>::type
                >
            {};

            template<typename Expr, typename Context>
            typename proto::result_of::eval<Expr, Context>::type
            operator ()(Expr &expr, Context &context) const
            {
                return typename Context::template eval<Expr>()(expr, context);
            }

            template<typename Expr, typename Context>
            typename proto::result_of::eval<Expr, Context>::type
            operator ()(Expr &expr, Context const &context) const
            {
                return typename Context::template eval<Expr>()(expr, context);
            }
        };
    }

    functional::eval const eval = {};
}}

#endif
