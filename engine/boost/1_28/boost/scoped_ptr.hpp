#ifndef BOOST_SCOPED_PTR_HPP_INCLUDED
#define BOOST_SCOPED_PTR_HPP_INCLUDED

//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org/libs/smart_ptr/scoped_ptr.htm for documentation.
//

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>

namespace boost
{

//  scoped_ptr mimics a built-in pointer except that it guarantees deletion
//  of the object pointed to, either on destruction of the scoped_ptr or via
//  an explicit reset(). scoped_ptr is a simple solution for simple needs;
//  use shared_ptr or std::auto_ptr if your needs are more complex.

template<typename T> class scoped_ptr // noncopyable
{
private:

    T* ptr;

    scoped_ptr(scoped_ptr const &);
    scoped_ptr & operator=(scoped_ptr const &);

public:

    typedef T element_type;

    explicit scoped_ptr(T * p = 0): ptr(p) // never throws
    {
    }

    ~scoped_ptr() // never throws
    {
        checked_delete(ptr);
    }

    void reset(T * p = 0) // never throws
    {
        if (ptr != p)
        {
            checked_delete(ptr);
            ptr = p;
        }
    }

    T & operator*() const // never throws
    {
        BOOST_ASSERT(ptr != 0);
        return *ptr;
    }

    T * operator->() const // never throws
    {
        BOOST_ASSERT(ptr != 0);
        return ptr;
    }

    T * get() const // never throws
    {
        return ptr;
    }

    void swap(scoped_ptr & b) // never throws
    {
        T * tmp = b.ptr;
        b.ptr = ptr;
        ptr = tmp;
    }
};

template<typename T> inline void swap(scoped_ptr<T> & a, scoped_ptr<T> & b) // never throws
{
    a.swap(b);
}

} // namespace boost

#endif // #ifndef BOOST_SCOPED_PTR_HPP_INCLUDED
