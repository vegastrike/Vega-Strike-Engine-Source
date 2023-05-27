///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains custom types.
 *	\file		IceTypes.h
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TYPES_H
#define VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TYPES_H

#define USE_HANDLE_MANAGER

// Constants
#ifndef PI
#define	PI					3.1415926535897932384626433832795028841971693993751f	//!< PI
#endif
#define    HALFPI                1.57079632679489661923f                                    //!< 0.5 * PI
#define    TWOPI                6.28318530717958647692f                                    //!< 2.0 * PI
#define    INVPI                0.31830988618379067154f                                    //!< 1.0 / PI

#define    RADTODEG            57.2957795130823208768f                                    //!< 180.0 / PI, convert radians to degrees
#define    DEGTORAD            0.01745329251994329577f                                    //!< PI / 180.0, convert degrees to radians

#define    EXP                    2.71828182845904523536f                                    //!< e
#define    INVLOG2                3.32192809488736234787f                                    //!< 1.0 / log10(2)
#define    LN2                    0.693147180559945f                                        //!< ln(2)
#define    INVLN2                1.44269504089f                                            //!< 1.0f / ln(2)

#define    INV3                0.33333333333333333333f                                    //!< 1/3
#define    INV6                0.16666666666666666666f                                    //!< 1/6
#define    INV7                0.14285714285714285714f                                    //!< 1/7
#define    INV9                0.11111111111111111111f                                    //!< 1/9
#define    INV255                0.00392156862745098039f                                    //!< 1/255

#define    SQRT2                1.41421356237f                                            //!< sqrt(2)
#define    INVSQRT2            0.707106781188f                                            //!< 1 / sqrt(2)

#define    SQRT3                1.73205080757f                                            //!< sqrt(3)
#define    INVSQRT3            0.577350269189f                                            //!< 1 / sqrt(3)

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

#endif //VEGA_STRIKE_ENGINE_CMD_COLLSION2_ICE_TYPES_H
