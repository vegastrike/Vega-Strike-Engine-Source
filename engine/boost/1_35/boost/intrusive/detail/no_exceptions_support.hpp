/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2007
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_NO_EXCEPTION_SUPPORT_HPP

#if !(defined BOOST_INTRUSIVE_DISABLE_EXCEPTION_HANDLING)
#    include <boost/detail/no_exceptions_support.hpp>
#    define BOOST_INTRUSIVE_TRY        BOOST_TRY
#    define BOOST_INTRUSIVE_CATCH(x)   BOOST_CATCH(x)
#    define BOOST_INTRUSIVE_RETHROW    BOOST_RETHROW
#    define BOOST_INTRUSIVE_CATCH_END  BOOST_CATCH_END
#else
#    define BOOST_INTRUSIVE_TRY        { if (true)
#    define BOOST_INTRUSIVE_CATCH(x)   else if (false)
#    define BOOST_INTRUSIVE_RETHROW
#    define BOOST_INTRUSIVE_CATCH_END  }
#endif

#endif   //#ifndef BOOST_INTRUSIVE_NO_EXCEPTION_SUPPORT_HPP
