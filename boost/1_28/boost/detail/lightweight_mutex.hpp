#ifndef BOOST_DETAIL_LIGHTWEIGHT_MUTEX_HPP_INCLUDED
#define BOOST_DETAIL_LIGHTWEIGHT_MUTEX_HPP_INCLUDED

#if _MSC_VER >= 1020
#pragma once
#endif

//
//  boost/detail/lightweight_mutex.hpp - lightweight mutex
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  typedef <implementation-defined> boost::detail::lightweight_mutex;
//
//  boost::detail::lightweight_mutex meets the Mutex concept requirements
//  See http://www.boost.org/libs/thread/doc/mutex_concept.html#Mutex
//
//  * Used by the smart pointer library
//  * Performance oriented
//  * Header-only implementation
//  * Small memory footprint
//  * Not a general purpose mutex, use boost::mutex, CRITICAL_SECTION or
//    pthread_mutex instead.
//  * Never spin in a tight lock/do-something/unlock loop, since
//    lightweight_mutex does not guarantee fairness.
//  * Never keep a lightweight_mutex locked for long periods.
//

//  Note: lwm_linux.hpp has been disabled by default; see the comments
//        inside for more info.


#include <boost/config.hpp>

//
//  Note to implementors: if you write a platform-specific lightweight_mutex
//  for a platform that supports pthreads, be sure to test its performance
//  against the pthreads-based version using smart_ptr_timing_test.cpp and
//  smart_ptr_mt_test.cpp. Custom versions are usually not worth the trouble
//  _unless_ the performance gains are substantial.
//

#ifndef BOOST_HAS_THREADS
#  include <boost/detail/lwm_nop.hpp>
#elif defined(BOOST_USE_ASM_ATOMIC_H)
#  include <boost/detail/lwm_linux.hpp>
#elif defined(BOOST_LWM_USE_CRITICAL_SECTION)
#  include <boost/detail/lwm_win32_cs.hpp>
#elif defined(BOOST_LWM_USE_PTHREADS)
#  include <boost/detail/lwm_pthreads.hpp>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  include <boost/detail/lwm_win32.hpp>
#elif defined(__sgi)
#  include <boost/detail/lwm_irix.hpp>
#elif defined(BOOST_HAS_PTHREADS)
#  define BOOST_LWM_USE_PTHREADS
#  include <boost/detail/lwm_pthreads.hpp>
#else
#  include <boost/detail/lwm_nop.hpp>
#endif

#endif // #ifndef BOOST_DETAIL_LIGHTWEIGHT_MUTEX_HPP_INCLUDED
