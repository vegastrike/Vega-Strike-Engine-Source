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

#include "vsnet_parsecmd.h"

namespace VsnetDownload
{

RecvCmdDownload::RecvCmdDownload( NetBuffer& buffer )
    : _buf( buffer )
    , _base( NULL )
{
}

RecvCmdDownload::~RecvCmdDownload( )
{
    if( _base ) delete _base;
}

VsnetDownload::Subcommand RecvCmdDownload::parse( )
{
    char c = _buf.getChar( );
    switch( c )
    {
    case VsnetDownload::ResolveRequest :
        {
            Adapter::ResolveRequest* r = new Adapter::ResolveRequest;
            r->c   = VsnetDownload::ResolveRequest;
            r->num = _buf.getShort( );
            for( int i=0; i<r->num; i++ )
            {
                Adapter::ResolveRequest::entry e;
                e.ft   = _buf.getChar( );
                e.file = _buf.getString( );
                r->files.push_back( e );
            }
            _base = r;
        }
        break;
    case VsnetDownload::ResolveResponse :
        {
            Adapter::ResolveResponse* r = new Adapter::ResolveResponse;
            r->c   = VsnetDownload::ResolveResponse;
            r->num = _buf.getShort( );
            for( int i=0; i<r->num; i++ )
            {
                Adapter::ResolveResponse::entry e;
                e.file = _buf.getString( );
                e.ok   = _buf.getChar( );
                r->files.push_back( e );
            }
            _base = r;
        }
        break;
    case VsnetDownload::DownloadRequest :
        {
            Adapter::DownloadRequest* r = new Adapter::DownloadRequest;
            r->c   = VsnetDownload::DownloadRequest;
            r->num = _buf.getShort( );
            for( int i=0; i<r->num; i++ )
            {
                Adapter::DownloadRequest::entry e;
                e.ft   = _buf.getChar( );
                e.file = _buf.getString( );
                r->files.push_back( e );
            }
            _base = r;
        }
        break;
    case VsnetDownload::DownloadError :
        {
            Adapter::DownloadError* r = new Adapter::DownloadError;
            r->c    = VsnetDownload::DownloadError;
            r->file = _buf.getString( );
            _base = r;
        }
        break;
    case VsnetDownload::Download :
        {
            Adapter::Download* r = new Adapter::Download;
            r->c           = VsnetDownload::Download;
            r->file        = _buf.getString( );
            r->payload_len = _buf.getShort( );
            r->payload     = _buf.getBuffer( r->payload_len );
            _base = r;
        }
        break;
    case VsnetDownload::DownloadFirstFragment :
        {
            Adapter::DownloadFirstFragment* r = new Adapter::DownloadFirstFragment;
            r->c           = VsnetDownload::DownloadFirstFragment;
            r->file        = _buf.getString( );
            r->file_len    = _buf.getInt32( );
            r->payload_len = _buf.getShort( );
            r->payload     = _buf.getBuffer( r->payload_len );
            _base = r;
        }
        break;
    case VsnetDownload::DownloadFragment :
        {
            Adapter::DownloadFragment* r = new Adapter::DownloadFragment;
            r->c           = VsnetDownload::DownloadFragment;
            r->payload_len = _buf.getShort( );
            r->payload     = _buf.getBuffer( r->payload_len );
            _base = r;
        }
        break;
    case VsnetDownload::DownloadLastFragment :
        {
            Adapter::DownloadLastFragment* r = new Adapter::DownloadLastFragment;
            r->c           = VsnetDownload::DownloadLastFragment;
            r->payload_len = _buf.getShort( );
            r->payload     = _buf.getBuffer( r->payload_len );
            _base = r;
        }
        break;
    default :
        {
            Adapter::UnexpectedSubcommand* r = new Adapter::UnexpectedSubcommand;
            r->c = VsnetDownload::UnexpectedSubcommand;
            _base = r;
        }
        break;
    }
    return _base->c;
}

}; // VsnetDownload

