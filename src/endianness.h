#ifndef _ENDIANNESS_H

#define _ENDIANNESS_H
#ifndef _WIN32
#include <endian.h>
#else
#define __BIG_ENDIAN 1
#define __LITTLE_ENDIAN 0
#define __BYTE_ORDER 0
#endif
# if __BYTE_ORDER == __BIG_ENDIAN
#  include <byteswap.h>
#  define le32_to_cpu(x) (bswap_32(x))
#  define le16_to_cpu(x) (bswap_16(x))
# else
#  define le32_to_cpu(x) (x)
#  define le16_to_cpu(x) (x)
# endif

#endif	// _ENDIANNESS_H
