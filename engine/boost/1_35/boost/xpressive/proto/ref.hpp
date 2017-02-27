///////////////////////////////////////////////////////////////////////////////
/// \file ref.hpp
/// Utility for storing a sub-expr by reference
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_REF_HPP_EAN_04_01_2005
#define BOOST_PROTO_REF_HPP_EAN_04_01_2005

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning(push)
# pragma warning(disable : 4510) // default constructor could not be generated
# pragma warning(disable : 4512) // assignment operator could not be generated
# pragma warning(disable : 4610) // user defined constructor required
#endif

namespace boost { namespace proto
{

#define BOOST_PROTO_ARG(z, n, data)\
    typedef\
        typename Expr::BOOST_PP_CAT(proto_arg, n)\
    BOOST_PP_CAT(proto_arg, n);\
    /**/

    namespace refns_
    {
        template<typename Expr>
        struct ref_
        {
            typedef typename Expr::proto_base_expr proto_base_expr;
            typedef typename Expr::proto_tag proto_tag;
            typedef typename Expr::proto_args proto_args;
            typedef typename Expr::proto_arity proto_arity;
            typedef typename Expr::proto_domain proto_domain;
            typedef tag::proto_ref fusion_tag;
            typedef void proto_is_ref_;
            typedef void proto_is_expr_;
            typedef Expr proto_derived_expr;

            BOOST_PP_REPEAT(BOOST_PROTO_MAX_ARITY, BOOST_PROTO_ARG, _)

            typename mpl::if_<is_const<Expr>, proto_base_expr const &, proto_base_expr &>::type
            proto_base() const
            {
                return this->expr.proto_base();
            }

            static ref_<Expr> make(Expr &expr)
            {
                ref_<Expr> that = {expr};
                return that;
            }

            Expr &expr;
        };

        // ref_-to-ref_ is not allowed. this will cause a compile error.
        template<typename Expr>
        struct ref_<ref_<Expr> >
        {};
    }

#undef BOOST_PROTO_ARG

    namespace result_of
    {
        template<typename T>
        struct unref
        {
            typedef T type;
            typedef T &reference;
            typedef T const &const_reference;
        };

        template<typename T>
        struct unref<ref_<T> >
        {
            typedef T type;
            typedef T &reference;
            typedef T &const_reference;
        };

        template<typename T>
        struct unref<ref_<T const> >
        {
            typedef T type;
            typedef T const &reference;
            typedef T const &const_reference;
        };

        template<typename T>
        struct unref<T &>
        {
            typedef T type;
            typedef T &reference;
            typedef T &const_reference;
        };

        template<typename T>
        struct unref<T const &>
        {
            typedef T type;
            typedef T const &reference;
            typedef T const &const_reference;
        };

        template<typename T, std::size_t N>
        struct unref<T (&)[N]>
        {
            typedef T (&type)[N];
            typedef T (&reference)[N];
            typedef T (&const_reference)[N];
        };

        template<typename T, std::size_t N>
        struct unref<T const (&)[N]>
        {
            typedef T const (&type)[N];
            typedef T const (&reference)[N];
            typedef T const (&const_reference)[N];
        };
    }

    namespace functional
    {
        struct unref
        {
            template<typename T>
            struct result {};

            template<typename This, typename T>
            struct result<This(T)>
              : result_of::unref<typename detail::remove_cv_ref<T>::type>
            {};

            template<typename T>
            T &operator()(T &t) const
            {
                return t;
            }

            template<typename T>
            T const &operator()(T const &t) const
            {
                return t;
            }

            template<typename T>
            T &operator()(ref_<T> &t) const
            {
                return t.expr;
            }

            template<typename T>
            T &operator()(ref_<T> const &t) const
            {
                return t.expr;
            }
        };
    }

    functional::unref const unref = {};
}}

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma warning(pop)
#endif

#endif
