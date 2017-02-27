// (C) Copyright 2005 Matthias Troyer

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Matthias Troyer

#ifndef BOOST_MPI_DETAIL_TYPE_MPI_DATATYPE_CACHE_HPP
#define BOOST_MPI_DETAIL_TYPE_MPI_DATATYPE_CACHE_HPP

#include <boost/mpi/datatype_fwd.hpp>
#include <boost/mpi/detail/mpi_datatype_oarchive.hpp>
#include <boost/mpi/exception.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <typeinfo>

// The std::type_info::before function in Visual C++ 8.0 (and probably earlier)
// incorrectly returns an "int" instead of a "bool". Then the compiler has the
// audacity to complain when that "int" is converted to a "bool". Silence
// this warning.
#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4800)
#endif

namespace boost { namespace mpi { namespace detail {

/// @brief comparison function object for two std::type_info pointers
///
/// is implemented using the before() member function of the std::type_info
/// class

struct type_info_compare
{
  bool operator()(std::type_info const* lhs, std::type_info const* rhs) const
  {
    return lhs->before(*rhs);
  }
};


/// @brief a map of MPI data types, indexed by their type_info
///
///
class BOOST_MPI_DECL mpi_datatype_map
 : private std::map<std::type_info const*,MPI_Datatype,type_info_compare>,
   public boost::noncopyable
{
public:
  mpi_datatype_map()
  {}

  ~mpi_datatype_map()
  {
    // do not free after call to MPI_FInalize
    int finalized=0;
    BOOST_MPI_CHECK_RESULT(MPI_Finalized,(&finalized));
    if (!finalized)
      free();
  }

  template <class T>
  MPI_Datatype datatype(const T& x = T(), typename boost::enable_if<is_mpi_builtin_datatype<T> >::type* =0)
  {
    return get_mpi_datatype<T>(x);
  }

  template <class T>
  MPI_Datatype datatype(const T& x =T(), typename boost::disable_if<is_mpi_builtin_datatype<T> >::type* =0 )
  {
    BOOST_MPL_ASSERT((is_mpi_datatype<T>));

    // check whether the type already exists
    std::type_info const* t = &typeid(T);
    const_iterator it = find(t);
    if(it ==end())
    {
      // need to create a type
      mpi_datatype_oarchive ar(x);
      insert(std::make_pair(t,ar.get_mpi_datatype()));
      it = find(t);
    }

  return it->second;
  }

private:
  // free all MPI data types
  void free()
  {
    // ignore errors in the destructor
    for (iterator it=begin(); it !=end(); ++it)
          MPI_Type_free(&(it->second));
  }

};

extern mpi_datatype_map mpi_datatype_cache;

} } } // end namespace boost::mpi::detail

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

#endif // BOOST_MPI_DETAIL_TYPE_MPI_DATATYPE_CACHE_HPP
