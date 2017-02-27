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

// Windows AL calls it "ILLEGAL" not "INVALID".....?
#if (!defined(ALC_INVALID_ENUM) && defined(ALC_ILLEGAL_ENUM))
#define ALC_INVALID_ENUM ALC_ILLEGAL_ENUM
#endif

// Both are major=0x1000 and minor=0x1001, but are completely different!?!?!?
#ifdef ALC_VERSION_0_1
#define ALCstring const ALCchar*
#else
#define ALCstring ALCubyte*
#endif

#endif//__AL_INCLUDES__INCLUDED__

