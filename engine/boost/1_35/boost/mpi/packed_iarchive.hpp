// (C) Copyright 2005 Matthias Troyer
// (C) Copyright 2006 Douglas Gregor <doug.gregor -at- gmail.com>

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Matthias Troyer
//           Douglas Gregor

/** @file packed_iarchive.hpp
 *
 *  This header provides the facilities for packing Serializable data
 *  types into a buffer using @c MPI_Pack. The buffers can then be
 *  transmitted via MPI and then be unpacked either via the facilities
 *  in @c packed_oarchive.hpp or @c MPI_Unpack.
 */
#ifndef BOOST_MPI_PACKED_IARCHIVE_HPP
#define BOOST_MPI_PACKED_IARCHIVE_HPP

#include <boost/mpi/datatype.hpp>
#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/basic_binary_iarchive.hpp>
#include <boost/archive/shared_ptr_helper.hpp>
#include <boost/mpi/detail/packed_iprimitive.hpp>
#include <boost/assert.hpp>

namespace boost { namespace mpi {

/** @brief An archive that packs binary data into an MPI buffer.
 *
 *  The @c packed_iarchive class is an Archiver (as in the
 *  Boost.Serialization library) that packs binary data into a buffer
 *  for transmission via MPI. It can operate on any Serializable data
 *  type and will use the @c MPI_Pack function of the underlying MPI
 *  implementation to perform serialization.
 */
class BOOST_MPI_DECL packed_iarchive
  : public packed_iprimitive
  , public archive::basic_binary_iarchive<packed_iarchive>
  , public archive::detail::shared_ptr_helper
{
public:
  /**
   *  Construct a @c packed_iarchive for transmission over the given
   *  MPI communicator and with an initial buffer.
   *
   *  @param comm The communicator over which this archive will be
   *  sent.
   *
   *  @param b A user-defined buffer that will be filled with the
   *  binary representation of serialized objects.
   *
   *  @param flags Control the serialization of the data types. Refer
   *  to the Boost.Serialization documentation before changing the
   *  default flags.
   *
   *  @param position Set the offset into buffer @p b at which
   *  deserialization will begin.
   */
  packed_iarchive(MPI_Comm const & comm, buffer_type & b, unsigned int flags = boost::archive::no_header, int position = 0)
        : packed_iprimitive(b,comm,position),
          archive::basic_binary_iarchive<packed_iarchive>(flags)
        {}

  /**
   *  Construct a @c packed_iarchive for transmission over the given
   *  MPI communicator.
   *
   *  @param comm The communicator over which this archive will be
   *  sent.
   *
   *  @param flags Control the serialization of the data types. Refer
   *  to the Boost.Serialization documentation before changing the
   *  default flags.
   */
  packed_iarchive
          ( MPI_Comm const & comm , unsigned int flags = boost::archive::no_header)
         : packed_iprimitive(internal_buffer_,comm),
           archive::basic_binary_iarchive<packed_iarchive>(flags)
        {}

private:
  /// An internal buffer to be used when the user does not supply his
  /// own buffer.
  buffer_type internal_buffer_;
};

} } // end namespace boost::mpi

BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(boost::mpi::packed_iarchive)
BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::mpi::packed_iarchive)

#endif // BOOST_MPI_PACKED_IARCHIVE_HPP
