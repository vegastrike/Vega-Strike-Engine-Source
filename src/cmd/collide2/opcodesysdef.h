#ifndef CS_COMPAT_H_
#define CS_COMPAT_H_


//#define OPC_USE_CALLBACKS 1

#ifndef CS_FORCEINLINE 
# ifdef CS_COMPILER_GCC 
#  define CS_FORCEINLINE inline __attribute__((always_inline)) 
#  if (__GNUC__ == 3) && (__GNUC_MINOR__ == 4) 
    // Work around a gcc 3.4 issue where forcing inline doesn't always work
#   define CS_FORCEINLINE_TEMPLATEMETHOD inline 
#  endif 
# else 
#  define CS_FORCEINLINE inline 
# endif 
#endif 
#ifndef CS_FORCEINLINE_TEMPLATEMETHOD 
# define CS_FORCEINLINE_TEMPLATEMETHOD CS_FORCEINLINE 
#endif 


#include "gfx/quaternion.h"
#define SMALL_EPSILON .000001
#define EPSILON .00001
#define ABS(x) (x>=0?x:-x)
#define __CS_CSSYSDEFS_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>


class csObject {
};
struct iBase {
};


#define CS_ASSERT assert

#include "opcodetypes.h"
#endif
