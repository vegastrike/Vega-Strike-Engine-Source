#ifndef _ENDIANNESS_H

#define _ENDIANNESS_H
#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN

# include <byteswap.h>
# define le32_to_cpu(x) (bswap_32(x))
# define le16_to_cpu(x) (bswap_16(x))

#else

# define le32_to_cpu(x) (x)
# define le16_to_cpu(x) (x)

#endif	// _ENDIANNESS_H
