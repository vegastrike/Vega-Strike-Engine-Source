#include "vsnet_debug.h"

using namespace std;

#if (defined(_WIN32) && defined(_MSC_VER)) || defined( __MINGW32__)
  /*
   * nothing if WIN32
   */
ostream& vsnetDbgOut( const char* file, int line )
{
    clog << file << " ";
    return clog;
}

#else /* not _WIN32 or _MSC_VER */

#include <time.h>

#if !defined(_WIN32)
#include <sys/time.h>
#endif

struct TimeTriggerStruct { };

static TimeTriggerStruct time_trigger;

ostream& operator<<( ostream& ostr, const TimeTriggerStruct& c )
{
    struct timeval tv;
    gettimeofday( &tv, NULL );
    ostr << tv.tv_sec << ":" << tv.tv_usec;
    return ostr;
}

ostream& vsnetDbgOut( const char* file, int line )
{
    const char* printme = file;
    int   len = strlen(file);
    if( len > 30 ) printme = &file[len-30];

#ifdef VSNET_DEBUG
    clog << PTHREAD_SELF_OR_NONE << " "
         << time_trigger
         << " " << printme << ":" << line << " ";
#else /* !VSNET_DEBUG */
    clog << printme << ":" << line << " ";
#endif /* VSNET_DEBUG */
    return clog;
}

#endif

