/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

//
//C++ Interface: FFMpeg initialization
//
#ifndef __FFMPEG_INIT_H__INCLUDED__
#define __FFMPEG_INIT_H__INCLUDED__

namespace FFMpeg {
void initLibraries();
};

#ifdef HAVE_FFMPEG

extern "C" {
#ifdef _WIN32
#define HAVE_FFMPEG_SWSCALE_H
#endif
    #ifdef HAVE_FFMPEG_SWSCALE_H //Not sure how many people have swscale.
        #include <ffmpeg/swscale.h>
    #else
        #ifdef HAVE_LIBSWSCALE_SWSCALE_H
            #include <libswscale/swscale.h>
        #else
            #ifdef HAVE_SWSCALE_H
                #include <swscale.h>
            #else
                #define DEPRECATED_IMG_CONVERT 1
            #endif
        #endif
    #endif
    #ifdef HAVE_LIBAVCODEC_AVCODEC_H
        #include <libavcodec/avcodec.h>
    #else
        #ifdef HAVE_AVCODEC_H
            #include <avcodec.h>
        #else
            #include <ffmpeg/avcodec.h>
        #endif
    #endif
    #ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
        #include <libavformat/avformat.h>
    #else
        #ifdef HAVE_AVFORMAT_H
            #include <avformat.h>
        #else
            #include <ffmpeg/avformat.h>
        #endif
    #endif
    #ifdef HAVE_LIBAVFORMAT_AVIO_H
        #include <libavformat/avio.h>
    #else
        #ifdef HAVE_AVIO_H
            #include <avio.h>
        #else
            #include <ffmpeg/avio.h>
        #endif
    #endif
}


#if (LIBAVCODEC_VERSION_MAJOR > 52)
#define CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO
#define CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
#else
#ifndef avformat_open_input
#define avformat_open_input(ctx, path, fmt, ap) av_open_input_file(ctx, path, fmt, 0, ap)
#endif
#endif


#endif

#endif //__FFMPEG_INIT_H__INCLUDED__

