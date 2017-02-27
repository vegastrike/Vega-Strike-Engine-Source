///////////////////////////////////////////////////////////////////////////////
// as_set.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_SET_HPP_EAN_04_05_2007
#define BOOST_XPRESSIVE_DETAIL_STATIC_TRANSFORMS_AS_SET_HPP_EAN_04_05_2007

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/mpl/assert.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform/arg.hpp>
#include <boost/xpressive/proto/transform/apply.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/static/static.hpp>
#include <boost/xpressive/detail/utility/chset/chset.hpp>
#include <boost/xpressive/detail/utility/traits_utils.hpp>

namespace boost { namespace xpressive { namespace detail
{

    template<typename I>
    typename I::next next_(I)
    {
        return typename I::next();
    }

    template<typename Grammar>
    struct next
      : Grammar
    {
        next();

        template<typename Expr, typename State, typename Visitor>
        struct apply
          : Grammar::template apply<Expr, State, Visitor>::type::next
        {};

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            return detail::next_(Grammar::call(expr, state, visitor));
        }
    };

    template<typename Grammar>
    struct push_back
      : Grammar
    {
        push_back();

        template<typename Expr, typename State, typename Visitor>
        static typename Grammar::template apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            visitor.accept(proto::arg(expr));
            return Grammar::call(expr, state, visitor);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // CharLiteral
    template<typename Char>
    struct CharLiteral
      : proto::or_<
            proto::terminal<char>
          , proto::terminal<Char>
        >
    {};

    template<>
    struct CharLiteral<char>
      : proto::terminal<char>
    {};

    ///////////////////////////////////////////////////////////////////////////
    // ListSet
    //  matches expressions like (set= 'a','b','c')
    //  calculates the size of the set
    //  populates an array of characters
    template<typename Char>
    struct ListSet
      : proto::transform::left<
            proto::or_<
                proto::comma<
                    next<ListSet<Char> >
                  , push_back<CharLiteral<Char> >
                >
              , proto::assign<
                    proto::transform::always<set_initializer_type, mpl::int_<1> >
                  , push_back<CharLiteral<Char> >
                >
            >
        >
    {};

    ///////////////////////////////////////////////////////////////////////////
    // set_fill_visitor
    template<typename Traits>
    struct set_fill_visitor
    {
        typedef typename Traits::char_type char_type;

        set_fill_visitor(char_type *buffer, Traits const &traits)
          : buffer_(buffer)
          , traits_(traits)
        {}

        template<typename Char>
        void accept(Char ch)
        {
            *this->buffer_++ = this->traits_.translate(
                char_cast<typename Traits::char_type>(ch, this->traits_)
            );
        }

        char_type *buffer_;
        Traits const &traits_;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // as_list_set
    template<typename Grammar>
    struct as_list_set
      : Grammar
    {
        as_list_set();

        template<typename Expr, typename State, typename Visitor>
        struct apply
        {
            typedef typename Visitor::traits_type traits_type;
            typedef set_matcher<
                traits_type
              , Grammar::template apply<Expr, State, set_fill_visitor<traits_type> >::type::value
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &state, Visitor &visitor)
        {
            typename apply<Expr, State, Visitor>::type set;
            set_fill_visitor<typename Visitor::traits_type> filler(set.set_, visitor.traits());
            Grammar::call(expr, state, filler);
            return set;
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // charset_context
    //
    template<typename Grammar, typename CharSet, typename Visitor>
    struct charset_context
    {
        template<typename Expr, typename Tag>
        struct eval_
        {
            typedef void result_type;
            void operator()(Expr const &expr, charset_context const &ctx) const
            {
                ctx.set(Grammar::call(expr, end_xpression(), ctx.visitor_));
            }
        };

        template<typename Expr>
        struct eval_<Expr, proto::tag::bitwise_or>
        {
            typedef void result_type;
            void operator()(Expr const &expr, charset_context const &ctx) const
            {
                proto::eval(proto::left(expr), ctx);
                proto::eval(proto::right(expr), ctx);
            }
        };

        // Gah, this is to work around a MSVC bug.
        template<typename Expr>
        struct eval
          : eval_<Expr, typename Expr::proto_tag>
        {};

        typedef typename Visitor::traits_type traits_type;
        typedef typename CharSet::char_type char_type;
        typedef typename CharSet::icase_type icase_type;

        explicit charset_context(CharSet &charset, Visitor &visitor)
          : charset_(charset)
          , visitor_(visitor)
        {}

        template<bool Not>
        void set(literal_matcher<traits_type, icase_type::value, Not> const &ch) const
        {
            // BUGBUG fixme!
            BOOST_MPL_ASSERT_NOT((mpl::bool_<Not>));
            set_char(this->charset_.charset_, ch.ch_, this->visitor_.traits(), icase_type());
        }

        void set(range_matcher<traits_type, icase_type::value> const &rg) const
        {
            // BUGBUG fixme!
            BOOST_ASSERT(!rg.not_);
            set_range(this->charset_.charset_, rg.ch_min_, rg.ch_max_, this->visitor_.traits(), icase_type());
        }

        template<int Size>
        void set(set_matcher<traits_type, Size> const &set_) const
        {
            // BUGBUG fixme!
            BOOST_ASSERT(!set_.not_);
            for(int i=0; i<Size; ++i)
            {
                set_char(this->charset_.charset_, set_.set_[i], this->visitor_.traits(), icase_type::value);
            }
        }

        void set(posix_charset_matcher<traits_type> const &posix) const
        {
            set_class(this->charset_.charset_, posix.mask_, posix.not_, this->visitor_.traits());
        }

        CharSet &charset_;
        Visitor &visitor_;
    };

    ///////////////////////////////////////////////////////////////////////////////
    //
    template<typename Grammar>
    struct as_set
      : Grammar
    {
        as_set();

        template<typename, typename, typename Visitor>
        struct apply
        {
            typedef typename Visitor::char_type char_type;

            // if sizeof(char_type)==1, merge everything into a basic_chset
            // BUGBUG this is not optimal.
            typedef typename mpl::if_<
                is_narrow_char<char_type>
              , basic_chset<char_type>
              , compound_charset<typename Visitor::traits_type>
            >::type charset_type;

            typedef charset_matcher<
                typename Visitor::traits_type
              , Visitor::icase_type::value
              , charset_type
            > type;
        };

        template<typename Expr, typename State, typename Visitor>
        static typename apply<Expr, State, Visitor>::type
        call(Expr const &expr, State const &, Visitor &visitor)
        {
            typedef typename apply<Expr, State, Visitor>::type set_type;
            set_type matcher;
            charset_context<Grammar, set_type, Visitor> ctx(matcher, visitor);
            // Walks the tree and fills in the charset
            proto::eval(expr, ctx);
            return matcher;
        }
    };

}}}

#endif
