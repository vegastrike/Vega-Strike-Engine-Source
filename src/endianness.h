#ifndef _ENDIANNESS_H

#define _ENDIANNESS_H

#ifdef macintosh
#include <byteswap.h>
#define le32_to_cpu(x) (bswap_32(x))
#define le16_to_cpu(x) (bswap_16(x))
#else
#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)
#endif

#endif	// _ENDIANNESS_H
