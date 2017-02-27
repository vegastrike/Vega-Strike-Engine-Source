///////////////////////////////////////////////////////////////////////////////
/// \file domain.hpp
/// Contains definition of domain\<\> class template, for defining domains
/// with a grammar for controlling operator overloading.
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_DOMAIN_HPP_EAN_02_13_2007
#define BOOST_PROTO_DOMAIN_HPP_EAN_02_13_2007

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/xpressive/proto/proto_fwd.hpp>
#include <boost/xpressive/proto/generate.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

namespace boost { namespace proto
{

    namespace detail
    {
        struct not_a_generator
        {};

        struct not_a_grammar
        {};
    }

    namespace domainns_
    {
        template<typename Generator, typename Grammar>
        struct domain
          : Generator
        {
            typedef Grammar grammar;

            /// INTERNAL ONLY
            ///
            typedef void proto_is_domain_;
        };

        struct default_domain
          : domain<>
        {};

        struct deduce_domain
          : domain<detail::not_a_generator, detail::not_a_grammar>
        {};
    }

    namespace result_of
    {
        template<typename T, typename EnableIf>
        struct is_domain
          : mpl::false_
        {};

        template<typename T>
        struct is_domain<T, typename T::proto_is_domain_>
          : mpl::true_
        {};

        template<typename T, typename EnableIf>
        struct domain_of
        {
            typedef default_domain type;
        };

        template<typename T>
        struct domain_of<T, typename T::proto_is_expr_>
        {
            typedef typename T::proto_domain type;
        };
    }
}}

#endif
