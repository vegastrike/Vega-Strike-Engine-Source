/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef VSNET_PARSECMD_H
#define VSNET_PARSECMD_H

#include "netbuffer.h"
#include "vsnet_dloadenum.h"

namespace VsnetDownload
{

namespace Adapter
{
    struct Base;
    struct ResolveRequest;
    struct ResolveResponse;
    struct DownloadRequest;
    struct DownloadError;
    struct Download;
    struct DownloadFirstFragment;
    struct DownloadFragment;
    struct DownloadLastFragment;
    struct UnexpectedSubcommand;
};

class RecvCmdDownload
{
public:
    RecvCmdDownload( NetBuffer& buffer );
    ~RecvCmdDownload( );

    Subcommand parse( );

    inline Adapter::ResolveRequest* asResolveRequest( ) const {
        return (Adapter::ResolveRequest*)_base;
    }
    inline Adapter::ResolveResponse* asResolveResponse( ) const {
        return (Adapter::ResolveResponse*)_base;
    }
    inline Adapter::DownloadRequest* asDownloadRequest( ) const {
        return (Adapter::DownloadRequest*)_base;
    }
    inline Adapter::DownloadError* asDownloadError( ) const {
        return (Adapter::DownloadError*)_base;
    }
    inline Adapter::Download* asDownload( ) const {
        return (Adapter::Download*)_base;
    }
    inline Adapter::DownloadFirstFragment* asDownloadFirstFragment( ) const {
        return (Adapter::DownloadFirstFragment*)_base;
    }
    inline Adapter::DownloadFragment* asDownloadFragment( ) const {
        return (Adapter::DownloadFragment*)_base;
    }
    inline Adapter::DownloadLastFragment* asDownloadLastFragment( ) const {
        return (Adapter::DownloadLastFragment*)_base;
    }
    inline Adapter::UnexpectedSubcommand* asUnexpectedSubcommand( ) const {
        return (Adapter::UnexpectedSubcommand*)_base;
    }

private:
    NetBuffer&     _buf;
    Adapter::Base* _base;

    RecvCmdDownload( );
    RecvCmdDownload( const RecvCmdDownload& );
    RecvCmdDownload& operator=( const RecvCmdDownload& );
};

namespace Adapter
{
    struct Base
    {
        VsnetDownload::Subcommand c;
    };

    struct ResolveRequest : public Base
    {
        struct entry {
            char   ft;
            string file;
        };

        typedef vector<entry>::iterator iterator;

        short         num;
        vector<entry> files;
    };

    struct ResolveResponse : public Base
    {
        struct entry {
            string file;
            char   ok;
        };

        typedef vector<entry>::iterator iterator;

        short         num;
        vector<entry> files;
    };

    struct DownloadRequest : public Base
    {
        struct entry {
            char   ft;
            string file;
        };

        typedef vector<entry>::iterator iterator;

        short         num;
        vector<entry> files;
    };

    struct DownloadError : public Base
    {
        string file;
    };

    struct Download : public Base
    {
        string          file;
        short           payload_len;
        unsigned char*  payload;
    };

    struct DownloadFirstFragment : public Base
    {
        string          file;
        int             file_len;
        short           payload_len;
        unsigned char*  payload;
    };

    struct DownloadFragment : public Base
    {
        short           payload_len;
        unsigned char*  payload;
    };

    struct DownloadLastFragment : public Base
    {
        short           payload_len;
        unsigned char*  payload;
    };

    struct UnexpectedSubcommand : public Base
    {
    };

}; // Adapter
}; // VsnetDownload

#endif

