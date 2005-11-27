#ifndef BOOST_INTRUSIVE_PTR_HPP_INCLUDED
#define BOOST_INTRUSIVE_PTR_HPP_INCLUDED

//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
//

#ifdef BOOST_MSVC  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator->
#endif

#include <functional> // std::less

namespace boost
{

//
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//
//  Relies on unqualified calls to
//  
//      void intrusive_ptr_add_ref(T * p);
//      void intrusive_ptr_release(T * p);
//
//          (p != 0)
//
//  The object is responsible for destroying itself.
//

template<class T> class intrusive_ptr
{
private:

    typedef intrusive_ptr this_type;

public:

    intrusive_ptr(): p_(0)
    {
    }

    intrusive_ptr(T * p): p_(p)
    {
        if(p_ != 0) intrusive_ptr_add_ref(p_);
    }

    ~intrusive_ptr()
    {
        if(p_ != 0) intrusive_ptr_release(p_);
    }

#ifdef BOOST_MSVC6_MEMBER_TEMPLATES

    template<class U> intrusive_ptr(intrusive_ptr<U> const & rhs): p_(rhs.get())
    {
        if(p_ != 0) intrusive_ptr_add_ref(p_);
    }

#endif

    intrusive_ptr(intrusive_ptr const & rhs): p_(rhs.p_)
    {
        if(p_ != 0) intrusive_ptr_add_ref(p_);
    }

#ifdef BOOST_MSVC6_MEMBER_TEMPLATES

    template<class U> intrusive_ptr & operator=(intrusive_ptr<U> const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

#endif

    intrusive_ptr & operator=(intrusive_ptr const & rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr & operator=(T * rhs)
    {
        this_type(rhs).swap(*this);
        return *this;
    }

    void swap(intrusive_ptr & rhs)
    {
        T * tmp = p_;
        p_ = rhs.p_;
        rhs.p_ = tmp;
    }

    T * get() const
    {
        return p_;
    }

    T & operator*() const
    {
        return *p_;
    }

    T * operator->() const
    {
        return p_;
    }

    bool empty() const
    {
        return p_ == 0;
    }

    typedef bool (intrusive_ptr::*bool_type) () const;

    operator bool_type () const
    {
        return p_ == 0? 0: &intrusive_ptr::empty;
    }

private:

    T * p_;
};

template<class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
    lhs.swap(rhs);
}

template<class T, class U> intrusive_ptr<T> shared_dynamic_cast(intrusive_ptr<U> const & p)
{
    return dynamic_cast<T *>(p.get());
}

template<class T, class U> intrusive_ptr<T> shared_static_cast(intrusive_ptr<U> const & p)
{
    return static_cast<T *>(p.get());
}

template<class T, class U> inline bool operator==(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(intrusive_ptr<T> const & a, intrusive_ptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T> inline bool operator<(intrusive_ptr<T> const & a, intrusive_ptr<T> const & b)
{
    return std::less<T *>(a.get(), b.get());
}

template<class T> inline bool operator==(intrusive_ptr<T> const & a, T * b)
{
    return a.get() == b;
}

template<class T> inline bool operator!=(intrusive_ptr<T> const & a, T * b)
{
    return a.get() != b;
}

template<class T> inline bool operator==(T * a, intrusive_ptr<T> const & b)
{
    return a == b.get();
}

template<class T> inline bool operator!=(T * a, intrusive_ptr<T> const & b)
{
    return a != b.get();
}

// mem_fn support

template<class T> T * get_pointer(intrusive_ptr<T> const & p)
{
    return p.get();
}

} // namespace boost

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif    

#endif  // #ifndef BOOST_INTRUSIVE_PTR_HPP_INCLUDED
