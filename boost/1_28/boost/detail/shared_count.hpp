#ifndef BOOST_DETAIL_SHARED_COUNT_HPP_INCLUDED
#define BOOST_DETAIL_SHARED_COUNT_HPP_INCLUDED

#if _MSC_VER >= 1020
#pragma once
#endif

//
//  detail/shared_count.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#include <boost/config.hpp>

#ifndef BOOST_NO_AUTO_PTR
# include <memory>
#endif

#include <boost/checked_delete.hpp>
#include <boost/detail/lightweight_mutex.hpp>

#include <functional>       // for std::less
#include <exception>        // for std::exception

namespace boost
{

class use_count_is_zero: public std::exception
{
public:

    virtual char const * what() const throw()
    {
        return "use_count_is_zero";
    }
};

class counted_base
{
private:

    typedef detail::lightweight_mutex mutex_type;

public:

    counted_base():
        use_count_(0), weak_count_(0), self_deleter_(&self_delete)
    {
    }

    // pre: initial_use_count <= initial_weak_count

    explicit counted_base(long initial_use_count, long initial_weak_count):
        use_count_(initial_use_count), weak_count_(initial_weak_count), self_deleter_(&self_delete)
    {
    }

    virtual ~counted_base() // nothrow
    {
    }

    // dispose() is called when use_count_ drops to zero, to release
    // the resources managed by *this.
    //
    // counted_base doesn't manage any resources except itself, and
    // the default implementation is a no-op.
    //
    // dispose() is not pure virtual since weak_ptr instantiates a
    // counted_base in its default constructor.

    virtual void dispose() // nothrow
    {
    }

    void add_ref()
    {
#ifdef BOOST_HAS_THREADS
        mutex_type::scoped_lock lock(mtx_);
#endif
        if(use_count_ == 0 && weak_count_ != 0) throw use_count_is_zero();
        ++use_count_;
        ++weak_count_;
    }

    void release() // nothrow
    {
        long new_use_count;
        long new_weak_count;

        {
#ifdef BOOST_HAS_THREADS
            mutex_type::scoped_lock lock(mtx_);
#endif
            new_use_count = --use_count_;
            new_weak_count = --weak_count_;
        }

        if(new_use_count == 0)
        {
            dispose();
        }

        if(new_weak_count == 0)
        {
            // not a direct 'delete this', because the inlined
            // release() may use a different heap manager
            self_deleter_(this);
        }
    }

    void weak_add_ref() // nothrow
    {
#ifdef BOOST_HAS_THREADS
        mutex_type::scoped_lock lock(mtx_);
#endif
        ++weak_count_;
    }

    void weak_release() // nothrow
    {
        long new_weak_count;

        {
#ifdef BOOST_HAS_THREADS
            mutex_type::scoped_lock lock(mtx_);
#endif
            new_weak_count = --weak_count_;
        }

        if(new_weak_count == 0)
        {
            self_deleter_(this);
        }
    }

    long use_count() const // nothrow
    {
#ifdef BOOST_HAS_THREADS
        mutex_type::scoped_lock lock(mtx_);
#endif
        return use_count_;
    }

private:

    counted_base(counted_base const &);
    counted_base & operator= (counted_base const &);

    static void self_delete(counted_base * p)
    {
        delete p;
    }

    // inv: use_count_ <= weak_count_

    long use_count_;
    long weak_count_;
#ifdef BOOST_HAS_THREADS
    mutable mutex_type mtx_;
#endif
    void (*self_deleter_) (counted_base *);
};

inline void intrusive_ptr_add_ref(counted_base * p)
{
    p->add_ref();
}

inline void intrusive_ptr_release(counted_base * p)
{
    p->release();
}

namespace detail
{

template<class P, class D> class counted_base_impl: public counted_base
{
private:

    P ptr; // copy constructor must not throw
    D del; // copy constructor must not throw

    counted_base_impl(counted_base_impl const &);
    counted_base_impl & operator= (counted_base_impl const &);

public:

    // pre: initial_use_count <= initial_weak_count, d(p) must not throw

    counted_base_impl(P p, D d, long initial_use_count, long initial_weak_count):
        counted_base(initial_use_count, initial_weak_count), ptr(p), del(d)
    {
    }

    virtual void dispose() // nothrow
    {
        del(ptr);
    }
};

class weak_count;

class shared_count
{
private:

    counted_base * pi_;

    friend class weak_count;

    template<class P, class D> shared_count(P, D, counted_base const *);

public:

    shared_count(): pi_(new counted_base(1, 1))
    {
    }

    explicit shared_count(counted_base * pi): pi_(pi) // never throws
    {
        pi_->add_ref();
    }

    template<class P, class D> shared_count(P p, D d, void const * = 0): pi_(0)
    {
        try
        {
            pi_ = new counted_base_impl<P, D>(p, d, 1, 1);
        }
        catch(...)
        {
            d(p); // delete p
            throw;
        }
    }

    template<class P, class D> shared_count(P, D, counted_base * pi): pi_(pi)
    {
        pi_->add_ref();
    }

#ifndef BOOST_NO_AUTO_PTR

    // auto_ptr<Y> is special cased to provide the strong guarantee

    template<typename Y>
    explicit shared_count(std::auto_ptr<Y> & r): pi_(new counted_base_impl< Y *, checked_deleter<Y> >(r.get(), checked_deleter<Y>(), 1, 1))
    {
        r.release();
    }

#endif 

    ~shared_count() // nothrow
    {
        pi_->release();
    }

    shared_count(shared_count const & r): pi_(r.pi_) // nothrow
    {
        pi_->add_ref();
    }

    explicit shared_count(weak_count const & r); // throws use_count_is_zero when r.use_count() == 0

    shared_count & operator= (shared_count const & r) // nothrow
    {
        counted_base * tmp = r.pi_;
        tmp->add_ref();
        pi_->release();
        pi_ = tmp;

        return *this;
    }

    void swap(shared_count & r) // nothrow
    {
        counted_base * tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_->use_count();
    }

    bool unique() const // nothrow
    {
        return pi_->use_count() == 1;
    }

    friend inline bool operator==(shared_count const & a, shared_count const & b)
    {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(shared_count const & a, shared_count const & b)
    {
        return std::less<counted_base *>()(a.pi_, b.pi_);
    }
};

class weak_count
{
private:

    counted_base * pi_;

    friend class shared_count;

public:

    weak_count(): pi_(new counted_base(0, 1)) // can throw
    {
    }

    weak_count(shared_count const & r): pi_(r.pi_) // nothrow
    {
        pi_->weak_add_ref();
    }

    weak_count(weak_count const & r): pi_(r.pi_) // nothrow
    {
        pi_->weak_add_ref();
    }

    ~weak_count() // nothrow
    {
        pi_->weak_release();
    }

    weak_count & operator= (shared_count const & r) // nothrow
    {
        counted_base * tmp = r.pi_;
        tmp->weak_add_ref();
        pi_->weak_release();
        pi_ = tmp;

        return *this;
    }

    weak_count & operator= (weak_count const & r) // nothrow
    {
        counted_base * tmp = r.pi_;
        tmp->weak_add_ref();
        pi_->weak_release();
        pi_ = tmp;

        return *this;
    }

    void swap(weak_count & r) // nothrow
    {
        counted_base * tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_->use_count();
    }

    friend inline bool operator==(weak_count const & a, weak_count const & b)
    {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(weak_count const & a, weak_count const & b)
    {
        return std::less<counted_base *>()(a.pi_, b.pi_);
    }
};

inline shared_count::shared_count(weak_count const & r): pi_(r.pi_)
{
    pi_->add_ref();
}

} // namespace detail

} // namespace boost

#endif  // #ifndef BOOST_DETAIL_SHARED_COUNT_HPP_INCLUDED
