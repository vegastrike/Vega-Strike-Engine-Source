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


#endif//__AL_INCLUDES__INCLUDED__

