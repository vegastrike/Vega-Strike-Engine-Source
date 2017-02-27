/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2006-2007
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_INTRUSIVE_HASHTABLE_HPP
#define BOOST_INTRUSIVE_HASHTABLE_HPP

#include <boost/intrusive/detail/config_begin.hpp>
//std C++
#include <functional>
#include <utility>
#include <algorithm>
#include <cstddef>
//boost
#include <boost/intrusive/detail/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/intrusive/detail/no_exceptions_support.hpp>
//General intrusive utilities
#include <boost/intrusive/intrusive_fwd.hpp>
#include <boost/intrusive/detail/pointer_to_other.hpp>
#include <boost/intrusive/detail/hashtable_node.hpp>
#include <boost/intrusive/detail/transform_iterator.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/detail/ebo_functor_holder.hpp>
//Implementation utilities
#include <boost/intrusive/trivial_value_traits.hpp>
#include <boost/intrusive/unordered_set_hook.hpp>
#include <boost/intrusive/slist.hpp>

namespace boost {
namespace intrusive {

/// @cond

namespace detail{

template <class T>
struct store_hash_bool
{
   template<bool Add>
   struct two_or_three {one _[2 + Add];};
   template <class U> static one test(...);
   template <class U> static two_or_three<U::store_hash>
      test (detail::bool_<U::store_hash>* = 0);
   static const std::size_t value = sizeof(test<T>(0));
};

template <class T>
struct store_hash_is_true
{
   static const bool value = store_hash_bool<T>::value > sizeof(one)*2;
};

template<class Config>
struct bucket_plus_size
   : public detail::size_holder
      < Config::constant_time_size
      , typename Config::size_type>
{
   typedef detail::size_holder
      < Config::constant_time_size
      , typename Config::size_type>       size_traits;
   typedef typename Config::bucket_traits bucket_traits;

   bucket_plus_size(const bucket_traits &b_traits)
      :  bucket_traits_(b_traits)
   {}
   bucket_traits bucket_traits_;
};

template<class Config>
struct bucket_hash_t : public detail::ebo_functor_holder<typename Config::hash>
{
   typedef typename Config::hash          hasher;
   typedef detail::size_holder
      < Config::constant_time_size
      , typename Config::size_type>       size_traits;
   typedef typename Config::bucket_traits bucket_traits;

   bucket_hash_t(const bucket_traits &b_traits, const hasher & h)
      :  detail::ebo_functor_holder<hasher>(h), bucket_plus_size_(b_traits)
   {}

   bucket_plus_size<Config> bucket_plus_size_;
};

template<class Config>
struct bucket_hash_equal_t : public detail::ebo_functor_holder<typename Config::equal>
{
   typedef typename Config::equal         equal;
   typedef typename Config::hash          hasher;
   typedef typename Config::bucket_traits bucket_traits;

   bucket_hash_equal_t(const bucket_traits &b_traits, const hasher & h, const equal &e)
      :  detail::ebo_functor_holder<typename Config::equal>(e), bucket_hash(b_traits, h)
   {}
   bucket_hash_t<Config> bucket_hash;
};

template<class Config>
struct data_t : public Config::value_traits
{
   typedef typename Config::value_traits  value_traits;
   typedef typename Config::equal         equal;
   typedef typename Config::hash          hasher;
   typedef typename Config::bucket_traits bucket_traits;

   data_t( const bucket_traits &b_traits, const hasher & h
         , const equal &e, const value_traits &val_traits)
      :  Config::value_traits(val_traits), bucket_hash_equal_(b_traits, h, e)
   {}
   bucket_hash_equal_t<Config> bucket_hash_equal_;
};

}  //namespace detail {

template <class T>
struct internal_default_uset_hook
{
   template <class U> static detail::one test(...);
   template <class U> static detail::two test(typename U::default_uset_hook* = 0);
   static const bool value = sizeof(test<T>(0)) == sizeof(detail::two);
};

template <class T>
struct get_default_uset_hook
{
   typedef typename T::default_uset_hook type;
};

template < class  ValueTraits
         , class  Hash
         , class  Equal
         , class  SizeType
         , bool   ConstantTimeSize
         , class  BucketTraits
         , bool   Power2Buckets
         >
struct usetopt
{
   typedef ValueTraits  value_traits;
   typedef Hash         hash;
   typedef Equal        equal;
   typedef SizeType     size_type;
   typedef BucketTraits bucket_traits;
   static const bool constant_time_size = ConstantTimeSize;
   static const bool power_2_buckets = Power2Buckets;
};

struct default_bucket_traits;

template <class T>
struct uset_defaults
   :  pack_options
      < none
      , base_hook
         <  typename detail::eval_if_c
               < internal_default_uset_hook<T>::value
               , get_default_uset_hook<T>
               , detail::identity<none>
               >::type
         >
      , constant_time_size<true>
      , size_type<std::size_t>
      , equal<std::equal_to<T> >
      , hash<boost::hash<T> >
      , bucket_traits<default_bucket_traits>
      , power_2_buckets<false>
      >::type
{};

template<class NodeTraits>
struct get_slist_impl
{
   typedef trivial_value_traits<NodeTraits, normal_link> trivial_traits;

   //Reducing symbol length
   struct type : make_slist
      < typename NodeTraits::node
      , boost::intrusive::value_traits<trivial_traits>
      , boost::intrusive::constant_time_size<false>
      , boost::intrusive::size_type<std::size_t>
      >::type
   {};
};

/// @endcond

template<class ValueTraitsOrHookOption>
struct unordered_bucket
{
   /// @cond
   typedef typename ValueTraitsOrHookOption::
      template pack<none>::value_traits         supposed_value_traits;

   typedef typename detail::eval_if_c
      < detail::external_value_traits_is_true
         <supposed_value_traits>::value
      , detail::eval_value_traits
         <supposed_value_traits>
      , detail::identity
         <supposed_value_traits>
      >::type                                   real_value_traits;

   typedef typename detail::get_node_traits
      <real_value_traits>::type                 node_traits;
   typedef typename get_slist_impl
      <node_traits>::type                       slist_impl;
   typedef detail::bucket_impl<slist_impl>      implementation_defined;
   /// @endcond
   typedef implementation_defined               type;
};

template<class ValueTraitsOrHookOption>
struct unordered_bucket_ptr
{
   /// @cond
   typedef typename ValueTraitsOrHookOption::
      template pack<none>::value_traits         supposed_value_traits;
   typedef typename detail::eval_if_c
      < detail::external_value_traits_is_true
         <supposed_value_traits>::value
      , detail::eval_value_traits
         <supposed_value_traits>
      , detail::identity
         <supposed_value_traits>
      >::type                                   real_value_traits;
   typedef typename detail::get_node_traits
      <supposed_value_traits>::type::node_ptr   node_ptr;
   typedef typename unordered_bucket
      <ValueTraitsOrHookOption>::type           bucket_type;
   typedef typename boost::pointer_to_other
      <node_ptr, bucket_type>::type             implementation_defined;
   /// @endcond
   typedef implementation_defined               type;
};

//! The class template hashtable is an intrusive hash table container, that
//! is used to construct intrusive unordered_set and unordered_multiset containers. The
//! no-throw guarantee holds only, if the Equal object and Hasher don't throw.
//!
//! hashtable is a pseudo-intrusive container: each object to be stored in the
//! container must contain a proper hook, but the container also needs
//! additional auxiliary memory to work: hashtable needs a pointer to an array
//! of type `bucket_type` to be passed in the constructor. This bucket array must
//! have at least the same lifetime as the container. This makes the use of
//! hashtable more complicated than purely intrusive containers.
//! `bucket_type` is default-constructible, copyable and assignable
//!
//! The template parameter \c T is the type to be managed by the container.
//! The user can specify additional options and if no options are provided
//! default options are used.
//!
//! The container supports the following options:
//! \c base_hook<>/member_hook<>/value_traits<>,
//! \c constant_time_size<>, \c size_type<>, \c hash<> and \c equal<> .
//!
//! hashtable only provides forward iterators but it provides 4 iterator types:
//! iterator and const_iterator to navigate through the whole container and
//! local_iterator and const_local_iterator to navigate through the values
//! stored in a single bucket. Local iterators are faster and smaller.
//!
//! It's not recommended to use non constant-time size hashtables because several
//! key functions, like "empty()", become non-constant time functions. Non
//! constant_time size hashtables are mainly provided to support auto-unlink hooks.
//!
//! hashtables, does not make automatic rehashings nor
//! offers functions related to a load factor. Rehashing can be explicitly requested
//! and the user must provide a new bucket array that will be used from that moment.
//!
//! Since no automatic rehashing is done, iterators are never invalidated when
//! inserting or erasing elements. Iterators are only invalidated when rehashing.
#ifdef BOOST_INTRUSIVE_DOXYGEN_INVOKED
template<class T, class ...Options>
#else
template<class Config>
#endif
class hashtable_impl
   :  private detail::data_t<Config>
{
   public:
   typedef typename Config::value_traits                             value_traits;

   /// @cond

   static const bool external_value_traits =
      detail::external_value_traits_is_true<value_traits>::value;
   typedef typename detail::eval_if_c
      < external_value_traits
      , detail::eval_value_traits<value_traits>
      , detail::identity<value_traits>
      >::type                                                        real_value_traits;
   typedef typename Config::bucket_traits                            bucket_traits;
   static const bool external_bucket_traits =
      detail::external_bucket_traits_is_true<bucket_traits>::value;
   typedef typename detail::eval_if_c
      < external_bucket_traits
      , detail::eval_bucket_traits<bucket_traits>
      , detail::identity<bucket_traits>
      >::type                                                        real_bucket_traits;
   typedef typename get_slist_impl
      <typename real_value_traits::node_traits>::type                slist_impl;
   /// @endcond

   typedef typename real_value_traits::pointer                       pointer;
   typedef typename real_value_traits::const_pointer                 const_pointer;
   typedef typename std::iterator_traits<pointer>::value_type        value_type;
   typedef typename std::iterator_traits<pointer>::reference         reference;
   typedef typename std::iterator_traits<const_pointer>::reference   const_reference;
   typedef typename std::iterator_traits<pointer>::difference_type   difference_type;
   typedef typename Config::size_type                                size_type;
   typedef value_type                                                key_type;
   typedef typename Config::equal                                    key_equal;
   typedef typename Config::hash                                     hasher;
   typedef detail::bucket_impl<slist_impl>                           bucket_type;
   typedef typename boost::pointer_to_other
      <pointer, bucket_type>::type                                   bucket_ptr;
   typedef typename slist_impl::iterator                             siterator;
   typedef typename slist_impl::const_iterator                       const_siterator;
   typedef detail::hashtable_iterator<hashtable_impl, false>         iterator;
   typedef detail::hashtable_iterator<hashtable_impl, true>          const_iterator;
   typedef typename real_value_traits::node_traits                        node_traits;
   typedef typename node_traits::node                                node;
   typedef typename boost::pointer_to_other
      <pointer, node>::type                                          node_ptr;
   typedef typename boost::pointer_to_other
      <node_ptr, const node>::type                                   const_node_ptr;
   typedef typename slist_impl::node_algorithms                      node_algorithms;

   static const bool constant_time_size = Config::constant_time_size;
   static const bool stateful_value_traits = detail::store_cont_ptr_on_it<hashtable_impl>::value;
   static const bool store_hash = detail::store_hash_is_true<node_traits>::value;

   /// @cond
   private:
   typedef detail::bool_<store_hash>                                 store_hash_t;
   typedef detail::size_holder<constant_time_size, size_type>        size_traits;
   typedef detail::data_t<Config>                                    base_type;
   typedef detail::transform_iterator
      < typename slist_impl::iterator
      , detail::node_to_value<hashtable_impl, false> >               local_iterator_impl;
   typedef detail::transform_iterator
      < typename slist_impl::iterator
      , detail::node_to_value<hashtable_impl, true> >                const_local_iterator_impl;

   //noncopyable
   hashtable_impl (const hashtable_impl&);
   hashtable_impl operator =(const hashtable_impl&);

   enum { safemode_or_autounlink  = 
            (int)real_value_traits::link_mode == (int)auto_unlink   ||
            (int)real_value_traits::link_mode == (int)safe_link     };

   //Constant-time size is incompatible with auto-unlink hooks!
   BOOST_STATIC_ASSERT(!(constant_time_size && ((int)real_value_traits::link_mode == (int)auto_unlink)));

   static const bool power_2_buckets = Config::power_2_buckets;

   std::size_t from_hash_to_bucket(std::size_t hash_value) const
   {  return from_hash_to_bucket(hash_value, detail::bool_<power_2_buckets>());  }

   std::size_t from_hash_to_bucket(std::size_t hash_value, detail::bool_<false>) const
   {  return hash_value % this->get_real_bucket_traits().bucket_count();  }

   std::size_t from_hash_to_bucket(std::size_t hash_value, detail::bool_<true>) const
   {  return hash_value & (this->get_real_bucket_traits().bucket_count() - 1);   }

   const key_equal &priv_equal() const
   {  return static_cast<const key_equal&>(this->bucket_hash_equal_.get());  }

   key_equal &priv_equal()
   {  return static_cast<key_equal&>(this->bucket_hash_equal_.get());  }

   const real_bucket_traits &get_real_bucket_traits(detail::bool_<false>) const
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_.bucket_traits_;  }

   const real_bucket_traits &get_real_bucket_traits(detail::bool_<true>) const
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_.bucket_traits_.get_bucket_traits(*this);  }

   real_bucket_traits &get_real_bucket_traits(detail::bool_<false>)
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_.bucket_traits_;  }

   real_bucket_traits &get_real_bucket_traits(detail::bool_<true>)
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_.bucket_traits_.get_bucket_traits(*this);  }

   const real_bucket_traits &get_real_bucket_traits() const
   {  return this->get_real_bucket_traits(detail::bool_<external_bucket_traits>());  }

   real_bucket_traits &get_real_bucket_traits()
   {  return this->get_real_bucket_traits(detail::bool_<external_bucket_traits>());  }

   const hasher &priv_hasher() const
   {  return static_cast<const hasher&>(this->bucket_hash_equal_.bucket_hash.get());  }

   hasher &priv_hasher()
   {  return static_cast<hasher&>(this->bucket_hash_equal_.bucket_hash.get());  }

   bucket_ptr priv_buckets() const
   {  return this->get_real_bucket_traits().bucket_begin();  }

   size_type priv_buckets_len() const
   {  return this->get_real_bucket_traits().bucket_count();  }

   static node_ptr uncast(const_node_ptr ptr)
   {
      return node_ptr(const_cast<node*>(detail::get_pointer(ptr)));
   }

   node &from_value_to_node(value_type &v)
   {  return *this->get_real_value_traits().to_node_ptr(v);  }

   const node &from_value_to_node(const value_type &v) const
   {  return *this->get_real_value_traits().to_node_ptr(v);  }

   size_traits &priv_size_traits()
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_;  }

   const size_traits &priv_size_traits() const
   {  return this->bucket_hash_equal_.bucket_hash.bucket_plus_size_;  }

   struct insert_commit_data_impl
   {
      size_type hash;
   };
   /// @endcond

   public:

   class local_iterator
      :  public local_iterator_impl
   {
      public:
      local_iterator()
      {}

      local_iterator(siterator sit, const hashtable_impl *cont)
         :  local_iterator_impl(sit, cont)
      {}
   };

   class const_local_iterator
      :  public const_local_iterator_impl
   {
      public:
      const_local_iterator()
      {}

      const_local_iterator(siterator sit, const hashtable_impl *cont)
         :  const_local_iterator_impl(sit, cont)
      {}
   };

   typedef insert_commit_data_impl insert_commit_data;

   /// @cond

   const real_value_traits &get_real_value_traits(detail::bool_<false>) const
   {  return *this;  }

   const real_value_traits &get_real_value_traits(detail::bool_<true>) const
   {  return base_type::get_value_traits(*this);  }

   real_value_traits &get_real_value_traits(detail::bool_<false>)
   {  return *this;  }

   real_value_traits &get_real_value_traits(detail::bool_<true>)
   {  return base_type::get_value_traits(*this);  }

   /// @endcond

   public:

   const real_value_traits &get_real_value_traits() const
   {  return this->get_real_value_traits(detail::bool_<external_value_traits>());  }

   real_value_traits &get_real_value_traits()
   {  return this->get_real_value_traits(detail::bool_<external_value_traits>());  }

   //! <b>Requires</b>: buckets must not be being used by any other resource.
   //!
   //! <b>Effects</b>: Constructs an empty unordered_set, storing a reference
   //!   to the bucket array and copies of the key_hasher and equal_func functors.
   //!   
   //! <b>Complexity</b>: Constant. 
   //! 
   //! <b>Throws</b>: If value_traits::node_traits::node
   //!   constructor throws (this does not happen with predefined Boost.Intrusive hooks)
   //!   or the copy constructor or invocation of hash_func or equal_func throws. 
   //!
   //! <b>Notes</b>: buckets array must be disposed only after
   //!   *this is disposed.
   hashtable_impl ( const bucket_traits &b_traits
                  , const hasher & hash_func = hasher()
                  , const key_equal &equal_func = key_equal()
                  , const value_traits &v_traits = value_traits()) 
      :  base_type(b_traits, hash_func, equal_func, v_traits)
   {
      priv_clear_buckets();
      this->priv_size_traits().set_size(size_type(0));
      BOOST_INTRUSIVE_INVARIANT_ASSERT(this->priv_buckets_len() != 0);
      //Check power of two bucket array if the option is activated
      BOOST_INTRUSIVE_INVARIANT_ASSERT
      (!power_2_buckets || (0 == (this->priv_buckets_len() & (this->priv_buckets_len()-1))));
   }

   //! <b>Effects</b>: Detaches all elements from this. The objects in the unordered_set 
   //!   are not deleted (i.e. no destructors are called).
   //! 
   //! <b>Complexity</b>: Linear to the number of elements in the unordered_set, if 
   //!   it's a safe-mode or auto-unlink value. Otherwise constant.
   //! 
   //! <b>Throws</b>: Nothing.
   ~hashtable_impl() 
   {  this->clear(); }

   //! <b>Effects</b>: Returns an iterator pointing to the beginning of the unordered_set.
   //! 
   //! <b>Complexity</b>: Amortized constant time.
   //!   Worst case (empty unordered_set): O(this->bucket_count())
   //! 
   //! <b>Throws</b>: Nothing.
   iterator begin()
   {
      size_type bucket_num;
      return iterator(this->priv_begin(bucket_num), this);
   }

   //! <b>Effects</b>: Returns a const_iterator pointing to the beginning
   //!   of the unordered_set.
   //!
   //! <b>Complexity</b>: Amortized constant time.
   //!   Worst case (empty unordered_set): O(this->bucket_count())
   //! 
   //! <b>Throws</b>: Nothing.
   const_iterator begin() const
   {  return this->cbegin();  }

   //! <b>Effects</b>: Returns a const_iterator pointing to the beginning
   //!   of the unordered_set.
   //!
   //! <b>Complexity</b>: Amortized constant time.
   //!   Worst case (empty unordered_set): O(this->bucket_count())
   //! 
   //! <b>Throws</b>: Nothing.
   const_iterator cbegin() const
   {
      size_type bucket_num;
      return const_iterator(this->priv_begin(bucket_num), this);
   }

   //! <b>Effects</b>: Returns an iterator pointing to the end of the unordered_set.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   iterator end()
   {  return iterator(invalid_local_it(this->get_real_bucket_traits()), 0);   }

   //! <b>Effects</b>: Returns a const_iterator pointing to the end of the unordered_set.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   const_iterator end() const
   {  return this->cend(); }

   //! <b>Effects</b>: Returns a const_iterator pointing to the end of the unordered_set.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   const_iterator cend() const
   {  return const_iterator(invalid_local_it(this->get_real_bucket_traits()), 0);  }

   //! <b>Effects</b>: Returns the hasher object used by the unordered_set.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If hasher copy-constructor throws.
   hasher hash_function() const
   {  return this->priv_hasher();  }

   //! <b>Effects</b>: Returns the key_equal object used by the unordered_set.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If key_equal copy-constructor throws.
   key_equal key_eq() const
   {  return this->priv_equal();   }

   //! <b>Effects</b>: Returns true is the container is empty.
   //! 
   //! <b>Complexity</b>: if constant_time_size is false, average constant time
   //!   (worst case, with empty() == true): O(this->bucket_count()).
   //!   Otherwise constant.
   //! 
   //! <b>Throws</b>: Nothing.
   bool empty() const
   {
      if(constant_time_size){
         return !this->size();
      }
      else{
         size_type buckets_len = this->priv_buckets_len();
         const bucket_type *b = detail::get_pointer(this->priv_buckets());
         for (size_type n = 0; n < buckets_len; ++n, ++b){
            if(!b->empty()){
               return false;
            }
         }
         return true;
      }
   }

   //! <b>Effects</b>: Returns the number of elements stored in the unordered_set.
   //! 
   //! <b>Complexity</b>: Linear to elements contained in *this if
   //!   constant_time_size is false. Constant-time otherwise.
   //! 
   //! <b>Throws</b>: Nothing.
   size_type size() const
   {
      if(constant_time_size)
         return this->priv_size_traits().get_size();
      else{
         size_type len = 0;
         size_type buckets_len = this->priv_buckets_len();
         const bucket_type *b = detail::get_pointer(this->priv_buckets());
         for (size_type n = 0; n < buckets_len; ++n, ++b){
            len += b->size();
         }
         return len;
      }
   }

   //! <b>Requires</b>: the hasher and the equality function unqualified swap
   //!   call should not throw.
   //! 
   //! <b>Effects</b>: Swaps the contents of two unordered_sets.
   //!   Swaps also the contained bucket array and equality and hasher functors.
   //! 
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Throws</b>: If the swap() call for the comparison or hash functors
   //!   found using ADL throw. Basic guarantee.
   void swap(hashtable_impl& other)
   {
      using std::swap;
      //These can throw
      swap(this->priv_equal(), other.priv_equal());
      swap(this->priv_hasher(), other.priv_hasher());
      //These can't throw
      swap(this->get_real_bucket_traits(), other.get_real_bucket_traits());
      if(constant_time_size){
         size_type backup = this->priv_size_traits().get_size();
         this->priv_size_traits().set_size(other.priv_size_traits().get_size());
         other.priv_size_traits().set_size(backup);
      }
   }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //!
   //! <b>Effects</b>: Erases all the elements from *this
   //!   calling Disposer::operator()(pointer), clones all the 
   //!   elements from src calling Cloner::operator()(const_reference )
   //!   and inserts them on *this.
   //!
   //!   If cloner throws, all cloned elements are unlinked and disposed
   //!   calling Disposer::operator()(pointer).
   //!   
   //! <b>Complexity</b>: Linear to erased plus inserted elements.
   //! 
   //! <b>Throws</b>: If cloner throws. Basic guarantee.
   template <class Cloner, class Disposer>
   void clone_from(const hashtable_impl &src, Cloner cloner, Disposer disposer)
   {
      this->clear_and_dispose(disposer);
      if(!constant_time_size || !src.empty()){
         const size_type src_bucket_count = src.bucket_count();
         const size_type dst_bucket_count = this->bucket_count();
         //Check power of two bucket array if the option is activated
         BOOST_INTRUSIVE_INVARIANT_ASSERT
         (!power_2_buckets || (0 == (src_bucket_count & (src_bucket_count-1))));
         BOOST_INTRUSIVE_INVARIANT_ASSERT
         (!power_2_buckets || (0 == (dst_bucket_count & (dst_bucket_count-1))));

         //If src bucket count is bigger or equal, structural copy is possible
         if(src_bucket_count >= dst_bucket_count){
            //First clone the first ones
            const bucket_ptr src_buckets = src.priv_buckets();
            const bucket_ptr dst_buckets = this->priv_buckets();
            size_type constructed;
            BOOST_INTRUSIVE_TRY{
               for( constructed = 0
                  ; constructed < dst_bucket_count
                  ; ++constructed){
                  dst_buckets[constructed].clone_from
                     ( src_buckets[constructed]
                     , detail::node_cloner<Cloner, hashtable_impl>(cloner, this)
                     , detail::node_disposer<Disposer, hashtable_impl>(disposer, this)
                     );
               }
               if(src_bucket_count != dst_bucket_count){
                  //Now insert the remaining ones using the modulo trick
                  for(//"constructed" comes from the previous loop
                     ; constructed < src_bucket_count
                     ; ++constructed){
                     bucket_type &dst_b = (power_2_buckets)
                        ?  dst_buckets[constructed & (dst_bucket_count-1)]
                        :  dst_buckets[constructed % dst_bucket_count];
                     bucket_type &src_b = src_buckets[constructed];
                     for( siterator b(src_b.begin()), e(src_b.end())
                        ; b != e
                        ; ++b){
                        dst_b.push_front(*detail::node_cloner<Cloner, hashtable_impl>
                              (cloner, this)(b.pointed_node()));
                     }
                  }
               }
            }
            BOOST_INTRUSIVE_CATCH(...){
               while(constructed--){
                  dst_buckets[constructed].clear_and_dispose
                  (detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
               }
               BOOST_INTRUSIVE_RETHROW;
            }
            BOOST_INTRUSIVE_CATCH_END
            this->priv_size_traits().set_size(src.priv_size_traits().get_size());
         }
         else{
            //Unlike previous cloning algorithm, this can throw
            //if cloner, the hasher or comparison functor throw
            const_iterator b(src.begin()), e(src.end());
            BOOST_INTRUSIVE_TRY{
               for(; b != e; ++b){
                  this->insert_equal(*cloner(*b));
               }
            }
            BOOST_INTRUSIVE_CATCH(...){
               this->clear_and_dispose(disposer);
               BOOST_INTRUSIVE_RETHROW;
            }
            BOOST_INTRUSIVE_CATCH_END
         }
      }
   }

   iterator insert_equal(reference value)
   {
      size_type bucket_num, hash_value;
      siterator it = this->priv_find
         (value, this->priv_hasher(), this->priv_equal(), bucket_num, hash_value);
      bucket_type &b = this->priv_buckets()[bucket_num];
      if(it == invalid_local_it(this->get_real_bucket_traits())){
         it = b.before_begin();
      }
      node_ptr n = node_ptr(&from_value_to_node(value));
      this->priv_store_hash(n, hash_value, store_hash_t());
      if(safemode_or_autounlink)
         BOOST_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT(node_algorithms::unique(n));
      this->priv_size_traits().increment();
      return iterator(b.insert_after(it, *n), this);
   }

   template<class Iterator>
   void insert_equal(Iterator b, Iterator e)
   {
      for (; b != e; ++b)
         this->insert_equal(*b);
   }

   //! <b>Requires</b>: value must be an lvalue
   //! 
   //! <b>Effects</b>: Tries to inserts value into the unordered_set.
   //!
   //! <b>Returns</b>: If the value
   //!   is not already present inserts it and returns a pair containing the
   //!   iterator to the new value and true. If there is an equivalent value
   //!   returns a pair containing an iterator to the already present value
   //!   and false.
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws. Strong guarantee.
   //! 
   //! <b>Note</b>: Does not affect the validity of iterators and references.
   //!   No copy-constructors are called.
   std::pair<iterator, bool> insert_unique(reference value)
   {
      insert_commit_data commit_data;
      std::pair<iterator, bool> ret = this->insert_unique_check
         (value, this->priv_hasher(), this->priv_equal(), commit_data);
      if(!ret.second)
         return ret;
      return std::pair<iterator, bool> 
         (this->insert_unique_commit(value, commit_data), true);
   }

   //! <b>Requires</b>: Dereferencing iterator must yield an lvalue 
   //!   of type value_type.
   //! 
   //! <b>Effects</b>: Equivalent to this->insert(t) for each element in [b, e).
   //! 
   //! <b>Complexity</b>: Average case O(N), where N is std::distance(b, e).
   //!   Worst case O(N*this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws. Basic guarantee.
   //! 
   //! <b>Note</b>: Does not affect the validity of iterators and references.
   //!   No copy-constructors are called.
   template<class Iterator>
   void insert_unique(Iterator b, Iterator e)
   {
      for (; b != e; ++b)
         this->insert_unique(*b);
   }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //! 
   //! <b>Effects</b>: Checks if a value can be inserted in the unordered_set, using
   //!   a user provided key instead of the value itself.
   //!
   //! <b>Returns</b>: If there is an equivalent value
   //!   returns a pair containing an iterator to the already present value
   //!   and false. If the value can be inserted returns true in the returned
   //!   pair boolean and fills "commit_data" that is meant to be used with
   //!   the "insert_commit" function.
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //!
   //! <b>Throws</b>: If hash_func or equal_func throw. Strong guarantee.
   //! 
   //! <b>Notes</b>: This function is used to improve performance when constructing
   //!   a value_type is expensive: if there is an equivalent value
   //!   the constructed object must be discarded. Many times, the part of the
   //!   node that is used to impose the hash or the equality is much cheaper to
   //!   construct than the value_type and this function offers the possibility to
   //!   use that the part to check if the insertion will be successful.
   //!
   //!   If the check is successful, the user can construct the value_type and use
   //!   "insert_commit" to insert the object in constant-time.
   //!
   //!   "commit_data" remains valid for a subsequent "insert_commit" only if no more
   //!   objects are inserted or erased from the unordered_set.
   //!
   //!   After a successful rehashing insert_commit_data remains valid.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   std::pair<iterator, bool> insert_unique_check
      ( const KeyType &key
      , KeyHasher hash_func
      , KeyValueEqual equal_func
      , insert_commit_data &commit_data)
   {
      size_type bucket_num;
      siterator prev_pos =
         this->priv_find(key, hash_func, equal_func, bucket_num, commit_data.hash);
      bool success = prev_pos == invalid_local_it(this->get_real_bucket_traits());
      if(success){
         prev_pos = this->priv_buckets()[bucket_num].before_begin();
      }
      return std::pair<iterator, bool>(iterator(prev_pos, this),success);
   }

   //! <b>Requires</b>: value must be an lvalue of type value_type. commit_data
   //!   must have been obtained from a previous call to "insert_check".
   //!   No objects should have been inserted or erased from the unordered_set between
   //!   the "insert_check" that filled "commit_data" and the call to "insert_commit".
   //! 
   //! <b>Effects</b>: Inserts the value in the unordered_set using the information obtained
   //!   from the "commit_data" that a previous "insert_check" filled.
   //!
   //! <b>Returns</b>: An iterator to the newly inserted object.
   //! 
   //! <b>Complexity</b>: Constant time.
   //!
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Notes</b>: This function has only sense if a "insert_check" has been
   //!   previously executed to fill "commit_data". No value should be inserted or
   //!   erased between the "insert_check" and "insert_commit" calls.
   //!
   //!   After a successful rehashing insert_commit_data remains valid.
   iterator insert_unique_commit(reference value, const insert_commit_data &commit_data)
   {
      size_type bucket_num = from_hash_to_bucket(commit_data.hash);
      bucket_type &b = this->priv_buckets()[bucket_num];
      this->priv_size_traits().increment();
      node_ptr n = node_ptr(&from_value_to_node(value));
      this->priv_store_hash(n, commit_data.hash, store_hash_t());
      if(safemode_or_autounlink)
         BOOST_INTRUSIVE_SAFE_HOOK_DEFAULT_ASSERT(node_algorithms::unique(n));
      return iterator( b.insert_after(b.before_begin(), *n), this);
   }

   //! <b>Effects</b>: Erases the element pointed to by i. 
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased element. No destructors are called.
   void erase(const_iterator i)
   {  this->erase_and_dispose(i, detail::null_disposer());  }

   //! <b>Effects</b>: Erases the range pointed to by b end e. 
   //! 
   //! <b>Complexity</b>: Average case O(std::distance(b, e)),
   //!   worst case O(this->size()).
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   void erase(const_iterator b, const_iterator e)
   {  this->erase_and_dispose(b, e, detail::null_disposer());  }

   //! <b>Effects</b>: Erases all the elements with the given value.
   //! 
   //! <b>Returns</b>: The number of erased elements.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws. 
   //!   Basic guarantee.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   size_type erase(const_reference value)
   {  return this->erase(value, this->priv_hasher(), this->priv_equal());  }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Erases all the elements that have the same hash and
   //!   compare equal with the given key.
   //! 
   //! <b>Returns</b>: The number of erased elements.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or equal_func throw. Basic guarantee.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   size_type erase(const KeyType& key, KeyHasher hash_func, KeyValueEqual equal_func)
   {  return this->erase_and_dispose(key, hash_func, equal_func, detail::null_disposer()); }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //!
   //! <b>Effects</b>: Erases the element pointed to by i. 
   //!   Disposer::operator()(pointer) is called for the removed element.
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators 
   //!    to the erased elements.
   template<class Disposer>
   void erase_and_dispose(const_iterator i, Disposer disposer)
   {
      siterator to_erase(i.slist_it());
      bucket_ptr f(priv_buckets()), l(f + priv_buckets_len());
      bucket_type &b = this->priv_buckets()[bucket_type::get_bucket_num(to_erase, *f, *l)];
      b.erase_after_and_dispose
         (b.previous(to_erase), detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
      this->priv_size_traits().decrement();
   }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //!
   //! <b>Effects</b>: Erases the range pointed to by b end e.
   //!   Disposer::operator()(pointer) is called for the removed elements.
   //! 
   //! <b>Complexity</b>: Average case O(std::distance(b, e)),
   //!   worst case O(this->size()).
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators
   //!    to the erased elements.
   template<class Disposer>
   void erase_and_dispose(const_iterator b, const_iterator e, Disposer disposer)
   {
      if(b == e)  return;

      //Get the bucket number and local iterator for both iterators
      bucket_ptr f(priv_buckets()), l(f + priv_buckets_len());
      size_type first_bucket_num = bucket_type::get_bucket_num(b.slist_it(), *f, *l);

      siterator before_first_local_it
         = priv_buckets()[first_bucket_num].previous(b.slist_it());
      size_type last_bucket_num;
      siterator last_local_it;

      //For the end iterator, we will assign the end iterator
      //of the last bucket
      if(e == end()){
         last_bucket_num   = this->bucket_count() - 1;
         last_local_it     = priv_buckets()[last_bucket_num].end();
      }
      else{
         last_local_it     = e.slist_it();
         last_bucket_num  = bucket_type::get_bucket_num(last_local_it, *f, *l);
      }

      const bucket_ptr buckets = priv_buckets();
      //First erase the nodes of the first bucket
      {
         bucket_type &first_b = buckets[first_bucket_num];
         siterator nxt(before_first_local_it); ++nxt;
         siterator end = first_b.end();
         while(nxt != end){
            nxt = first_b.erase_after_and_dispose
               ( before_first_local_it
               , detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
            this->priv_size_traits().decrement();
         }
      }

      //Now fully clear the intermediate buckets
      for(size_type i = first_bucket_num+1; i < last_bucket_num; ++i){
         bucket_type &b = buckets[i];
         if(b.empty())
            continue;
         siterator b_begin(b.before_begin());
         siterator nxt(b_begin); ++nxt;
         siterator end = b.end();
         while(nxt != end){
            nxt = b.erase_after_and_dispose
               (b_begin, detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
            this->priv_size_traits().decrement();
         }
      }

      //Now erase nodes from the last bucket
      {
         bucket_type &last_b = buckets[last_bucket_num];
         siterator b_begin(last_b.before_begin());
         siterator nxt(b_begin); ++nxt;
         while(nxt != last_local_it){
            nxt = last_b.erase_after_and_dispose
               (b_begin, detail::node_disposer<Disposer, hashtable_impl>
                              (disposer, this));
            this->priv_size_traits().decrement();
         }
      }
   }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //!
   //! <b>Effects</b>: Erases all the elements with the given value.
   //!   Disposer::operator()(pointer) is called for the removed elements.
   //! 
   //! <b>Returns</b>: The number of erased elements.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   //!   Basic guarantee.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   template<class Disposer>
   size_type erase_and_dispose(const_reference value, Disposer disposer)
   {  return this->erase_and_dispose(value, priv_hasher(), priv_equal(), disposer);   }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //!
   //! <b>Effects</b>: Erases all the elements with the given key.
   //!   according to the comparison functor "equal_func".
   //!   Disposer::operator()(pointer) is called for the removed elements.
   //!
   //! <b>Returns</b>: The number of erased elements.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or equal_func throw. Basic guarantee.
   //! 
   //! <b>Note</b>: Invalidates the iterators
   //!    to the erased elements.
   template<class KeyType, class KeyHasher, class KeyValueEqual, class Disposer>
   size_type erase_and_dispose(const KeyType& key, KeyHasher hash_func
                              ,KeyValueEqual equal_func, Disposer disposer)
   {
      size_type count(0);

      if(constant_time_size && this->empty()){
         return 0;
      }

      bucket_type &b = this->priv_buckets()[from_hash_to_bucket(hash_func(key))];
      siterator it    = b.begin();
      siterator prev  = b.before_begin();

      bool found = false;
      //Find equal value
      while(it != b.end()){
         const value_type &v = 
            *this->get_real_value_traits().to_value_ptr(it.pointed_node());
         if(equal_func(key, v)){
            found = true;
            break;
         }
         ++prev;
         ++it;
      }
   
      if(!found)
         return 0;

      //If found erase all equal values
      for(siterator end = b.end(); it != end && 
           equal_func(key, *this->get_real_value_traits().to_value_ptr(it.pointed_node()))
         ; ++count){
         it = b.erase_after_and_dispose
            (prev, detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
         this->priv_size_traits().decrement();
      }
      return count;
   }

   //! <b>Effects</b>: Erases all of the elements. 
   //! 
   //! <b>Complexity</b>: Linear to the number of elements on the container.
   //!   if it's a safe-mode or auto-unlink value_type. Constant time otherwise.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   void clear()
   {
      priv_clear_buckets();
      this->priv_size_traits().set_size(size_type(0));
   }

   //! <b>Requires</b>: Disposer::operator()(pointer) shouldn't throw.
   //! 
   //! <b>Effects</b>: Erases all of the elements. 
   //! 
   //! <b>Complexity</b>: Linear to the number of elements on the container.
   //!   Disposer::operator()(pointer) is called for the removed elements.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: Invalidates the iterators (but not the references)
   //!    to the erased elements. No destructors are called.
   template<class Disposer>
   void clear_and_dispose(Disposer disposer)
   {
      if(!constant_time_size || !this->empty()){
         size_type num_buckets = this->bucket_count();
         bucket_ptr b = this->priv_buckets();
         for(; num_buckets--; ++b){
            b->clear_and_dispose
               (detail::node_disposer<Disposer, hashtable_impl>(disposer, this));
         }
         this->priv_size_traits().set_size(size_type(0));
      }
   }

   //! <b>Effects</b>: Returns the number of contained elements with the given value
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   size_type count(const_reference value) const
   {  return this->count(value, this->priv_hasher(), this->priv_equal());  }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Returns the number of contained elements with the given key
   //!
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or equal throw.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   size_type count(const KeyType &key, const KeyHasher &hash_func, const KeyValueEqual &equal_func) const
   {
      size_type bucket_n1, bucket_n2, count;
      this->priv_equal_range(key, hash_func, equal_func, bucket_n1, bucket_n2, count);
      return count;
   }

   //! <b>Effects</b>: Finds an iterator to the first element is equal to
   //!   "value" or end() if that element does not exist.
   //!
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   iterator find(const_reference value)
   {  return this->find(value, this->priv_hasher(), this->priv_equal());   }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Finds an iterator to the first element whose key is 
   //!   "key" according to the given hash and equality functor or end() if
   //!   that element does not exist.
   //!
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or equal_func throw.
   //!
   //! <b>Note</b>: This function is used when constructing a value_type
   //!   is expensive and the value_type can be compared with a cheaper
   //!   key type. Usually this key is part of the value_type.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   iterator find(const KeyType &key, KeyHasher hash_func, KeyValueEqual equal_func)
   {
      size_type bucket_n, hash;
      siterator local_it = this->priv_find(key, hash_func, equal_func, bucket_n, hash);
      return iterator(local_it, this);
   }

   //! <b>Effects</b>: Finds a const_iterator to the first element whose key is 
   //!   "key" or end() if that element does not exist.
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   const_iterator find(const_reference value) const
   {  return this->find(value, this->priv_hasher(), this->priv_equal());   }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Finds an iterator to the first element whose key is 
   //!   "key" according to the given hasher and equality functor or end() if
   //!   that element does not exist.
   //! 
   //! <b>Complexity</b>: Average case O(1), worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or equal_func throw.
   //!
   //! <b>Note</b>: This function is used when constructing a value_type
   //!   is expensive and the value_type can be compared with a cheaper
   //!   key type. Usually this key is part of the value_type.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   const_iterator find
      (const KeyType &key, KeyHasher hash_func, KeyValueEqual equal_func) const
   {
      size_type bucket_n, hash_value;
      siterator sit = this->priv_find(key, hash_func, equal_func, bucket_n, hash_value);
      return const_iterator(sit, this);
   }

   //! <b>Effects</b>: Returns a range containing all elements with values equivalent
   //!   to value. Returns std::make_pair(this->end(), this->end()) if no such 
   //!   elements exist.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)). Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   std::pair<iterator,iterator> equal_range(const_reference value)
   {  return this->equal_range(value, this->priv_hasher(), this->priv_equal());  }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Returns a range containing all elements with equivalent
   //!   keys. Returns std::make_pair(this->end(), this->end()) if no such 
   //!   elements exist.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(key, hash_func, equal_func)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If hash_func or the equal_func throw.
   //!
   //! <b>Note</b>: This function is used when constructing a value_type
   //!   is expensive and the value_type can be compared with a cheaper
   //!   key type. Usually this key is part of the value_type.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   std::pair<iterator,iterator> equal_range
      (const KeyType &key, KeyHasher hash_func, KeyValueEqual equal_func)
   {
      size_type bucket_n1, bucket_n2, count;
      std::pair<siterator, siterator> ret = this->priv_equal_range
         (key, hash_func, equal_func, bucket_n1, bucket_n2, count);
      return std::pair<iterator, iterator>
         (iterator(ret.first, this), iterator(ret.second, this));
   }

   //! <b>Effects</b>: Returns a range containing all elements with values equivalent
   //!   to value. Returns std::make_pair(this->end(), this->end()) if no such 
   //!   elements exist.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(value)). Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the internal hasher or the equality functor throws.
   std::pair<const_iterator, const_iterator>
      equal_range(const_reference value) const
   {  return this->equal_range(value, this->priv_hasher(), this->priv_equal());  }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //!   "equal_func" must be a equality function that induces 
   //!   the same equality as key_equal. The difference is that
   //!   "equal_func" compares an arbitrary key with the contained values.
   //!
   //! <b>Effects</b>: Returns a range containing all elements with equivalent
   //!   keys. Returns std::make_pair(this->end(), this->end()) if no such 
   //!   elements exist.
   //! 
   //! <b>Complexity</b>: Average case O(this->count(key, hash_func, equal_func)).
   //!   Worst case O(this->size()).
   //! 
   //! <b>Throws</b>: If the hasher or equal_func throw.
   //!
   //! <b>Note</b>: This function is used when constructing a value_type
   //!   is expensive and the value_type can be compared with a cheaper
   //!   key type. Usually this key is part of the value_type.
   template<class KeyType, class KeyHasher, class KeyValueEqual>
   std::pair<const_iterator,const_iterator> equal_range
      (const KeyType &key, KeyHasher hash_func, KeyValueEqual equal_func) const
   {
      size_type bucket_n1, bucket_n2, count;
      std::pair<siterator, siterator> ret =
         this->priv_equal_range(key, hash_func, equal_func, bucket_n1, bucket_n2, count);
      return std::pair<const_iterator, const_iterator>
         (const_iterator(ret.first, this), const_iterator(ret.second, this));
   }

   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid iterator belonging to the unordered_set
   //!   that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If the internal hash function throws.
   iterator iterator_to(reference value)
   {
      return iterator(bucket_type::s_iterator_to(from_value_to_node(value)), this);
   }

   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid const_iterator belonging to the
   //!   unordered_set that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If the internal hash function throws.
   const_iterator iterator_to(const_reference value) const
   {
      return const_iterator(bucket_type::s_iterator_to(from_value_to_node(const_cast<reference>(value))), this);
   }




   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid local_iterator belonging to the unordered_set
   //!   that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: This static function is available only if the <i>value traits</i>
   //!   is stateless.
   static local_iterator s_local_iterator_to(reference value)
   {
      BOOST_STATIC_ASSERT((!stateful_value_traits));
      siterator sit = bucket_type::s_iterator_to(((hashtable_impl*)0)->from_value_to_node(value));
      return local_iterator(sit, (hashtable_impl*)0);  
   }

   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid const_local_iterator belonging to
   //!   the unordered_set that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>: This static function is available only if the <i>value traits</i>
   //!   is stateless.
   static const_local_iterator s_local_iterator_to(const_reference value)
   {
      BOOST_STATIC_ASSERT((!stateful_value_traits));
      siterator sit = bucket_type::s_iterator_to(((hashtable_impl*)0)->from_value_to_node(const_cast<value_type&>(value)));
      return const_local_iterator(sit, (hashtable_impl*)0);  
   }

   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid local_iterator belonging to the unordered_set
   //!   that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   local_iterator local_iterator_to(reference value)
   {
      siterator sit = bucket_type::s_iterator_to(this->from_value_to_node(value));
      return local_iterator(sit, this);  
   }

   //! <b>Requires</b>: value must be an lvalue and shall be in a unordered_set of
   //!   appropriate type. Otherwise the behavior is undefined.
   //! 
   //! <b>Effects</b>: Returns: a valid const_local_iterator belonging to
   //!   the unordered_set that points to the value
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   const_local_iterator local_iterator_to(const_reference value) const
   {
      siterator sit = bucket_type::s_iterator_to
         (const_cast<node &>(this->from_value_to_node(value)));
      return const_local_iterator(sit, this);  
   }

   //! <b>Effects</b>: Returns the number of buckets passed in the constructor
   //!   or the last rehash function.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   size_type bucket_count() const
   {  return this->priv_buckets_len();   }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns the number of elements in the nth bucket.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   size_type bucket_size(size_type n) const
   {  return this->priv_buckets()[n].size();   }

   //! <b>Effects</b>: Returns the index of the bucket in which elements
   //!   with keys equivalent to k would be found, if any such element existed.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If the hash functor throws.
   //!
   //! <b>Note</b>: the return value is in the range [0, this->bucket_count()).
   size_type bucket(const key_type& k)  const
   {  return this->bucket(k, this->priv_hasher());   }

   //! <b>Requires</b>: "hash_func" must be a hash function that induces 
   //!   the same hash values as the stored hasher. The difference is that
   //!   "hash_func" hashes the given key instead of the value_type.
   //!
   //! <b>Effects</b>: Returns the index of the bucket in which elements
   //!   with keys equivalent to k would be found, if any such element existed.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: If hash_func throws.
   //!
   //! <b>Note</b>: the return value is in the range [0, this->bucket_count()).
   template<class KeyType, class KeyHasher>
   size_type bucket(const KeyType& k, const KeyHasher &hash_func)  const
   {  return from_hash_to_bucket(hash_func(k));   }

   //! <b>Effects</b>: Returns the bucket array pointer passed in the constructor
   //!   or the last rehash function.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   bucket_ptr bucket_pointer() const
   {  return this->priv_buckets();   }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a local_iterator pointing to the beginning
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket. 
   local_iterator begin(size_type n)
   {  return local_iterator(this->priv_buckets()[n].begin(), this);  }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a const_local_iterator pointing to the beginning
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket. 
   const_local_iterator begin(size_type n) const
   {  return this->cbegin(n);  }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a const_local_iterator pointing to the beginning
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket. 
   const_local_iterator cbegin(size_type n) const
   {
      siterator sit = const_cast<bucket_type&>(this->priv_buckets()[n]).begin();
      return const_local_iterator(sit, this);
   }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a local_iterator pointing to the end
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket. 
   local_iterator end(size_type n)
   {  return local_iterator(this->priv_buckets()[n].end(), this);  }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a const_local_iterator pointing to the end
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket.
   const_local_iterator end(size_type n) const
   {  return this->cend(n);  }

   //! <b>Requires</b>: n is in the range [0, this->bucket_count()).
   //!
   //! <b>Effects</b>: Returns a const_local_iterator pointing to the end
   //!   of the sequence stored in the bucket n.
   //! 
   //! <b>Complexity</b>: Constant.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Note</b>:  [this->begin(n), this->end(n)) is a valid range
   //!   containing all of the elements in the nth bucket. 
   const_local_iterator cend(size_type n) const
   {  return const_local_iterator(const_cast<bucket_type&>(this->priv_buckets()[n]).end(), this);  }

   //! <b>Requires</b>: new_buckets must be a pointer to a new bucket array
   //!   or the same as the old bucket array. new_size is the length of the
   //!   the array pointed by new_buckets. If new_buckets == this->bucket_pointer()
   //!   n can be bigger or smaller than this->bucket_count().
   //!
   //! <b>Effects</b>: Updates the internal reference with the new bucket erases
   //!   the values from the old bucket and inserts then in the new one. 
   //! 
   //! <b>Complexity</b>: Average case linear in this->size(), worst case quadratic.
   //! 
   //! <b>Throws</b>: If the hasher functor throws. Basic guarantee.
   void rehash(const bucket_traits &new_bucket_traits)
   {
      bucket_ptr new_buckets     = new_bucket_traits.bucket_begin();
      size_type  new_buckets_len = new_bucket_traits.bucket_count();
      bucket_ptr old_buckets     = this->priv_buckets();
      size_type  old_buckets_len = this->priv_buckets_len();
      //Check power of two bucket array if the option is activated      
      BOOST_INTRUSIVE_INVARIANT_ASSERT
      (!power_2_buckets || (0 == (new_buckets_len & (new_buckets_len-1u))));

      BOOST_INTRUSIVE_TRY{
         size_type n = 0;
         const bool same_buffer = old_buckets == new_buckets;
         //If the new bucket length is a common factor
         //of the old one we can avoid hash calculations.
         const bool fast_shrink = (old_buckets_len > new_buckets_len) && 
            (power_2_buckets ||(old_buckets_len % new_buckets_len) == 0);
         //If we are shrinking the same bucket array and it's
         //is a fast shrink, just rehash the last nodes
         if(same_buffer && fast_shrink){
            n = new_buckets_len;
         }

         //Iterate through nodes
         for(; n < old_buckets_len; ++n){
            bucket_type &old_bucket = old_buckets[n];

            if(!fast_shrink){
               siterator before_i(old_bucket.before_begin());
               siterator end(old_bucket.end());
               siterator i(old_bucket.begin());
               for(;i != end; ++i){
                  const value_type &v = *this->get_real_value_traits().to_value_ptr(i.pointed_node());
                  const std::size_t hash_value = this->priv_hash_when_rehashing(v, store_hash_t());
                  const size_type new_n = (power_2_buckets)
                     ? (hash_value & (new_buckets_len-1)) : (hash_value % new_buckets_len);
                  //If this is a buffer expansion don't move if it's not necessary
                  if(same_buffer && new_n == n){
                     ++before_i;
                  }
                  else{
                     bucket_type &new_b = new_buckets[new_n];
                     new_b.splice_after(new_b.before_begin(), old_bucket, before_i);
                     i = before_i;
                  }
               }
            }
            else{
               const size_type new_n = (power_2_buckets)
                  ?  (n & (new_buckets_len-1))
                  :  (n % new_buckets_len);
               bucket_type &new_b = new_buckets[new_n];
               new_b.splice_after(new_b.before_begin(), old_bucket);
            }
         }

         this->get_real_bucket_traits()= new_bucket_traits;
      }
      BOOST_INTRUSIVE_CATCH(...){
         for(size_type n = 0; n < new_buckets_len; ++n){
            if(safemode_or_autounlink){
               new_buckets[n].clear_and_dispose
                  (detail::init_disposer<node_algorithms>());
               old_buckets[n].clear_and_dispose
                  (detail::init_disposer<node_algorithms>());
            }
            else{
               new_buckets[n].clear();
               old_buckets[n].clear();
            }
         }
         this->priv_size_traits().set_size(size_type(0));
         BOOST_INTRUSIVE_RETHROW;
      }
      BOOST_INTRUSIVE_CATCH_END
   }

   //! <b>Effects</b>: Returns the nearest new bucket count optimized for
   //!   the container that is bigger than n. This suggestion can be used
   //!   to create bucket arrays with a size that will usually improve
   //!   container's performance. If such value does not exist, the 
   //!   higher possible value is returned.
   //! 
   //! <b>Complexity</b>: Amortized constant time.
   //! 
   //! <b>Throws</b>: Nothing.
   static size_type suggested_upper_bucket_count(size_type n)
   {
      const std::size_t *primes     = &detail::prime_list_holder<0>::prime_list[0];
      const std::size_t *primes_end = primes + detail::prime_list_holder<0>::prime_list_size;
      size_type const* bound =
            std::lower_bound(primes, primes_end, n);
      if(bound == primes_end)
            bound--;
      return size_type(*bound);
   }

   //! <b>Effects</b>: Returns the nearest new bucket count optimized for
   //!   the container that is smaller than n. This suggestion can be used
   //!   to create bucket arrays with a size that will usually improve
   //!   container's performance. If such value does not exist, the 
   //!   lower possible value is returned.
   //! 
   //! <b>Complexity</b>: Amortized constant time.
   //! 
   //! <b>Throws</b>: Nothing.
   static size_type suggested_lower_bucket_count(size_type n)
   {
      const std::size_t *primes     = &detail::prime_list_holder<0>::prime_list[0];
      const std::size_t *primes_end = primes + detail::prime_list_holder<0>::prime_list_size;
      size_type const* bound =
            std::upper_bound(primes, primes_end, n);
      if(bound != primes_end)
            bound--;
      return size_type(*bound);
   }

   /// @cond
   private:

   std::size_t priv_hash_when_rehashing(const value_type &v, detail::true_)
   {  return node_traits::get_hash(this->get_real_value_traits().to_node_ptr(v));  }

   std::size_t priv_hash_when_rehashing(const value_type &v, detail::false_)
   {  return priv_hasher()(v);   }

   void priv_store_hash(node_ptr p, std::size_t h, detail::true_)
   {  return node_traits::set_hash(p, h); }

   void priv_store_hash(node_ptr, std::size_t, detail::false_)
   {}
   
   static siterator invalid_local_it(const real_bucket_traits &b)
   {  return b.bucket_begin()->end();  }

   siterator priv_begin(size_type &bucket_num) const
   {
      size_type buckets_len = this->priv_buckets_len();
      for (bucket_num = 0; bucket_num < buckets_len; ++bucket_num){
         bucket_type &b = this->priv_buckets()[bucket_num];
         if(!b.empty())
            return b.begin();
      }
      return invalid_local_it(this->get_real_bucket_traits());
   }

   void priv_clear_buckets()
   {  priv_clear_buckets(this->priv_buckets(), this->priv_buckets_len());  }

   static void priv_clear_buckets(bucket_ptr buckets_ptr, size_type buckets_len)
   {
      for(; buckets_len--; ++buckets_ptr){
         if(safemode_or_autounlink){
            buckets_ptr->clear_and_dispose(detail::init_disposer<node_algorithms>());
         }
         else{
            buckets_ptr->clear();
         }
      }
   }

   template<class KeyType, class KeyHasher, class KeyValueEqual>
   siterator priv_find
      ( const KeyType &key,  KeyHasher hash_func
      , KeyValueEqual equal_func, size_type &bucket_number, size_type &h) const
   {
      bucket_number = from_hash_to_bucket((h = hash_func(key)));

      if(constant_time_size && this->empty()){
         return invalid_local_it(this->get_real_bucket_traits());
      }
      
      bucket_type &b = this->priv_buckets()[bucket_number];
      siterator it = b.begin();

      while(it != b.end()){
         const value_type &v = 
            *this->get_real_value_traits().to_value_ptr(it.pointed_node());
         if(equal_func(key, v)){
            return it;
         }
         ++it;
      }

      return invalid_local_it(this->get_real_bucket_traits());
   }

   template<class KeyType, class KeyHasher, class KeyValueEqual>
   std::pair<siterator, siterator> priv_equal_range
      ( const KeyType &key
      , KeyHasher hash_func
      , KeyValueEqual equal_func
      , size_type &bucket_number_first
      , size_type &bucket_number_second
      , size_type &count) const
   {
      size_type h;
      count = 0;
      //Let's see if the element is present
      std::pair<siterator, siterator> to_return
         ( priv_find(key, hash_func, equal_func, bucket_number_first, h)
         , invalid_local_it(this->get_real_bucket_traits()));
      if(to_return.first == to_return.second){
         bucket_number_second = bucket_number_first;
         return to_return;
      }
      ++count;
      //If it's present, find the first that it's not equal in
      //the same bucket
      bucket_type &b = this->priv_buckets()[bucket_number_first];
      siterator it = to_return.first;
      ++it;

      while(it != b.end()){
         const value_type &v = 
            *this->get_real_value_traits().to_value_ptr(it.pointed_node());
         if(!equal_func(key, v)){
            to_return.second = it;
            bucket_number_second = bucket_number_first;
            return to_return;
         }
         ++it;
         ++count;
      }
   
      //If we reached the end, find the first, non-empty bucket
      for(bucket_number_second = bucket_number_first+1
         ; bucket_number_second != this->priv_buckets_len()
         ; ++bucket_number_second){
         bucket_type &b = this->priv_buckets()[bucket_number_second];
         if(!b.empty()){
            to_return.second = b.begin();
            return to_return;
         }
      }

      //Otherwise, return the end node
      to_return.second = invalid_local_it(this->get_real_bucket_traits());
      return to_return;
   }
   /// @endcond
};

/// @cond
template<class T, class O1 = none, class O2 = none
                , class O3 = none, class O4 = none
                , class O5 = none, class O6 = none
                , class O7 = none
                >
struct make_hashtable_opt
{
   typedef typename pack_options
      < uset_defaults<T>, O1, O2, O3, O4, O5, O6, O7>::type packed_options;

   //Real value traits must be calculated from options
   typedef typename detail::get_value_traits
      <T, typename packed_options::value_traits>::type   value_traits;
   /// @cond
   static const bool external_value_traits =
      detail::external_value_traits_is_true<value_traits>::value;
   typedef typename detail::eval_if_c
      < external_value_traits
      , detail::eval_value_traits<value_traits>
      , detail::identity<value_traits>
      >::type                                            real_value_traits;
   typedef typename packed_options::bucket_traits        specified_bucket_traits;   
   /// @endcond
   //Real bucket traits must be calculated from options and calculated valute_traits
   typedef typename get_slist_impl
      <typename real_value_traits::node_traits>::type    slist_impl;
   typedef typename
      detail::if_c< detail::is_same
                     < specified_bucket_traits
                     , default_bucket_traits
                     >::value
                  , detail::bucket_traits_impl<slist_impl>
                  , specified_bucket_traits
                  >::type                                real_bucket_traits;

   typedef usetopt
      < value_traits
      , typename packed_options::hash
      , typename packed_options::equal
      , typename packed_options::size_type
      , packed_options::constant_time_size
      , real_bucket_traits
      , packed_options::power_2_buckets
      > type;
};
/// @endcond

//! Helper metafunction to define a \c hashtable that yields to the same type when the
//! same options (either explicitly or implicitly) are used.
#ifdef BOOST_INTRUSIVE_DOXYGEN_INVOKED
template<class T, class ...Options>
#else
template<class T, class O1 = none, class O2 = none
                , class O3 = none, class O4 = none
                , class O5 = none, class O6 = none
                , class O7 = none
                >
#endif
struct make_hashtable
{
   /// @cond
   typedef hashtable_impl
      <  typename make_hashtable_opt
            <T, O1, O2, O3, O4, O5, O6, O7>::type
      > implementation_defined;

   /// @endcond
   typedef implementation_defined type;
};

#ifndef BOOST_INTRUSIVE_DOXYGEN_INVOKED
template<class T, class O1, class O2, class O3, class O4, class O5, class O6, class O7>
class hashtable
   :  public make_hashtable<T, O1, O2, O3, O4, O5, O6, O7>::type
{
   typedef typename make_hashtable
      <T, O1, O2, O3, O4, O5, O6, O7>::type   Base;

   public:
   typedef typename Base::value_traits       value_traits;
   typedef typename Base::real_value_traits  real_value_traits;
   typedef typename Base::iterator           iterator;
   typedef typename Base::const_iterator     const_iterator;
   typedef typename Base::bucket_ptr         bucket_ptr;
   typedef typename Base::size_type          size_type;
   typedef typename Base::hasher             hasher;
   typedef typename Base::bucket_traits      bucket_traits;
   typedef typename Base::key_equal          key_equal;

   //Assert if passed value traits are compatible with the type
   BOOST_STATIC_ASSERT((detail::is_same<typename real_value_traits::value_type, T>::value));

   hashtable ( const bucket_traits &b_traits
             , const hasher & hash_func = hasher()
             , const key_equal &equal_func = key_equal()
             , const value_traits &v_traits = value_traits())
      :  Base(b_traits, hash_func, equal_func, v_traits)
   {}
};

#endif


} //namespace intrusive 
} //namespace boost 

#include <boost/intrusive/detail/config_end.hpp>

#endif //BOOST_INTRUSIVE_HASHTABLE_HPP
