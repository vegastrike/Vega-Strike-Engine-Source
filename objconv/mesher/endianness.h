#ifndef _ENDIANNESS_H
#define _ENDIANNESS_H
double DONTUSE__NXSwapBigDoubleToLittleEndian(double x);


double VSSwapHostDoubleToLittle (double x);
float VSSwapHostFloatToLittle (float x);
unsigned int VSSwapHostIntToLittle (unsigned int x);
unsigned short VSSwapHostShortToLittle (unsigned short x);
#if defined(__APPLE__) || defined(MACOSX) || defined(BSD) || defined(__FreeBSD__)
    #include<machine/endian.h>
#else

    #if defined(IRIX)
    # include <sys/endian.h>
    
    #elif !defined(_WIN32) && !defined(__CYGWIN__)
    # include <endian.h>
    #else
    # define __BIG_ENDIAN 1
    # define __LITTLE_ENDIAN 0
    # define __BYTE_ORDER 0
    #endif
#endif
#if defined(__APPLE__) || defined(MACOSX)
    #include<machine/endian.h>
    #if __BYTE_ORDER == __BIG_ENDIAN
    # include <machine/byte_order.h>
    # define le32_to_cpu(x) (NXSwapHostLongToLittle(x))
    # define le16_to_cpu(x) (NXSwapHostShortToLittle(x))
    # define le64_to_cpu(x) (DONTUSE__NXSwapBigDoubleToLittleEndian(x))
    #else
    # define le32_to_cpu(x) (x)
    # define le16_to_cpu(x) (x)
    # define le64_to_cpu(x) (x)
    #endif
#else
    #if defined(IRIX)
    # if BYTE_ORDER == BIG_ENDIAN	/* depends on MIPSEB or MIPSEL and SGIAPI */
    #  define le32_to_cpu(x) (((x) << 24) | \
                            (((x) << 8) & 0x00FF0000) | \
                            (((uint32_t)(x) >> 8) & 0x0000FF00) | \
                            ((uint32_t)(x) >> 24))
    #  define le16_to_cpu(x) (((x&0xFF)<<8) | ((unsigned short)(x)>>8))
    # define le64_to_cpu(x) (DONTUSE__NXSwapBigDoubleToLittle(x))
    # else
    #  define le32_to_cpu(x) (x)
    #  define le16_to_cpu(x) (x)
    #  define le64_to_cpu(x) (x)
    # endif
    
    #elif __BYTE_ORDER == __BIG_ENDIAN && !defined(BSD) && !defined(__FreeBSD__)
    # include <byteswap.h>
    # define le32_to_cpu(x) (bswap_32(x))
    # define le16_to_cpu(x) (bswap_16(x))
    # define le64_to_cpu(x) (DONTUSE__NXSwapBigDoubleToLittle(x))
    #else
    # define le32_to_cpu(x) (x)
    # define le16_to_cpu(x) (x)
    #  define le64_to_cpu(x) (x)
    #endif
#endif
#endif	// _ENDIANNESS_H
