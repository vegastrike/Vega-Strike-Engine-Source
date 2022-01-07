
// Should be included by Opcode.h if needed

#define ICE_DONT_CHECK_COMPILER_OPTIONS

// From Windows...
typedef int BOOL;
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#ifndef OPASSERT
#define    OPASSERT(exp)    {}
#endif
#define ICE_COMPILE_TIME_ASSERT(exp)    extern char ICE_Dummy[ (exp) ? 1 : -1 ]

//	extern void Opcode_Log (const char* msg, ...);
//	#define	Log			Opcode_Log
//#define	Log				{}
//	extern bool Opcode_Err (const char* msg, ...);
//	#define	SetIceError		Opcode_Err
#define    EC_OUTOFMEMORY    "Out of memory"

#include "Ice/IcePreprocessor.h"

#undef ICECORE_API
#define ICECORE_API    OPCODE_API

#include "Ice/IceTypes.h"
#include "Ice/IceFPU.h"
#include "Ice/IceMemoryMacros.h"
#define ICEMATHS_API    OPCODE_API
