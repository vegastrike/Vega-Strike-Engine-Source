///////////////////////////////////////////////////////////////////////////////
// as_inverse.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INVERSE_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_INVERSE_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/sizeof.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/proto/proto.hpp>

namespace boost { namespace xpressive { namespace detail
{

    template<typename T>
    struct inverter
    {
        typedef T type;
        static T call(T t)
        {
            t.inverse();
            return t;
        }
    };

    template<typename Traits, bool ICase, bool Not>
    struct inverter<literal_matcher<Traits, ICase, Not> >
    {
        typedef literal_matcher<Traits, ICase, !Not> type;
        static type call(literal_matcher<Traits, ICase, Not> t)
        {
            return type(t);
        }
    };

    template<typename Traits>
    struct inverter<logical_newline_matcher<Traits> >
    {
        // ~_ln matches any one character that is not in the "newline" character class
        typedef posix_charset_matcher<Traits> type;
        static type call(logical_newline_matcher<Traits> t)
        {
            return type(t.newline(), true);
        }
    };

    template<typename Traits>
    struct inverter<assert_word_matcher<word_boundary<true>, Traits> >
    {
        typedef assert_word_matcher<word_boundary<false>, Traits> type;
        static type call(assert_word_matcher<word_boundary<true>, Traits> t)
        {
            return type(t.word());
        }
    };

    template<typename T>
    typename inverter<T>::type invert(T const &t)
    {
        return inverter<T>::call(t);
    }

    template<typename Grammar>
    struct as_inverse
      : Grammar
    {
        as_inverse();

        template<typename Expr, typename State, typename Visitor>
        struct apply
          : inverter<typename Grammar::template apply<Expr, State, Visitor>::type>
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            return detail::invert(Grammar::call(expr, state, visitor));
        }
    };

}}}

#endif
