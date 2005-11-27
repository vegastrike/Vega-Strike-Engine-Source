//  boost utility.hpp header file  -------------------------------------------//

//  (C) Copyright boost.org 1999. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright
//  notice appears in all copies. This software is provided "as is" without
//  express or implied warranty, and with no claim as to its suitability for
//  any purpose.

//  See http://www.boost.org for most recent version including documentation.

//  Classes appear in alphabetical order

#ifndef BOOST_UTILITY_HPP
#define BOOST_UTILITY_HPP

// certain headers are part of the <utility.hpp> interface

#include <boost/checked_delete.hpp>
#include <boost/utility/base_from_member.hpp>  
#include <boost/utility/addressof.hpp>

namespace boost
{
//  next() and prior() template functions  -----------------------------------//

    //  Helper functions for classes like bidirectional iterators not supporting
    //  operator+ and operator-.
    //
    //  Usage:
    //    const std::list<T>::iterator p = get_some_iterator();
    //    const std::list<T>::iterator prev = boost::prior(p);

    //  Contributed by Dave Abrahams

    template <class T>
    inline T next(T x) { return ++x; }

    template <class T>
    inline T prior(T x) { return --x; }


//  class noncopyable  -------------------------------------------------------//

    //  Private copy constructor and copy assignment ensure classes derived from
    //  class noncopyable cannot be copied.

    //  Contributed by Dave Abrahams

    class noncopyable
    {
    protected:
        noncopyable(){}
        ~noncopyable(){}
    private:  // emphasize the following members are private
        noncopyable( const noncopyable& );
        const noncopyable& operator=( const noncopyable& );
    }; // noncopyable


} // namespace boost

#endif  // BOOST_UTILITY_HPP

