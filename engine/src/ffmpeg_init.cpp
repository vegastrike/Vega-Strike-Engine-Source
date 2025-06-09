/*
 * ffmpeg_init.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

//
//C++ Implementation: vid_file
//

#include "root_generic/vsfilesystem.h"
#include "ffmpeg_init.h"

#include <string.h>
#include <utility>

//define a 128k buffer for video streamers
#define BUFFER_SIZE 128*(1<<10)

#ifndef ENOENT
#define ENOENT 2
#endif

/*
 * FOLLOWING CODE IS ONLY INCLUDED IF YOU HAVE FFMPEG
 * ********************************************
 */
#ifdef HAVE_FFMPEG
//#ifdef _WIN32
//#define offset_t xoffset_t
//#endif

#ifndef offset_t
    #if (LIBAVCODEC_VERSION_MAJOR >= 52) || (LIBAVCODEC_VERSION_INT >= ( ( 51<<16)+(49<<8)+0 ) ) || defined (__amd64__) \
    || defined (_M_AMD64) || defined (__x86_64) || defined (__x86_64__)
typedef int64_t offset_t;
    #else
typedef int     offset_t;
    #endif
#endif

using namespace VSFileSystem;

extern "C" int _url_open( URLContext *h, const char *filename, int flags )
{
    if (strncmp( filename, "vsfile:", 7 ) != 0)
        return AVERROR( ENOENT );

    const char *type   = strchr( filename+7, '|' );
    std::string path( filename+7, type ? type-filename-7 : strlen( filename+7 ) );
    VSFileType  vstype = ( (type && *type) ? (VSFileType) atoi( type+1 ) : VideoFile);

    VSFile     *f = new VSFile();
    if (f->OpenReadOnly( path, vstype ) > Ok) {
        delete f;
        return AVERROR( ENOENT );
    } else {
        h->priv_data = f;
        return 0;
    }
}

extern "C" int _url_close( URLContext *h )
{
    delete (VSFile*) (h->priv_data);
    return 0;
}

extern "C" int _url_read( URLContext *h, unsigned char *buf, int size )
{
    return ( (VSFile*) (h->priv_data) )->Read( buf, size );
}

// Changed on June 1, 2010. During minor version 67.
// See http://git.ffmpeg.org/?p=ffmpeg;a=commitdiff;h=2967315b9ee7afa15d2849b473e359f50a815696
extern "C" int _url_write( URLContext *h,
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(52, 67, 0)
                const
#endif
                    unsigned char *buf, int size )
{
    //read-only please
    return 0;
}

extern "C" offset_t _url_seek( URLContext *h, offset_t pos, int whence )
{
    if (whence != AVSEEK_SIZE) {
        ( (VSFile*) (h->priv_data) )->GoTo( long(pos) );
        return ( (VSFile*) (h->priv_data) )->GetPosition();
    } else {
        return ( (VSFile*) (h->priv_data) )->Size();
    }
}

struct URLProtocol vsFileProtocol = {
    "vsfile",
    _url_open,
    _url_read,
    _url_write,
    _url_seek,
    _url_close,

#if (LIBAVCODEC_VERSION_MAJOR >= 53)
    NULL, NULL, NULL, NULL,
    0,
    NULL,
    0,
    NULL
#endif
};

namespace FFMpeg
{
void initLibraries()
{
    static bool initted = false;
    if (!initted) {
        initted = true;
        av_register_all();
#if (LIBAVFORMAT_VERSION_MAJOR >= 53)
        av_register_protocol2( &vsFileProtocol, sizeof(vsFileProtocol) );
#else
        register_protocol( &vsFileProtocol );
#endif
    }
}
};

//Workaround for a missing export in libavcodec 52.47.0
#if (LIBAVCODEC_VERSION_MAJOR == 52 && LIBAVCODEC_VERSION_MINOR == 47 && LIBAVCODEC_VERSION_MICRO == 0)
extern "C" {
void av_free_packet( AVPacket *pkt )
{
    if (pkt) {
        if (pkt->destruct) pkt->destruct( pkt );
        pkt->data = NULL;
        pkt->size = 0;
    }
}
}
#endif

#else //No FFMPEG

namespace FFMpeg {
void initLibraries() {
    //No-op stub
}
};

#endif

