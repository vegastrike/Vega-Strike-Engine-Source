#ifndef BOOST_DETAIL_LWM_WIN32_HPP_INCLUDED
#define BOOST_DETAIL_LWM_WIN32_HPP_INCLUDED

#if _MSC_VER >= 1020
#pragma once
#endif

//
//  boost/detail/lwm_win32.hpp
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#include <boost/detail/winapi.hpp>

namespace boost
{

namespace detail
{

class lightweight_mutex
{
private:

    long l_;

    lightweight_mutex(lightweight_mutex const &);
    lightweight_mutex & operator=(lightweight_mutex const &);

public:

    lightweight_mutex(): l_(0)
    {
    }

    class scoped_lock;
    friend class scoped_lock;

    class scoped_lock
    {
    private:

        lightweight_mutex & m_;

        scoped_lock(scoped_lock const &);
        scoped_lock & operator=(scoped_lock const &);

    public:

        explicit scoped_lock(lightweight_mutex & m): m_(m)
        {
            while( winapi::InterlockedExchange(&m_.l_, 1) )
            {
                winapi::Sleep(0);
            }
        }

        ~scoped_lock()
        {
            winapi::InterlockedExchange(&m_.l_, 0);

            // Note: adding a Sleep(0) here will make
            // the mutex more fair and will increase the overall
            // performance of some applications substantially in
            // high contention situations, but will penalize the
            // low contention / single thread case up to 5x
        }
    };
};

} // namespace detail

} // namespace boost

#endif // #ifndef BOOST_DETAIL_LWM_WIN32_HPP_INCLUDED
