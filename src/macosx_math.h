#ifndef MACOSX_MATH
#define MACOSX_MATH

#if defined(__APPLE__) || defined(MACOSX)
    #include <math.h>
    extern float sqrtf(float v);
    extern float cosf (float v);
    extern float sinf (float v);
    extern float tanf (float v);
    extern float powf (float v, float p);
#endif

#endif
