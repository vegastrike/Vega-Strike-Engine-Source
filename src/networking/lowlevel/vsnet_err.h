/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef VSNET_ERR_H
#define VSNET_ERR_H

#include <assert.h>
#include <sstream>
#include <string>

inline std::string vsnetLastError( )
{
    std::ostringstream ostr;
#if defined(_WIN32) && !defined(__CYGWIN__)
    ostr << WSAGetLastError() << std::ends;
#else
    ostr << strerror(errno) << std::ends;
#endif
    return ostr.str();
}

inline bool vsnetEWouldBlock( )
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    return ( WSAGetLastError() == WSAEWOULDBLOCK );
#else
    return ( errno == EWOULDBLOCK );
#endif
}

inline bool vsnetEConnAborted()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    return ( WSAGetLastError() == WSAECONNABORTED );
#else
    return ( errno == ECONNABORTED );
#endif
}

inline bool vsnetEConnReset()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    return ( WSAGetLastError() == WSAECONNRESET );
#else
    return ( errno == ECONNRESET );
#endif
}

inline int vsnetGetLastError()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
    return WSAGetLastError();
#else
    return errno;
#endif
}

#endif /* VSNET_ERR_H */

