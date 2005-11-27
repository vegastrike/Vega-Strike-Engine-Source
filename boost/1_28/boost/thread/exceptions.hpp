// Copyright (C) 2001
// William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.

#ifndef BOOST_THREAD_EXCEPTIONS_PDM070801_H
#define BOOST_THREAD_EXCEPTIONS_PDM070801_H

//  pdm: Sorry, but this class is used all over the place & I end up
//       with recursive headers if I don't separate it
//  wek: Not sure why recursive headers would cause compilation problems
//       given the include guards, but regardless it makes sense to
//       seperate this out any way.

#include <stdexcept>

namespace boost {

class lock_error : public std::logic_error
{
public:
    lock_error();
};

class thread_resource_error : public std::runtime_error
{
public:
    thread_resource_error();
};

} // namespace boost

#endif // BOOST_THREAD_CONFIG_PDM070801_H
