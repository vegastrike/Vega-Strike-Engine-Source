#include "vsnet_debug.h"

#if defined(_WIN32) && defined(_MSC_VER) && defined(USE_BOOST_129) //wierd error in MSVC
  /*
   * nothing if WIN32
   */
#else
#ifdef VSNET_DEBUG

#include <time.h>
#include <sys/time.h>

std::ostream& operator<<( std::ostream& ostr, const cout_time& c )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    ostr << tv.tv_sec << ":" << tv.tv_usec;
    return ostr;
}

#endif /* VSNET_DEBUG */
#endif

