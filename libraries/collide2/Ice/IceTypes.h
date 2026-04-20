///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom types.
 *  \file		IceTypes.h
 *  \author		Pierre Terdiman
 *  \date		April, 4, 2000
 *
 *  Copyright (C) 1998-2026 Pierre Terdiman, Stephen G. Tuggy, Benjamen R. Meyer
 *  Public Domain
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_LIBRARIES_COLLIDE2_ICE_ICE_TYPES_H
#define VEGA_STRIKE_LIBRARIES_COLLIDE2_ICE_ICE_TYPES_H

#include <numbers>

#define USE_HANDLE_MANAGER

// Constants
#ifndef PI
#define	PI                      (std::numbers::pi_v<float>)                                 //!< PI
#endif
#define    HALFPI               (std::numbers::pi_v<float> / 2.0F)                          //!< 0.5 * PI
#define    TWOPI                (2.0F * std::numbers::pi_v<float>)                          //!< 2.0 * PI
#define    INVPI                (std::numbers::inv_pi_v<float>)                             //!< 1.0 / PI

#define    RADTODEG             (180.0F / std::numbers::pi_v<float>)                        //!< 180.0 / PI, convert radians to degrees
#define    DEGTORAD             (std::numbers::pi_v<float> / 180.0F)                        //!< PI / 180.0, convert degrees to radians

#define    EXP                  (std::numbers::e_v<float>)                                  //!< e
#define    INVLOG2              3.32192809488736234787F                                     //!< 1.0 / log10(2)
#define    LN2                  (std::numbers::ln2_v<float>)                                //!< ln(2)
#define    INVLN2               (1.0F / std::numbers::ln2_v<float>)                         //!< 1.0f / ln(2)

#define    INV3                 0.33333333333333333333F                                     //!< 1/3
#define    INV6                 0.16666666666666666666F                                     //!< 1/6
#define    INV7                 0.14285714285714285714F                                     //!< 1/7
#define    INV9                 0.11111111111111111111F                                     //!< 1/9
#define    INV255               0.00392156862745098039F                                     //!< 1/255

#define    SQRT2                (std::numbers::sqrt2_v<float>)                              //!< sqrt(2)
#define    INVSQRT2             (1.0F / std::numbers::sqrt2_v<float>)                       //!< 1 / sqrt(2)

#define    SQRT3                (std::numbers::sqrt3_v<float>)                              //!< sqrt(3)
#define    INVSQRT3             (std::numbers::inv_sqrt3_v<float>)                          //!< 1 / sqrt(3)

// #define null				0														//!< our own NULL pointer

// // Custom types used in ICE
// typedef int8_t              int8_t;      //!< sizeof(int8_t)      must be 1
// typedef uint8_t             uint8_t;      //!< sizeof(uint8_t)      must be 1
// typedef int16_t             int16_t;     //!< sizeof(int16_t)     must be 2
// typedef uint16_t            uint16_t;     //!< sizeof(uint16_t)     must be 2
// typedef int32_t             int32_t;     //!< sizeof(int32_t)     must be 4
// typedef uint32_t            uint32_t;     //!< sizeof(uint32_t)     must be 4
// typedef int64_t             int64_t;     //!< sizeof(int64_t)	    must be 8
// typedef uint64_t            uint64_t;     //!< sizeof(uint64_t)     must be 8
// typedef float               float;    //!< sizeof(float)    must be 4
// typedef double              ice_float64;    //!< sizeof(ice_float64)    must be 8

// ICE_COMPILE_TIME_ASSERT(sizeof(bool)==1);	// ...otherwise things might fail with VC++ 4.2 !
ICE_COMPILE_TIME_ASSERT(sizeof(uint8_t) == 1);
ICE_COMPILE_TIME_ASSERT(sizeof(int8_t) == 1);
ICE_COMPILE_TIME_ASSERT(sizeof(int16_t) == 2);
ICE_COMPILE_TIME_ASSERT(sizeof(uint16_t) == 2);
ICE_COMPILE_TIME_ASSERT(sizeof(uint32_t) == 4);
ICE_COMPILE_TIME_ASSERT(sizeof(int32_t) == 4);
ICE_COMPILE_TIME_ASSERT(sizeof(uint64_t) == 8);
ICE_COMPILE_TIME_ASSERT(sizeof(int64_t) == 8);

//! TO BE DOCUMENTED
#define DECLARE_ICE_HANDLE(name)    struct name##__ { int unused; }; typedef struct name##__ *name

typedef uint32_t DynID;        //!< Dynamic identifier
#ifdef USE_HANDLE_MANAGER
typedef uint32_t KID;        //!< Kernel ID
//	DECLARE_ICE_HANDLE(KID);
#else
typedef uint16_t            KID;		//!< Kernel ID
#endif
typedef uint32_t RTYPE;        //!< Relationship-type (!) between owners and references
#define    INVALID_ID            0xffffffff    //!< Invalid dword ID (counterpart of null pointers)
#ifdef USE_HANDLE_MANAGER
#define    INVALID_KID            0xffffffff    //!< Invalid Kernel ID
#else
#define	INVALID_KID			0xffff		//!< Invalid Kernel ID
#endif
#define    INVALID_NUMBER        0xDEADBEEF    //!< Standard junk value

// Define BOOL if needed
#ifndef BOOL
typedef int BOOL;                        //!< Another boolean type.
#endif

//! Union of a float and a int32_t
typedef union {
    float f;                         //!< The float
    int32_t d;                         //!< The integer
} scell;

//! Union of a float and a uint32_t
typedef union {
    float f;                         //!< The float
    uint32_t d;                         //!< The integer
} ucell;

// Type ranges
// #define	MAX_SBYTE				INT8_MAX				    //!< max possible int8_t value
// #define	MIN_SBYTE				INT8_MIN    				//!< min possible int8_t value
// #define	MAX_UBYTE				UINT8_MAX   				//!< max possible uint8_t value
#define    MIN_UBYTE                uint8_t(0x00)                //!< min possible uint8_t value
// #define	MAX_SWORD				INT16_MAX       			//!< max possible int16_t value
// #define	MIN_SWORD				INT16_MIN       			//!< min possible int16_t value
// #define	MAX_UWORD				UINT16_MAX					//!< max possible uint16_t value
#define    MIN_UWORD                uint16_t(0x0000)            //!< min possible uint16_t value
// #define	MAX_SDWORD				INT32_MAX					//!< max possible int32_t value
// #define	MIN_SDWORD				INT32_MIN					//!< min possible int32_t value
// #define	MAX_UDWORD				UINT32_MAX					//!< max possible uint32_t value
#define    MIN_UDWORD                uint32_t(0x00000000)        //!< min possible uint32_t value
#define    MAX_FLOAT                FLT_MAX                        //!< max possible float value
#define    MIN_FLOAT                (-FLT_MAX)                    //!< min possible loat value
#define IEEE_1_0                0x3f800000                    //!< integer representation of 1.0
#define IEEE_255_0                0x437f0000                    //!< integer representation of 255.0
#define IEEE_MAX_FLOAT            0x7f7fffff                    //!< integer representation of MAX_FLOAT
#define IEEE_MIN_FLOAT            0xff7fffff                    //!< integer representation of MIN_FLOAT
#define IEEE_UNDERFLOW_LIMIT    0x1a000000

#define ONE_OVER_RAND_MAX        (1.0f / float(RAND_MAX))    //!< Inverse of the max possible value returned by rand()

typedef bool
(*ENUMERATION)(uint32_t value, uint32_t param, uint32_t context); //!< ICE standard enumeration call
typedef void **VTABLE;                            //!< A V-Table.

#undef        MIN
#undef        MAX
#define        MIN(a, b)       ((a) < (b) ? (a) : (b))            //!< Returns the min value between a and b
#define        MAX(a, b)       ((a) > (b) ? (a) : (b))            //!< Returns the max value between a and b
#define        MAXMAX(a, b, c)   ((a) > (b) ? MAX (a,c) : MAX (b,c))    //!<	Returns the max value between a, b and c

template<class T>
inline_ const T &TMin(const T &a, const T &b) {
    return b < a ? b : a;
}

template<class T>
inline_ const T &TMax(const T &a, const T &b) {
    return a < b ? b : a;
}

template<class T>
inline_ void TSetMin(T &a, const T &b) {
    if (a > b) {
        a = b;
    }
}

template<class T>
inline_ void TSetMax(T &a, const T &b) {
    if (a < b) {
        a = b;
    }
}

#define        SQR(x)            ((x)*(x))                        //!< Returns x square
#define        CUBE(x)            ((x)*(x)*(x))                    //!< Returns x cube

#define        AND        &                                        //!< ...
#define        OR        |                                        //!< ...
#define        XOR        ^                                        //!< ...

#define        QUADRAT(x)        ((x)*(x))                        //!< Returns x square

#ifdef _WIN32
#   define srand48(x) srand((unsigned int) (x))
#	define srandom(x) srand((unsigned int) (x))
#	define random()   ((double) rand())
#   define drand48()  ((double) (((double) rand()) / ((double) RAND_MAX)))
#endif

#endif //VEGA_STRIKE_LIBRARIES_COLLIDE2_ICE_ICE_TYPES_H
