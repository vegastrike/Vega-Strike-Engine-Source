/*
 * posh.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 *  @file    posh.c
 *  @author  Brian Hook
 *  @version 0.0.
 *  @date    2002
 *  @brief   Portable Open Source Harness primary source file
 */
#include "posh.h"

#if !defined POSH_NO_FLOAT
#  define POSH_FLOAT_STRING "enabled"
#else
#  define POSH_FLOAT_STRING "disabled"
#endif

#if defined POSH_64BIT_INTEGER
#  define POSH_64BIT_INTEGER_STRING "yes"
#else
#  define POSH_64BIT_INTEGER_STRING "no"
#endif

#if defined POSH_64BIT_POINTER
#  define POSH_POINTER_STRING "64-bits"
#else
#  define POSH_POINTER_STRING "32-bits"
#endif

#if defined POSH_LITTLE_ENDIAN
#  define IS_BIG_ENDIAN 0

#  define NATIVE16 POSH_LittleU16
#  define NATIVE32 POSH_LittleU32
#  define NATIVE64 POSH_LittleU64
#  define FOREIGN16 POSH_BigU16
#  define FOREIGN32 POSH_BigU32
#  define FOREIGN64 POSH_BigU64
#else
#  define IS_BIG_ENDIAN 1

#  define NATIVE16 POSH_BigU16
#  define NATIVE32 POSH_BigU32
#  define NATIVE64 POSH_BigU64
#  define FOREIGN16 POSH_LittleU16
#  define FOREIGN32 POSH_LittleU32
#  define FOREIGN64 POSH_LittleU64
#endif

static
int s_testBigEndian(void) {
    union {
        posh_byte_t c[4];
        posh_u32_t i;
    }
            u;

    u.i = 1;
    if (u.c[0] == 1) {
        return 0;
    }
    return 1;
}

static
const char *s_testSerialization(void) {
    posh_byte_t serbuf[8];
    posh_u16_t tmp16;
    posh_u32_t tmp32;

    /* 16-bit serialization */
    POSH_WriteU16ToLittle(serbuf, 0xABCD);
    if ((tmp16 = POSH_ReadU16FromLittle(serbuf)) != 0xABCD) {
        return "*ERROR: failed little-endian 16-bit serialization test";
    }
    POSH_WriteU16ToBig(serbuf, 0xABCD);
    if ((tmp16 = POSH_ReadU16FromBig(serbuf)) != 0xABCD) {
        return "*ERROR: failed big-endian 16-bit serialization test";
    }
    /* 32-bit serialization */
    POSH_WriteU32ToLittle(serbuf, 0xABCD1234L);
    if ((tmp32 = POSH_ReadU32FromLittle(serbuf)) != 0xABCD1234) {
        return "*ERROR: failed little-endian 32-bit serialization test";
    }
    POSH_WriteU32ToBig(serbuf, 0xABCD1234L);
    if ((tmp32 = POSH_ReadU32FromBig(serbuf)) != 0xABCD1234) {
        return "*ERROR: failed big-endian 32-bit serialization test";
    }
#if defined POSH_64BIT_INTEGER
    {
#define REF64 POSH_U64( 0xFEDCBA9876543210 )

        posh_u64_t tmp64;

        POSH_WriteU64ToLittle(serbuf, REF64);
        if ((tmp64 = POSH_ReadU64FromLittle(serbuf)) != REF64) {
            return "*ERROR: failed little-endian 64-bit serialization test";
        }
        POSH_WriteU64ToBig(serbuf, REF64);
        if ((tmp64 = POSH_ReadU64FromBig(serbuf)) != REF64) {
            return "*ERROR: failed big-endian 64-bit serialization test";
        }
    }
#endif

    return 0;
}

#if !defined POSH_NO_FLOAT

static
const char *s_testFloatingPoint(void) {
    float fRef = 10.0f / 30.0f;
    double dRef = 10.0 / 30.0;
    posh_byte_t dbuf[8];
    float fTmp;
    double dTmp;

    fTmp = POSH_FloatFromLittleBits(POSH_LittleFloatBits(fRef));
    if (fTmp != fRef) {
        return "*ERROR: POSH little endian floating point conversion failed.  Please report this to poshlib@poshlib.org!\n";
    }
    fTmp = POSH_FloatFromBigBits(POSH_BigFloatBits(fRef));
    if (fTmp != fRef) {
        return "*ERROR: POSH big endian floating point conversion failed.  Please report this to poshlib@poshlib.org!\n";
    }
    POSH_DoubleBits(dRef, dbuf);

    dTmp = POSH_DoubleFromBits(dbuf);
    if (dTmp != dRef) {
        return
                "*ERROR: POSH double precision floating point serialization failed.  Please report this to poshlib@poshlib.org!\n";
    }
    return 0;
}

#endif /* !defined POSH_NO_FLOAT */

static
const char *s_testEndianess(void) {
    /* check endianess */
    if (s_testBigEndian() != IS_BIG_ENDIAN) {
        return
                "*ERROR: POSH compile time endianess does not match run-time endianess verification.  Please report this to poshlib@poshlib.org!\n";
    }
    /* make sure our endian swap routines work */
    if ((NATIVE32(0x11223344L) != 0x11223344L)
            || (FOREIGN32(0x11223344L) != 0x44332211L)
            || (NATIVE16(0x1234) != 0x1234)
            || (FOREIGN16(0x1234) != 0x3412)) {
        return "*ERROR: POSH endianess macro selection failed.  Please report this to poshlib@poshlib.org!\n";
    }
    /* test serialization routines */

    return 0;
}

/**
 *  Returns a string describing this platform's basic attributes.
 *
 *  POSH_GetArchString() reports on an architecture's statically determined
 *  attributes.  In addition, it will perform run-time verification checks
 *  to make sure the various platform specific functions work.  If an error
 *  occurs, please contact me at poshlib@poshlib.org so we can try to resolve
 *  what the specific failure case is.
 *  @returns a string describing this platform on success, or a string in the
 *          form "*ERROR: [text]" on failure.  You can simply check to see if
 *          the first character returned is '*' to verify an error condition.
 */
const char *POSH_GetArchString(void) {
    const char *err;
    const char *s = "OS:.............." POSH_OS_STRING "\n"
                    "CPU:............." POSH_CPU_STRING "\n"
                    "endian:.........." POSH_ENDIAN_STRING "\n"
                    "ptr size:........" POSH_POINTER_STRING "\n"
                    "64-bit ints......" POSH_64BIT_INTEGER_STRING "\n"
                    "floating point..." POSH_FLOAT_STRING "\n";

    /* test endianess */
    err = s_testEndianess();
    if (err != 0) {
        return err;
    }
    /* test serialization */
    err = s_testSerialization();
    if (err != 0) {
        return err;
    }
#if !defined POSH_NO_FLOAT
    /* check that our floating point support is correct */
    err = s_testFloatingPoint();
    if (err != 0) {
        return err;
    }
#endif

    return s;
}

/*
 * ---------------------------------------------------------------------------
 *                           BYTE SWAPPING SUPPORT
 * ---------------------------------------------------------------------------
 */
/**
 * Byte swaps a 16-bit unsigned value
 *
 *  @param v[in] unsigned 16-bit input value to swap
 *  @returns a byte swapped version of v
 */
posh_u16_t POSH_SwapU16(posh_u16_t v) {
    posh_u16_t swapped;

    swapped = v << 8;
    swapped |= v >> 8;

    return swapped;
}

/**
 * Byte swaps a 16-bit signed value
 *
 *  @param v[in] signed 16-bit input value to swap
 *  @returns a byte swapped version of v
 *  @remarks This just calls back to the unsigned version, since byte swapping
 *           is independent of sign.  However, we still provide this function to
 *           avoid signed/unsigned mismatch compiler warnings.
 */
posh_s16_t POSH_SwapS16(posh_s16_t v) {
    return (posh_s16_t) POSH_SwapU16(v);
}

/**
 * Byte swaps a 32-bit unsigned value
 *
 *  @param v[in] unsigned 32-bit input value to swap
 *  @returns a byte swapped version of v
 */
posh_u32_t POSH_SwapU32(posh_u32_t v) {
    posh_u32_t swapped;

    swapped = (v & 0xFF) << 24;
    swapped |= (v & 0xFF00) << 8;
    swapped |= (v >> 8) & 0xFF00;
    swapped |= (v >> 24);

    return swapped;
}

/**
 * Byte swaps a 32-bit signed value
 *
 *  @param v[in] signed 32-bit input value to swap
 *  @returns a byte swapped version of v
 *  @remarks This just calls back to the unsigned version, since byte swapping
 *           is independent of sign.  However, we still provide this function to
 *           avoid signed/unsigned mismatch compiler warnings.
 */
posh_s32_t POSH_SwapS32(posh_s32_t v) {
    return (posh_s32_t) POSH_SwapU32((posh_u32_t) v);
}

#if defined POSH_64BIT_INTEGER

/**
 * Byte swaps a 64-bit unsigned value
 *
 *  @param v[in] a 64-bit input value to swap
 *  @ingroup SixtyFourBit
 *  @returns a byte swapped version of v
 */
posh_u64_t POSH_SwapU64(posh_u64_t v) {
    posh_byte_t tmp;
    union {
        posh_byte_t bytes[8];
        posh_u64_t u64;
    }
            u;

    u.u64 = v;

    tmp = u.bytes[0];
    u.bytes[0] = u.bytes[7];
    u.bytes[7] = tmp;
    tmp = u.bytes[1];
    u.bytes[1] = u.bytes[6];
    u.bytes[6] = tmp;
    tmp = u.bytes[2];
    u.bytes[2] = u.bytes[5];
    u.bytes[5] = tmp;
    tmp = u.bytes[3];
    u.bytes[3] = u.bytes[4];
    u.bytes[4] = tmp;

    return u.u64;
}

/**
 * Byte swaps a 64-bit signed value
 *
 *  @param v[in] a 64-bit input value to swap
 *  @ingroup SixtyFourBit
 *  @returns a byte swapped version of v
 */
posh_s64_t POSH_SwapS64(posh_s64_t v) {
    return (posh_s64_t) POSH_SwapU64((posh_u64_t) v);
}

#endif /* defined POSH_64BIT_INTEGER */

/*
 * ---------------------------------------------------------------------------
 *                           IN-MEMORY SERIALIZATION
 * ---------------------------------------------------------------------------
 */

/**
 * Writes an unsigned 16-bit value to a little endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 16-bit value
 *  @returns a pointer to the location two bytes after dst
 *  @remarks does no validation of the inputs
 */
posh_u16_t *POSH_WriteU16ToLittle(void *dst, posh_u16_t value) {
    posh_u16_t *p16 = (posh_u16_t *) dst;

    *p16 = POSH_LittleU16(value);

    return p16 + 2;
}

/**
 * Writes a signed 16-bit value to a little endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 16-bit value
 *  @returns a pointer to the location two bytes after dst
 *  @remarks does no validation of the inputs.  This simply calls
 *         POSH_WriteU16ToLittle() with appropriate casting.
 */
posh_s16_t *POSH_WriteS16ToLittle(void *dst, posh_s16_t value) {
    return (posh_s16_t *) POSH_WriteU16ToLittle(dst, (posh_u16_t) value);
}

/**
 * Writes an unsigned 32-bit value to a little endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 32-bit value
 *  @returns a pointer to the location four bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_u32_t *POSH_WriteU32ToLittle(void *dst, posh_u32_t value) {
    posh_u32_t *p32 = (posh_u32_t *) dst;

    *p32 = POSH_LittleU32(value);

    return p32 + 4;
}

/**
 * Writes a signed 32-bit value to a little endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 32-bit value
 *  @returns a pointer to the location four bytes after dst
 *  @remarks does no validation of the inputs.  This simply calls
 *         POSH_WriteU32ToLittle() with appropriate casting.
 */
posh_s32_t *POSH_WriteS32ToLittle(void *dst, posh_s32_t value) {
    return (posh_s32_t *) POSH_WriteU32ToLittle(dst, (posh_u32_t) value);
}

/**
 * Writes an unsigned 16-bit value to a big endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 16-bit value
 *  @returns a pointer to the location two bytes after dst
 *  @remarks does no validation of the inputs
 */
posh_u16_t *POSH_WriteU16ToBig(void *dst, posh_u16_t value) {
    posh_u16_t *p16 = (posh_u16_t *) dst;

    *p16 = POSH_BigU16(value);

    return p16 + 2;
}

/**
 * Writes a signed 16-bit value to a big endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 16-bit value
 *  @returns a pointer to the location two bytes after dst
 *  @remarks does no validation of the inputs.  This simply calls
 *         POSH_WriteU16ToLittle() with appropriate casting.
 */
posh_s16_t *POSH_WriteS16ToBig(void *dst, posh_s16_t value) {
    return (posh_s16_t *) POSH_WriteU16ToBig(dst, (posh_u16_t) value);
}

/**
 * Writes an unsigned 32-bit value to a big endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 32-bit value
 *  @returns a pointer to the location four bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_u32_t *POSH_WriteU32ToBig(void *dst, posh_u32_t value) {
    posh_u32_t *p32 = (posh_u32_t *) dst;

    *p32 = POSH_BigU32(value);

    return p32 + 4;
}

/**
 * Writes a signed 32-bit value to a big endian buffer
 *
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 32-bit value
 *  @returns a pointer to the location four bytes after dst
 *  @remarks does no validation of the inputs.  This simply calls
 *         POSH_WriteU32ToBig() with appropriate casting.
 */
posh_s32_t *POSH_WriteS32ToBig(void *dst, posh_s32_t value) {
    return (posh_s32_t *) POSH_WriteU32ToBig(dst, (posh_u32_t) value);
}

#if defined POSH_64BIT_INTEGER

/**
 * Writes an unsigned 64-bit value to a little-endian buffer
 *
 *  @ingroup SixtyFourBit
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 64-bit value
 *  @returns a pointer to the location eight bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_u64_t *POSH_WriteU64ToLittle(void *dst, posh_u64_t value) {
    posh_u64_t *p64 = (posh_u64_t *) dst;

    *p64 = POSH_LittleU64(value);

    return p64 + 8;
}

/**
 * Writes a signed 64-bit value to a little-endian buffer
 *
 *  @ingroup SixtyFourBit
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 64-bit value
 *  @returns a pointer to the location eight bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_s64_t *POSH_WriteS64ToLittle(void *dst, posh_s64_t value) {
    return (posh_s64_t *) POSH_WriteU64ToLittle(dst, (posh_u64_t) value);
}

/**
 * Writes an unsigned 64-bit value to a big-endian buffer
 *
 *  @ingroup SixtyFourBit
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian unsigned 64-bit value
 *  @returns a pointer to the location eight bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_u64_t *POSH_WriteU64ToBig(void *dst, posh_u64_t value) {
    posh_u64_t *p64 = (posh_u64_t *) dst;

    *p64 = POSH_BigU64(value);

    return p64 + 8;
}

/**
 * Writes a signed 64-bit value to a big-endian buffer
 *
 *  @ingroup SixtyFourBit
 *  @param dst[out] pointer to the destination buffer, may not be NULL
 *  @param value[in] host-endian signed 64-bit value
 *  @returns a pointer to the location eight bytes after dst
 *  @remarks does no validation of the inputs.
 */
posh_s64_t *POSH_WriteS64ToBig(void *dst, posh_s64_t value) {
    return (posh_s64_t *) POSH_WriteU64ToBig(dst, (posh_s64_t) value);
}

#endif /* POSH_64BIT_INTEGER */

/*
 * ---------------------------------------------------------------------------
 *                         IN-MEMORY DESERIALIZATION
 * ---------------------------------------------------------------------------
 */

/**
 * Reads an unsigned 16-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 16-bit value
 */
posh_u16_t POSH_ReadU16FromLittle(const void *src) {
    return POSH_LittleU16((*(const posh_u16_t *) src));
}

/**
 * Reads a signed 16-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 16-bit value
 */
posh_s16_t POSH_ReadS16FromLittle(const void *src) {
    return POSH_LittleS16((*(const posh_s16_t *) src));
}

/**
 * Reads an unsigned 32-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 32-bit value
 */
posh_u32_t POSH_ReadU32FromLittle(const void *src) {
    return POSH_LittleU32((*(const posh_u32_t *) src));
}

/**
 * Reads a signed 32-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 32-bit value
 */
posh_s32_t POSH_ReadS32FromLittle(const void *src) {
    return POSH_LittleS32((*(const posh_s32_t *) src));
}

/**
 * Reads an unsigned 16-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 16-bit value
 */
posh_u16_t POSH_ReadU16FromBig(const void *src) {
    return POSH_BigU16((*(const posh_u16_t *) src));
}

/**
 * Reads a signed 16-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 16-bit value
 */
posh_s16_t POSH_ReadS16FromBig(const void *src) {
    return POSH_BigS16((*(const posh_s16_t *) src));
}

/**
 * Reads an unsigned 32-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 32-bit value
 */
posh_u32_t POSH_ReadU32FromBig(const void *src) {
    return POSH_BigU32((*(const posh_u32_t *) src));
}

/**
 * Reads a signed 32-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 32-bit value
 */
posh_s32_t POSH_ReadS32FromBig(const void *src) {
    return POSH_BigS32((*(const posh_s32_t *) src));
}

#if defined POSH_64BIT_INTEGER

/**
 * Reads an unsigned 64-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 32-bit value
 */
posh_u64_t POSH_ReadU64FromLittle(const void *src) {
    return POSH_LittleU64((*(const posh_u64_t *) src));
}

/**
 * Reads a signed 64-bit value from a little-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 32-bit value
 */
posh_s64_t POSH_ReadS64FromLittle(const void *src) {
    return POSH_LittleS64((*(const posh_s64_t *) src));
}

/**
 * Reads an unsigned 64-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian unsigned 32-bit value
 */
posh_u64_t POSH_ReadU64FromBig(const void *src) {
    return POSH_BigU64((*(const posh_u64_t *) src));
}

/**
 * Reads an signed 64-bit value from a big-endian buffer
 *  @param src[in] source buffer
 *  @returns host-endian signed 32-bit value
 */
posh_s64_t POSH_ReadS64FromBig(const void *src) {
    return POSH_BigS64((*(const posh_s64_t *) src));
}

#endif /* POSH_64BIT_INTEGER */

/*
 * ---------------------------------------------------------------------------
 *                           FLOATING POINT SUPPORT
 * ---------------------------------------------------------------------------
 */

#if !defined POSH_NO_FLOAT

/**
 * Extracts raw little-endian bits from a 32-bit floating point value
 *
 *  @ingroup FloatingPoint
 *  @param f[in] floating point value
 *  @returns a little-endian bit representation of f
 */
posh_u32_t POSH_LittleFloatBits(float f) {
    union {
        float f32;
        posh_u32_t u32;
    }
            u;

    u.f32 = f;

#if defined POSH_LITTLE_ENDIAN
    return u.u32;

#else
    return POSH_SwapU32( u.u32 );
#endif
}

/**
 * Extracts raw big-endian bits from a 32-bit floating point value
 *
 *  @ingroup FloatingPoint
 *  @param   f[in] floating point value
 *  @returns a big-endian bit representation of f
 */
posh_u32_t POSH_BigFloatBits(float f) {
    union {
        float f32;
        posh_u32_t u32;
    }
            u;

    u.f32 = f;

#if defined POSH_LITTLE_ENDIAN
    return POSH_SwapU32(u.u32);

#else
    return u.u32;
#endif
}

/**
 * Extracts raw, little-endian bit representation from a 64-bit double.
 *
 *  @param d[in] 64-bit double precision value
 *  @param dst[out] 8-byte storage buffer
 *  @ingroup FloatingPoint
 *  @returns the raw bits used to represent the value 'd', in the form dst[0]=LSB
 */
void POSH_DoubleBits(double d, posh_byte_t dst[8]) {
    union {
        double d64;
        posh_byte_t bytes[8];
    }
            u;

    u.d64 = d;

#if defined POSH_LITTLE_ENDIAN
    dst[0] = u.bytes[0];
    dst[1] = u.bytes[1];
    dst[2] = u.bytes[2];
    dst[3] = u.bytes[3];
    dst[4] = u.bytes[4];
    dst[5] = u.bytes[5];
    dst[6] = u.bytes[6];
    dst[7] = u.bytes[7];
#else
    dst[0] = u.bytes[7];
    dst[1] = u.bytes[6];
    dst[2] = u.bytes[5];
    dst[3] = u.bytes[4];
    dst[4] = u.bytes[3];
    dst[5] = u.bytes[2];
    dst[6] = u.bytes[1];
    dst[7] = u.bytes[0];
#endif
}

/**
 * Creates a double-precision, 64-bit floating point value from a set if raw,
 * little-endian bits
 *
 *  @ingroup FloatingPoint
 *  @param src[in] little-endian byte representation of 64-bit double precision
 *                 floating point value
 *  @returns double precision floating point representation of the raw bits
 *  @remarks No error checking is performed, so there are no guarantees that the
 *           result is a valid number, nor is there any check to ensure that src is
 *           non-NULL.  BE CAREFUL USING THIS.
 */
double POSH_DoubleFromBits(const posh_byte_t src[8]) {
    union {
        double d64;
        posh_byte_t bytes[8];
    }
            u;

#if defined POSH_LITTLE_ENDIAN
    u.bytes[0] = src[0];
    u.bytes[1] = src[1];
    u.bytes[2] = src[2];
    u.bytes[3] = src[3];
    u.bytes[4] = src[4];
    u.bytes[5] = src[5];
    u.bytes[6] = src[6];
    u.bytes[7] = src[7];
#else
    u.bytes[0] = src[7];
    u.bytes[1] = src[6];
    u.bytes[2] = src[5];
    u.bytes[3] = src[4];
    u.bytes[4] = src[3];
    u.bytes[5] = src[2];
    u.bytes[6] = src[1];
    u.bytes[7] = src[0];
#endif

    return u.d64;
}

/**
 * Creates a floating point number from little endian bits
 *
 *  @ingroup FloatingPoint
 *  @param   bits[in] raw floating point bits in little-endian form
 *  @returns a floating point number based on the given bit representation
 *  @remarks No error checking is performed, so there are no guarantees that the
 *           result is a valid number.  BE CAREFUL USING THIS.
 */
float POSH_FloatFromLittleBits(posh_u32_t bits) {
    union {
        float f32;
        posh_u32_t u32;
    }
            u;

    u.u32 = bits;
#if defined POSH_BIG_ENDIAN
    u.u32 = POSH_SwapU32( u.u32 );
#endif

    return u.f32;
}

/**
 * Creates a floating point number from big-endian bits
 *
 *  @ingroup FloatingPoint
 *  @param   bits[in] raw floating point bits in big-endian form
 *  @returns a floating point number based on the given bit representation
 *  @remarks No error checking is performed, so there are no guarantees that the
 *           result is a valid number.  BE CAREFUL USING THIS.
 */
float POSH_FloatFromBigBits(posh_u32_t bits) {
    union {
        float f32;
        posh_u32_t u32;
    }
            u;

    u.u32 = bits;
#if defined POSH_LITTLE_ENDIAN
    u.u32 = POSH_SwapU32(u.u32);
#endif

    return u.f32;
}

#endif /* !defined POSH_NO_FLOAT */

