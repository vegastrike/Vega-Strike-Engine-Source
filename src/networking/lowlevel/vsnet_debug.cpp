#include "vsnet_debug.h"

using namespace std;

#include <time.h>
#include <cstring>
#if !defined(_WIN32)
#include <sys/time.h>
#else
#include <winsock.h>
#endif

struct TimeTriggerStruct { };

static TimeTriggerStruct time_trigger;
ostream& operator<<( ostream& ostr, const TimeTriggerStruct& c )
{
#ifndef _WIN32
    struct timeval tv;
    gettimeofday( &tv, NULL );
    ostr << tv.tv_sec << ":" << tv.tv_usec;
#endif
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


