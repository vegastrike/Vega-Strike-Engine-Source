#ifndef MACOSX_MATH
#define MACOSX_MATH
/// works on *any* float... INF is 0x7ffffff and -INF is -(1<<31) NaN is 0
extern int float_to_int(float);
extern int double_to_int(float); // works on *any* float
#if defined(__APPLE__) || defined(MACOSX)
    #include <math.h>
    extern float sqrtf(float v);
    extern float cosf (float v);
    extern float sinf (float v);
    extern float tanf (float v);
    extern float powf (float v, float p);
#endif

#endif
