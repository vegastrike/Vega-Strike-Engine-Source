#include <macosx_math.h>

#if defined(__APPLE__) || defined(MACOSX)

float sqrtf(float v)
{
    return (float) sqrt((double)v);
}

float cosf (float v)
{
    return (float) cos((double)v);
}

float sinf (float v)
{
    return (float) sin((double)v);
}

float tanf (float v)
{
    return (float) tan((double)v);
}

float powf (float v, float p)
{
    return (float) pow((double)v, (double) p);
}

#endif