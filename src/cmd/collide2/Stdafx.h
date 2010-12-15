///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//chuck starchaser shutting up the warnings about violations of strict aliasing...
#ifndef _WIN32 // pheonixstorm shutting up pragma warnings on win32
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
//believe me I tried to fix the code, using a union of float, int and unsigned, but the amount of
//changes needed in the code was immense.
//So, bottom line:
//    assert( sizeof(float) == sizeof(int) == sizeof(unsigned int) == sizeof(dword) ) or this library won't work
//also
//    assert( (dword&)float(0.0f) == (dword&)int(0) ) must pass or this library will blow on your face

#include "opcodetypes.h"
#include "opcodesysdef.h"
#include "Opcode.h"

