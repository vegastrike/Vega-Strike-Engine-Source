#ifndef VSNET_DLOADENUM_H
#define VSNET_DLOADENUM_H

#include <config.h>

#include <iostream>

namespace VsnetDownload
{
/*------------------------------------------------------------*
 * declaration VsnetDownload::Subcommand
 *------------------------------------------------------------*/

enum Subcommand
{
    ResolveRequest,
    ResolveResponse,
    DownloadRequest,
    DownloadError,
    Download,
    DownloadFirstFragment,
    DownloadFragment,
    DownloadLastFragment,
    UnexpectedSubcommand
};

std::ostream& operator<<( std::ostream& ostr, Subcommand e );

namespace Client
{

class Manager;

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::State
 *------------------------------------------------------------*/

enum State
{
    Idle,
    Queued,
    Resolving,
    Resolved,
    Requested,
    FragmentReceived,
    Completed
};

std::ostream& operator<<( std::ostream& ostr, State s );

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Error
 *------------------------------------------------------------*/

enum Error
{
    Ok,
    SocketError,
    FileNotFound,
    LocalPermissionDenied,
    RemotePermissionDenied,
    DownloadInterrupted
};

std::ostream& operator<<( std::ostream& ostr, Error e );

}; // namespace Client

}; // namespace VsnetDownload

#endif /* VSNET_DLOADENUM_H */

