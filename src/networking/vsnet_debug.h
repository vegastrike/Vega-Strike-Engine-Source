#ifndef VSNET_DEBUG_H
#define VSNET_DEBUG_H

#include <config.h>
#include <iostream>

#if !defined( COUT)
	#if defined(_WIN32) && defined(_MSC_VER) && defined(USE_BOOST_129) //wierd error in MSVC
	#define COUT std::clog << __FILE__ << ":"
	#else
	#define COUT std::clog << __FILE__ << ":" << __LINE__ << " "
	#endif
#endif

#ifndef NDEBUG
#define VSNET_DEBUG
#else
#undef VSNET_DEBUG
#endif

#define VSNET_DEBUG

#define FIND_WIN_NBIO

/* --- BEGIN memory debugging macros --- */
/** The following macros are intended for default versions a class validity
 *  debugging. The idea is to increase the likelyhood that uninitialized
 *  and re-initialized memory areas are captured as well as possible.
 */
#ifndef NDEBUG

  #include <assert.h>
  #include <stdio.h>

  #define DECLARE_VALID \
  private: \
      bool _valid; \
      bool _invalid; \
  public: \
      void validate( const char* file, int line ) const { \
	  if( !_valid || _invalid ) { \
	      fprintf( stderr, "object invalid in %s:%d\n", file, line ); \
	  } \
          assert( _valid ); \
          assert( !_invalid ); \
      }

  #define MAKE_VALID \
      _valid = true; \
      _invalid = false;

  #define MAKE_INVALID \
      _valid = false; \
      _invalid = true;

  #define CHECK_VALID \
      validate( __FILE__, __LINE__ );

  #define CHECK_VALID_OBJ(a) \
      (a).validate( __FILE__, __LINE__ );

  #define ASSERT(a) if(!(a)) { std::cerr << __FILE__ << ":" << __LINE__ << " assertion failed, forcing segfault for postmortem debugging"; int x = 1/0; }

#else /* NDEBUG */
  #define DECLARE_VALID
  #define MAKE_VALID
  #define MAKE_INVALID
  #define CHECK_VALID
  #define CHECK_VALID_OBJ(a)
  #define ASSERT(a)
#endif /* NDEBUG */
/* --- END memory debugging macros --- */

#if !defined( _WIN32) || defined( __CYGWIN__)
  #define PSEUDO__LINE__(x) __LINE__
#else
  #ifndef __PRETTY_FUNCTION
    #define __PRETTY_FUNCTION__ "<Unknown>"
  #endif

  #ifndef __FUNCTION__
    #define __FUNCTION__ "<Unknown>"
  #endif

  #if (defined(_WIN32)&&defined(_MSC_VER)&&(_MSC_VER<1300)&&defined(_DEBUG)&&defined(USE_BOOST_129))
    #define PSEUDO__LINE__(x) x
  #else
    #define PSEUDO__LINE__(x) __LINE__
  #endif

#endif

#endif /* VSNET_DEBUG_H */

