#include <config.h>

#include "vsnet_dloadenum.h"
// #include "vsnet_dloadmgr.h"
// #include "vsnet_notify.h"
// #include "vsnet_cmd.h"
// #include "netbuffer.h"
// #include "packet.h"

using namespace std;

namespace VsnetDownload
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Subcommand
 *------------------------------------------------------------*/

/** Protocol:
 *
 *  resolve request
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand ResolveRequest
 *    int16                      : number of entries
 *    ( char,                    : list of filetype
 *      int16 stringlen, char* )*:     and filenames
 *
 *  resolve response
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                      : subcommand ResolveResponse
 *    int16                     : number of entries
 *    ( int16 stringlen, char*  : name
 *      char )*                 : ok(1), not ok(0)
 *
 *  download request
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadRequest
 *    int16                      : number of entries
 *    ( int16 stringlen, char* )*: list of filenames
 *
 *  download error
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadError
 *    ( int16 stringlen, char* ) : failed filename
 *    
 *  download entire file in one packet
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand Download
 *    ( int16 stringlen, char* ) : filename
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download first fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadFirstFragment
 *    ( int16 stringlen, char* ) : filename
 *    int32                      : file length
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download a middle fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadFragment
 *    int16                      : payload length
 *    char*                      : payload
 *
 *  download last fragment of a file
 *    CMD_DOWNLOAD, object serial 0, COMPRESS|SENDRELIABLE|LOPRI
 *    char                       : subcommand DownloadLastFragment
 *    int16                      : payload length
 *    char*                      : payload
 */

std::ostream& operator<<( std::ostream& ostr, Subcommand e )
{
    switch( e )
    {
    case ResolveRequest :
        ostr << "ResolveRequest";
        break;
    case ResolveResponse :
        ostr << "ResolveResponse";
        break;
    case DownloadRequest :
        ostr << "DownloadRequest";
        break;
    case DownloadError :
        ostr << "DownloadError";
        break;
    case Download :
        ostr << "Download";
        break;
    case DownloadFirstFragment :
        ostr << "DownloadFirstFragment";
        break;
    case DownloadFragment :
        ostr << "DownloadFragment";
        break;
    case DownloadLastFragment :
        ostr << "DownloadLastFragment";
        break;
    case UnexpectedSubcommand :
        ostr << "UnexpectedSubcommand";
        break;
    default :
        ostr << "missing case";
        break;
    }
    return ostr;
}

namespace Client
{

/*------------------------------------------------------------*
 * definition VsnetDownload::Client::State
 *------------------------------------------------------------*/

std::ostream& operator<<( std::ostream& ostr, State s )
{
    switch( s )
    {
    case Idle :
        ostr << "Idle";
        break;
    case Queued :
        ostr << "Queued";
        break;
    case Resolving :
        ostr << "Resolving";
        break;
    case Resolved :
        ostr << "Resolved";
        break;
    case Requested :
        ostr << "Requested";
        break;
    case FragmentReceived :
        ostr << "FragmentReceived";
        break;
    case Completed :
        ostr << "Completed";
        break;
    default :
        ostr << "unknown";
        break;
    };
    return ostr;
}
/*------------------------------------------------------------*
 * definition VsnetDownload::Client::Error
 *------------------------------------------------------------*/

std::ostream& operator<<( std::ostream& ostr, VSError e )
{
    switch( e )
    {
    case Ok :
        ostr << "Ok";
        break;
    case SocketError :
        ostr << "SocketError";
        break;
    case FileNotFound :
        ostr << "FileNotFound";
        break;
    case LocalPermissionDenied :
        ostr << "LocalPermissionDenied";
        break;
    case RemotePermissionDenied :
        ostr << "RemotePermissionDenied";
        break;
    case DownloadInterrupted :
        ostr << "DownloadInterrupted";
        break;
    default :
        ostr << "unknown";
        break;
    };
    return ostr;
}

}; // namespace Client

}; // namespace VsnetDownload

