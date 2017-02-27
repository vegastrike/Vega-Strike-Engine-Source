#ifndef BOOST_SERIALIZATION_EXPORT_HPP
#define BOOST_SERIALIZATION_EXPORT_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// export.hpp: set traits of classes to be serialized

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// (C) Copyright 2006 David Abrahams - http://www.boost.org.
// implementation of class export functionality.  This is an alternative to
// "forward declaration" method to provoke instantiation of derived classes
// that are to be serialized through pointers.

#include <utility>

#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <boost/archive/detail/dynamically_initialized.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/is_abstract.hpp>
#include <boost/serialization/force_include.hpp>

#include <boost/archive/detail/register_archive.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>

#include <iostream>

namespace boost {
namespace archive {
namespace detail {

class basic_pointer_iserializer;
class basic_pointer_oserializer;

template<class Archive, class T>
class pointer_iserializer;
template<class Archive, class T>
class pointer_oserializer;

template <class Archive, class Serializable>
struct export_impl
{
    static const basic_pointer_iserializer &
    enable_load(mpl::true_){
        return pointer_iserializer<Archive, Serializable>::get_instance();
    }

    static const basic_pointer_oserializer &
    enable_save(mpl::true_){
        return pointer_oserializer<Archive, Serializable>::get_instance();
    }

    inline static void enable_load(mpl::false_) {}
    inline static void enable_save(mpl::false_) {}
};

template<class T>
struct guid_initializer
{
    typedef typename
    boost::serialization::type_info_implementation<T>::type eti_type;
    
    static void export_register(const char *key)
    {
        eti_type::export_register(key);
    }
    
    static const guid_initializer& get_instance(char const* key)
    {
        static guid_initializer const instance(key);
        return instance;
    }
    
    BOOST_DLLEXPORT guid_initializer(const char *key = 0) BOOST_USED ;
};


template<class T>
BOOST_DLLEXPORT guid_initializer<T>::guid_initializer(const char *key)
{
    if(0 != key)
        export_register(key);

    // generates the statically-initialized objects whose constructors
    // register the information allowing serialization of T objects
    // through pointers to their base classes.
    instantiate_ptr_serialization((T*)0, 0);
}

// On many platforms, naming a specialization of this template is
// enough to cause its argument to be instantiated.
template <void(*)()>
struct instantiate_function {};

template <class Archive, class Serializable>
struct ptr_serialization_support
{
# ifdef BOOST_MSVC
    virtual BOOST_DLLEXPORT void instantiate() BOOST_USED;
    
# elif defined(__BORLANDC__)
    
    static void instantiate();
    enum { x = sizeof(instantiate(),3) };
    
# else
    
    static void instantiate();
    typedef instantiate_function<
        &ptr_serialization_support::instantiate
    > x;

# endif
};

template <class Archive, class Serializable>
BOOST_DLLEXPORT void ptr_serialization_support<Archive,Serializable>::instantiate()
{
    typedef mpl::not_<serialization::is_abstract<Serializable> > concrete;
    
    export_impl<Archive,Serializable>::enable_save(
        mpl::and_<concrete, BOOST_DEDUCED_TYPENAME Archive::is_saving>());

    export_impl<Archive,Serializable>::enable_load(
        mpl::and_<concrete, BOOST_DEDUCED_TYPENAME Archive::is_loading>());
}

} // namespace detail
} // namespace archive
} // namespace boost

#define BOOST_CLASS_EXPORT_GUID(T, K)                                               \
namespace                                                                           \
{                                                                                   \
    ::boost::archive::detail::guid_initializer< T > const&                          \
        BOOST_PP_CAT(boost_serialization_guid_initializer_, __LINE__)               \
          = ::boost::archive::detail::guid_initializer< T >::get_instance(K);       \
}

// the following is solely to support de-serialization of pointers serialized
// under 1.32
#define BOOST_CLASS_EXPORT_GUID_1(T, K)                                             \
namespace                                                                           \
{                                                                                   \
    ::boost::archive::detail::guid_initializer< T > const&                          \
    BOOST_PP_CAT(boost_serialization_guid_initializer_, __LINE__ ## _1)             \
          = ::boost::archive::detail::guid_initializer< T >::get_instance(K);       \
}

#if BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3205))

// CodeWarrior fails to construct static members of class templates
// when they are instantiated from within templates, so on that
// compiler we ask users to specifically register base/derived class
// relationships for exported classes.  On all other compilers, use of
// this macro is entirely optional.
# define BOOST_SERIALIZATION_MWERKS_BASE_AND_DERIVED(Base,Derived)                  \
namespace                                                                           \
{                                                                                   \
  int BOOST_PP_CAT(boost_serialization_mwerks_init_, __LINE__) =                    \
      (::boost::archive::detail::instantiate_ptr_serialization((Derived*)0,0), 3);  \
  int BOOST_PP_CAT(boost_serialization_mwerks_init2_, __LINE__) = (                 \
      ::boost::serialization::void_cast_register((Derived*)0,(Base*)0)              \
    , 3);                                                                           \
}

#else

# define BOOST_SERIALIZATION_MWERKS_BASE_AND_DERIVED(Base,Derived)

#endif 

// check for unnecessary export.  T isn't polymorphic so there is no
// need to export it.
#define BOOST_CLASS_EXPORT_CHECK(T)                              \
    BOOST_STATIC_WARNING(                                        \
        boost::serialization::type_info_implementation< T >      \
            ::type::is_polymorphic::value                        \
    );                                                           \
    /**/

// the default exportable class identifier is the class name
// the default list of archives types for which code id generated
// are the originally included with this serialization system
#define BOOST_CLASS_EXPORT(T)                   \
    BOOST_CLASS_EXPORT_GUID(                    \
        T,                                      \
        BOOST_PP_STRINGIZE(T)                   \
    )                                           \
    /**/

#endif // BOOST_SERIALIZATION_EXPORT_HPP

