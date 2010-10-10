#ifndef __AL_INCLUDES__INCLUDED__
#define __AL_INCLUDES__INCLUDED__

#include "config.h"

#ifdef __APPLE__
    #include <al.h>
    #include <alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

typedef ALuint ALSourceHandle;
typedef ALuint ALBufferHandle;

#define AL_NULL_BUFFER (ALBufferHandle(0))

#ifndef AL_SEC_OFFSET
/* Supported on Windows, but the headers might be out of date. */
#define AL_SEC_OFFSET 0x1024
#endif
#if (!defined(ALC_INVALID_ENUM) && defined(ALC_ILLEGAL_ENUM))
#define ALC_INVALID_ENUM ALC_ILLEGAL_ENUM
#endif


#endif//__AL_INCLUDES__INCLUDED__

