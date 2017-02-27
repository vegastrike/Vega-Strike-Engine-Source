///////////////////////////////////////////////////////////////////////////////
// peeker.hpp
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_XPRESSIVE_DETAIL_CORE_PEEKER_HPP_EAN_10_04_2005
#define BOOST_XPRESSIVE_DETAIL_CORE_PEEKER_HPP_EAN_10_04_2005

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <string>
#include <typeinfo>
#include <boost/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/xpressive/detail/detail_fwd.hpp>
#include <boost/xpressive/detail/core/matchers.hpp>
#include <boost/xpressive/detail/utility/hash_peek_bitset.hpp>
#include <boost/xpressive/detail/utility/never_true.hpp>
#include <boost/xpressive/detail/utility/algorithm.hpp>

namespace boost { namespace xpressive { namespace detail
{

///////////////////////////////////////////////////////////////////////////////
// peek_next
//   tell whether or not to keep looking for a peek optimization
template<typename Matcher>
struct peek_next
  : mpl::bool_<Matcher::width == 0>
{
};

template<>
struct peek_next<mark_begin_matcher>
  : mpl::true_
{
};

template<>
struct peek_next<repeat_begin_matcher>
  : mpl::true_
{
};

///////////////////////////////////////////////////////////////////////////////
// xpression_peeker
//
template<typename Char>
struct peeker_string
{
    Char const *begin_;
    Char const *end_;
    bool icase_;
};

///////////////////////////////////////////////////////////////////////////////
// char_sink
//
template<typename Traits, bool ICase>
struct char_sink
{
    typedef typename Traits::char_type char_type;

    char_sink(hash_peek_bitset<char_type> &bset, Traits const &traits)
      : bset_(bset)
      , traits_(traits)
    {}

    void operator()(char_type ch) const
    {
        this->bset_.set_char(ch, ICase, this->traits_);
    }

    hash_peek_bitset<char_type> &bset_;
    Traits const &traits_;
};

///////////////////////////////////////////////////////////////////////////////
// xpression_peeker
//
template<typename Char>
struct xpression_peeker
{
    template<typename Traits>
    xpression_peeker(hash_peek_bitset<Char> &bset, Traits const &traits)
      : bset_(bset)
      , str_()
      , line_start_(false)
      , traits_(0)
      , traits_type_(0)
    {
        this->set_traits(traits);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // accessors
    peeker_string<Char> const &get_string() const
    {
        return this->str_;
    }

    bool line_start() const
    {
        return this->line_start_;
    }

    hash_peek_bitset<Char> const &bitset() const
    {
        return this->bset_;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // modifiers
    void fail(bool do_fail = true)
    {
        if(do_fail)
        {
            this->bset_.set_all();
        }
    }

    template<typename Matcher>
    peek_next<Matcher> accept(Matcher const &)
    {
        this->fail(!peek_next<Matcher>::value);
        return peek_next<Matcher>();
    }

    template<typename Traits>
    mpl::true_ accept(assert_bol_matcher<Traits> const &)
    {
        this->line_start_ = true;
        return mpl::true_();
    }

    template<typename Traits, bool ICase>
    mpl::false_ accept(literal_matcher<Traits, ICase, false> const &xpr)
    {
        this->bset_.set_char(xpr.ch_, ICase, this->get_traits_<Traits>());
        return mpl::false_();
    }

    template<typename Traits, bool ICase>
    mpl::false_ accept(string_matcher<Traits, ICase> const &xpr)
    {
        this->bset_.set_char(xpr.str_[0], ICase, this->get_traits_<Traits>());
        this->str_.begin_ = detail::data_begin(xpr.str_);
        this->str_.end_ = detail::data_end(xpr.str_);
        this->str_.icase_ = ICase;
        return mpl::false_();
    }

    template<typename Alternates, typename Traits>
    mpl::false_ accept(alternate_matcher<Alternates, Traits> const &xpr)
    {
        BOOST_ASSERT(0 != xpr.bset_.count());
        this->bset_.set_bitset(xpr.bset_);
        return mpl::false_();
    }

    template<typename Matcher, typename Traits, bool ICase>
    mpl::false_ accept(attr_matcher<Matcher, Traits, ICase> const &xpr)
    {
        xpr.sym_.peek(char_sink<Traits, ICase>(this->bset_, this->get_traits_<Traits>()));
        return mpl::false_();
    }

    template<typename Xpr, bool Greedy>
    mpl::false_ accept(optional_matcher<Xpr, Greedy> const &)
    {
        this->fail();  // a union of xpr and next
        return mpl::false_();
    }

    template<typename Xpr, bool Greedy>
    mpl::false_ accept(optional_mark_matcher<Xpr, Greedy> const &)
    {
        this->fail();  // a union of xpr and next
        return mpl::false_();
    }

    //template<typename Xpr, bool Greedy>
    //mpl::true_ accept(optional_matcher<Xpr, Greedy> const &xpr)
    //{
    //    xpr.xpr_.peek(*this);  // a union of xpr and next
    //    return mpl::true_();
    //}

    //template<typename Xpr, bool Greedy>
    //mpl::true_ accept(optional_mark_matcher<Xpr, Greedy> const &xpr)
    //{
    //    xpr.xpr_.peek(*this);  // a union of xpr and next
    //    return mpl::true_();
    //}

    template<typename Traits>
    mpl::false_ accept(posix_charset_matcher<Traits> const &xpr)
    {
        this->bset_.set_class(xpr.mask_, xpr.not_, this->get_traits_<Traits>());
        return mpl::false_();
    }

    template<bool ICase, typename Traits>
    typename enable_if<is_narrow_char<typename Traits::char_type>, mpl::false_>::type
    accept(charset_matcher<Traits, ICase, basic_chset<Char> > const &xpr)
    {
        BOOST_ASSERT(0 != xpr.charset_.base().count());
        this->bset_.set_charset(xpr.charset_, ICase);
        return mpl::false_();
    }

    template<typename Traits, bool ICase>
    mpl::false_ accept(range_matcher<Traits, ICase> const &xpr)
    {
        this->bset_.set_range(xpr.ch_min_, xpr.ch_max_, xpr.not_, ICase, this->get_traits_<Traits>());
        return mpl::false_();
    }

    template<typename Xpr, bool Greedy>
    mpl::false_ accept(simple_repeat_matcher<Xpr, Greedy> const &xpr)
    {
        0 != xpr.min_ ? xpr.xpr_.peek(*this) : this->fail(); // could be a union of xpr and next
        return mpl::false_();
    }

    template<typename Traits>
    void set_traits(Traits const &traits)
    {
        if(0 == this->traits_)
        {
            this->traits_ = &traits;
            this->traits_type_ = &typeid(Traits);
        }
        else if(*this->traits_type_ != typeid(Traits) || this->get_traits_<Traits>() != traits)
        {
            this->fail(); // traits mis-match! set all and bail
        }
    }

private:

    template<typename Traits>
    Traits const &get_traits_() const
    {
        BOOST_ASSERT(!!(*this->traits_type_ == typeid(Traits)));
        return *static_cast<Traits const *>(this->traits_);
    }

    hash_peek_bitset<Char> &bset_;
    peeker_string<Char> str_;
    bool str_icase_;
    bool line_start_;
    void const *traits_;
    std::type_info const *traits_type_;
};

}}} // namespace boost::xpressive::detail

#endif
