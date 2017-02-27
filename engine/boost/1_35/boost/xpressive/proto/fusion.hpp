///////////////////////////////////////////////////////////////////////////////
/// \file fusion.hpp
/// Make any Proto parse tree a valid Fusion sequence
//
//  Copyright 2007 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROTO_FUSION_HPP_EAN_04_29_2006
#define BOOST_PROTO_FUSION_HPP_EAN_04_29_2006

#include <boost/xpressive/proto/detail/prefix.hpp>
#include <boost/xpressive/proto/proto.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/advance.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/begin_end.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/fusion/include/is_view.hpp>
#include <boost/fusion/include/tag_of_fwd.hpp>
#include <boost/fusion/include/category_of.hpp>
#include <boost/fusion/include/iterator_base.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/intrinsic.hpp>
#include <boost/fusion/include/single_view.hpp>
#include <boost/fusion/include/transform_view.hpp>
#include <boost/fusion/support/ext_/is_segmented.hpp>
#include <boost/fusion/sequence/intrinsic/ext_/segments.hpp>
#include <boost/fusion/sequence/intrinsic/ext_/size_s.hpp>
#include <boost/fusion/view/ext_/segmented_iterator.hpp>
#include <boost/xpressive/proto/detail/suffix.hpp>

#define UNREF(x) typename remove_reference<x>::type
#define UNCVREF(x) typename remove_cv<typename remove_reference<x>::type>::type

namespace boost { namespace proto
{
    namespace detail
    {
        template<typename Expr, int Pos>
        struct ref_iterator
          : fusion::iterator_base<ref_iterator<Expr, Pos> >
        {
            typedef Expr expr_type;
            typedef mpl::long_<Pos> index;
            typedef fusion::forward_traversal_tag category;
            typedef tag::proto_ref_iterator fusion_tag;

            ref_iterator(Expr const &expr)
              : expr_(expr)
            {}

            Expr expr_;
        };
    }

    template<typename Expr>
    struct children
      : proto::ref_<Expr>
    {
        children(Expr &expr)
          : proto::ref_<Expr>(proto::ref_<Expr>::make(expr))
        {}
    };

    template<typename Expr>
    children<Expr> children_of(Expr &expr)
    {
        return children<Expr>(expr);
    }

    template<typename Context>
    struct eval_fun
    {
        eval_fun(Context &ctx)
          : ctx_(ctx)
        {}

        template<typename Sig>
        struct result {};

        template<typename This, typename Expr>
        struct result<This(Expr)>
          : proto::result_of::eval<UNREF(Expr), Context>
        {};

        template<typename Expr>
        typename proto::result_of::eval<Expr, Context>::type
        operator()(Expr &expr) const
        {
            return proto::eval(expr, this->ctx_);
        }

    private:
        Context &ctx_;
    };
}}

namespace boost { namespace fusion
{
    namespace extension
    {
        template<typename Tag>
        struct is_view_impl;

        template<>
        struct is_view_impl<proto::tag::proto_ref>
        {
            template<typename Iterator>
            struct apply
              : mpl::true_
            {};
        };

        template<>
        struct is_view_impl<proto::tag::proto_expr>
        {
            template<typename Iterator>
            struct apply
              : mpl::false_
            {};
        };

        template<typename Tag>
        struct value_of_impl;

        template<>
        struct value_of_impl<proto::tag::proto_ref_iterator>
        {
            template<typename Iterator>
            struct apply
              : proto::result_of::arg<typename Iterator::expr_type, typename Iterator::index>
            {};
        };

        template<typename Tag>
        struct deref_impl;

        template<>
        struct deref_impl<proto::tag::proto_ref_iterator>
        {
            template<typename Iterator>
            struct apply
            {
                typedef typename proto::result_of::arg<
                    typename Iterator::expr_type
                  , typename Iterator::index
                >::type const &type;

                static type call(Iterator const &iter)
                {
                    return proto::arg<typename Iterator::index>(iter.expr_);
                }
            };
        };

        template<typename Tag>
        struct advance_impl;

        template<>
        struct advance_impl<proto::tag::proto_ref_iterator>
        {
            template<typename Iterator, typename N>
            struct apply
            {
                typedef typename proto::detail::ref_iterator<
                    typename Iterator::expr_type
                  , Iterator::index::value + N::value
                > type;

                static type call(Iterator const &iter)
                {
                    return type(iter.expr_);
                }
            };
        };

        template<typename Tag>
        struct distance_impl;

        template<>
        struct distance_impl<proto::tag::proto_ref_iterator>
        {
            template<typename IteratorFrom, typename IteratorTo>
            struct apply
              : mpl::long_<IteratorTo::index::value - IteratorFrom::index::value>
            {};
        };

        template<typename Tag>
        struct next_impl;

        template<>
        struct next_impl<proto::tag::proto_ref_iterator>
        {
            template<typename Iterator>
            struct apply
              : advance_impl<proto::tag::proto_ref_iterator>::template apply<Iterator, mpl::long_<1> >
            {};
        };

        template<typename Tag>
        struct prior_impl;

        template<>
        struct prior_impl<proto::tag::proto_ref_iterator>
        {
            template<typename Iterator>
            struct apply
              : advance_impl<proto::tag::proto_ref_iterator>::template apply<Iterator, mpl::long_<-1> >
            {};
        };

        template<typename Tag>
        struct category_of_impl;

        template<>
        struct category_of_impl<proto::tag::proto_ref>
        {
            template<typename Sequence>
            struct apply
            {
                typedef random_access_traversal_tag type;
            };
        };

        template<typename Tag>
        struct size_impl;

        template<>
        struct size_impl<proto::tag::proto_ref>
        {
            template<typename Sequence>
            struct apply
              : Sequence::proto_arity
            {};
        };

        template<typename Tag>
        struct begin_impl;

        template<>
        struct begin_impl<proto::tag::proto_ref>
        {
            template<typename Sequence>
            struct apply
            {
                typedef proto::detail::ref_iterator<Sequence const, 0> type;

                static type call(Sequence& seq)
                {
                    return type(seq);
                }
            };
        };

        template<typename Tag>
        struct end_impl;

        template<>
        struct end_impl<proto::tag::proto_ref>
        {
            template<typename Sequence>
            struct apply
            {
                typedef proto::detail::ref_iterator<Sequence const, Sequence::proto_arity::value> type;

                static type call(Sequence& seq)
                {
                    return type(seq);
                }
            };
        };

        template<typename Tag>
        struct value_at_impl;

        template<>
        struct value_at_impl<proto::tag::proto_ref>
        {
            template<typename Sequence, typename N>
            struct apply
            {
                typedef typename proto::result_of::arg<Sequence, N>::type type;
            };
        };

        template<typename Tag>
        struct at_impl;

        template<>
        struct at_impl<proto::tag::proto_ref>
        {
            template<typename Sequence, typename N>
            struct apply
            {
                typedef typename proto::result_of::arg<Sequence, N>::type const &type;

                static type call(Sequence &seq)
                {
                    return proto::arg_c<N::value>(seq);
                }
            };
        };

        template<typename Tag>
        struct is_segmented_impl;

        template<>
        struct is_segmented_impl<proto::tag::proto_expr>
        {
            template<typename Iterator>
            struct apply
              : mpl::true_
            {};
        };

        template<typename Tag>
        struct as_element
        {
            template<typename Sig>
            struct result {};

            template<typename This, typename Expr>
            struct result<This(Expr)>
              : mpl::if_<
                    is_same<Tag, UNREF(Expr)::proto_tag>
                  , UNCVREF(Expr) const &
                  , fusion::single_view<UNCVREF(Expr) const &>
                >
            {};

            template<typename Expr>
            typename result<as_element(Expr)>::type
            operator()(Expr &expr) const
            {
                return typename result<as_element(Expr)>::type(expr);
            }
        };

        template<typename Tag>
        struct segments_impl;

        template<>
        struct segments_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
            {
                typedef typename Sequence::proto_tag proto_tag;

                typedef fusion::transform_view<
                    proto::ref_<Sequence>
                  , as_element<proto_tag>
                > type;

                static type call(Sequence &sequence)
                {
                    proto::ref_<Sequence> r = {sequence};
                    return type(r, as_element<proto_tag>());
                }
            };
        };

        template<>
        struct category_of_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
            {
                typedef forward_traversal_tag type;
            };
        };

        template<>
        struct begin_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_begin<Sequence>
            {};
        };

        template<>
        struct end_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_end<Sequence>
            {};
        };

        template<>
        struct size_impl<proto::tag::proto_expr>
        {
            template<typename Sequence>
            struct apply
              : fusion::segmented_size<Sequence>
            {};
        };
    }
}}

//namespace boost { namespace mpl
//{
    //template<>
    //struct begin_impl<proto::tag::proto_expr>
    //{
    //    template<typename Sequence>
    //    struct apply
    //      : begin_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args>
    //    {};
    //};

    //template<>
    //struct end_impl<proto::tag::proto_expr>
    //{
    //    template<typename Sequence>
    //    struct apply
    //      : end_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args>
    //    {};
    //};

    //template<>
    //struct size_impl<proto::tag::proto_expr>
    //{
    //    template<typename Sequence>
    //    struct apply
    //    {
    //        typedef typename Sequence::proto_arity type;
    //    };
    //};

    //template<>
    //struct at_impl<proto::tag::proto_expr>
    //{
    //    template<typename Sequence, typename N>
    //    struct apply
    //      : at_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args, N>
    //    {};
    //};


    //template<>
    //struct begin_impl<proto::tag::proto_ref>
    //{
    //    template<typename Sequence>
    //    struct apply
    //      : begin_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args>
    //    {};
    //};

    //template<>
    //struct end_impl<proto::tag::proto_ref>
    //{
    //    template<typename Sequence>
    //    struct apply
    //      : end_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args>
    //    {};
    //};

    //template<>
    //struct size_impl<proto::tag::proto_ref>
    //{
    //    template<typename Sequence>
    //    struct apply
    //    {
    //        typedef typename Sequence::proto_arity type;
    //    };
    //};

    //template<>
    //struct at_impl<proto::tag::proto_ref>
    //{
    //    template<typename Sequence, typename N>
    //    struct apply
    //      : at_impl<typename sequence_tag<typename Sequence::proto_args>::type>
    //            ::template apply<typename Sequence::proto_args, N>
    //    {};
    //};


//}} // namespace boost::mpl

//namespace boost { namespace mpl
//{
//    template<typename Tag, typename Args, long Arity>
//    struct sequence_tag<proto::expr<Tag, Args, Arity> >
//    {
//        typedef proto::tag::proto_expr type;
//    };
//
//    template<typename Expr>
//    struct sequence_tag<proto::ref_<Expr> >
//    {
//        typedef proto::tag::proto_expr type;
//    };
//
//    template<>
//    struct begin_impl<proto::tag::proto_expr>
//    {
//        template<typename Sequence>
//        struct apply
//          : begin_impl<typename sequence_tag<typename Sequence::proto_args>::type>
//                ::template apply<typename Sequence::proto_args>
//        {};
//    };
//
//    template<>
//    struct end_impl<proto::tag::proto_expr>
//    {
//        template<typename Sequence>
//        struct apply
//          : end_impl<typename sequence_tag<typename Sequence::proto_args>::type>
//                ::template apply<typename Sequence::proto_args>
//        {};
//    };
//
//    template<>
//    struct size_impl<proto::tag::proto_expr>
//    {
//        template<typename Sequence>
//        struct apply
//        {
//            typedef typename Sequence::proto_arity type;
//        };
//    };
//
//    template<>
//    struct at_impl<proto::tag::proto_expr>
//    {
//        template<typename Sequence, typename N>
//        struct apply
//          : at_impl<typename sequence_tag<typename Sequence::proto_args>::type>
//                ::template apply<typename Sequence::proto_args, N>
//        {};
//    };
//
//}} // namespace boost::mpl

#undef UNREF
#undef UNCVREF

#endif
