/**
* endianness.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _ENDIANNESS_H
#define _ENDIANNESS_H
double DONTUSE__NXSwapBigDoubleToLittleEndian( double x );


#if defined (__HAIKU__) //For unknow reasons, Haiku don't fit into any case below
    #include <endian.h>
#elif defined (__APPLE__) || defined (MACOSX) || defined (BSD) || defined (__FreeBSD__)
    #include <machine/endian.h>
#else

    #if defined (IRIX)
    # include <sys/endian.h>

    #elif !defined (_WIN32) && !defined (__CYGWIN__) && !defined (SOLARIS)
    # include <endian.h>
    #else
    # define __BIG_ENDIAN 1
    # define __LITTLE_ENDIAN 0
    # define __BYTE_ORDER 0
    #endif
#endif
#if defined (__APPLE__) || defined (MACOSX)
    #include <machine/endian.h>
    #if __BYTE_ORDER == __BIG_ENDIAN
    # include <machine/byte_order.h>
    # define le32_to_cpu( x ) ( NXSwapHostLongToLittle( x ) )
    # define le16_to_cpu( x ) ( NXSwapHostShortToLittle( x ) )
    # define le64_to_cpu( x ) ( DONTUSE__NXSwapBigDoubleToLittleEndian( x ) )
    #else
    # define le32_to_cpu( x ) (x)
    # define le16_to_cpu( x ) (x)
    # define le64_to_cpu( x ) (x)
    #endif
#else
    #if defined (IRIX) || (defined (__SVR4) && defined (__sun ) )
    #include <sys/types.h>
    # if BYTE_ORDER == BIG_ENDIAN       /* depends on MIPSEB or MIPSEL and SGIAPI */
    #  define le32_to_cpu( x )             \
    ( ( (x)<<24 )                          \
     |( ( (x)<<8 )&0x00FF0000 )            \
     |( ( (uint32_t) (x)>>8 )&0x0000FF00 ) \
     |( (uint32_t) (x)>>24 ) )
    #  define le16_to_cpu( x ) ( ( (x&0xFF)<<8 )|( (unsigned short) (x)>>8 ) )
    # define le64_to_cpu( x ) ( DONTUSE__NXSwapBigDoubleToLittleEndian( x ) )
    # else
    #  define le32_to_cpu( x ) (x)
    #  define le16_to_cpu( x ) (x)
    #  define le64_to_cpu( x ) (x)
    # endif

    #elif __BYTE_ORDER == __BIG_ENDIAN && !defined (BSD) && !defined (__FreeBSD__)
    # include <byteswap.h>
    # define le32_to_cpu( x ) ( bswap_32( x ) )
    # define le16_to_cpu( x ) ( bswap_16( x ) )
    # define le64_to_cpu( x ) ( DONTUSE__NXSwapBigDoubleToLittleEndian( x ) )
    #else
    # define le32_to_cpu( x ) (x)
    # define le16_to_cpu( x ) (x)
    #  define le64_to_cpu( x ) (x)
    #endif
#endif

inline float VSSwapHostFloatToLittle( float x )
{
    union LILfloat
    {
        float f;
        unsigned int i;
    }
    l;
    l.f = x;
    l.i = le32_to_cpu( l.i );
    return l.f;
}

inline double VSSwapHostDoubleToLittle( double x )
{
    return le64_to_cpu( x );
}

inline unsigned int VSSwapHostIntToLittle( unsigned int x )
{
    return le32_to_cpu( x );
}

inline unsigned short VSSwapHostShortToLittle( unsigned short x )
{
    return le16_to_cpu( x );
}

inline double DONTUSE__NXSwapBigDoubleToLittleEndian( double x )
{
    union LILdubl
    {
        double d;
        unsigned int i[2];
    }
    l;
    l.d    = x;
    unsigned int tmp = le32_to_cpu( l.i[0] );
    l.i[0] = le32_to_cpu( l.i[1] );
    l.i[1] = tmp;
    return l.d;
}

#endif  //_ENDIANNESS_H

