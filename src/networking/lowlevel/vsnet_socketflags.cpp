#include <config.h>

#include "vsnet_headers.h"
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#endif

#include "vsnet_socketflags.h"

using namespace std;

/***********************************************************************
 * PCKTFLAGS
 ***********************************************************************/
 
ostream& operator<<( ostream& ostr, PCKTFLAGS f )
{
    int flags = f;
    if( flags == 0 ) ostr << "NONE"; return ostr;
    if( flags & SENDANDFORGET ) ostr << "SENDANDFORGET ";
    if( flags & SENT          ) ostr << "SENT ";
    if( flags & RESENT        ) ostr << "RESENT ";
    if( flags & ACKED         ) ostr << "ACKED ";
    if( flags & SENDRELIABLE  ) ostr << "SENDRELIABLE ";
    if( flags & LOPRI         ) ostr << "LOPRI ";
    if( flags & HIPRI         ) ostr << "HIPRI ";
    if( flags & COMPRESSED    ) ostr << "COMPRESSED ";
    flags &= ~0x101f;
    if( flags != 0 ) ostr << hex << flags;
    return ostr;
}

