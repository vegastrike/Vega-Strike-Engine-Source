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

#if !defined(ALC_MAJOR_VERSION) || !defined(ALC_MINOR_VERSION) || ALC_MAJOR_VERSION < 0x1000 || ALC_MINOR_VERSION < 0x1001
#define ALCstring const ALCubyte*
#else
#define ALCstring const ALCchar*
#endif

#endif//__AL_INCLUDES__INCLUDED__

