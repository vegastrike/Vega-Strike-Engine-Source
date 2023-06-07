/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_FFMPEG_INIT_H
#define VEGA_STRIKE_ENGINE_FFMPEG_INIT_H

//
//C++ Interface: FFMpeg initialization
//

namespace FFMpeg {
void initLibraries();
};

#ifdef HAVE_FFMPEG

extern "C" {
#ifdef _WIN32
#define HAVE_FFMPEG_SWSCALE_H
#endif //_WIN32

    #ifdef HAVE_FFMPEG_SWSCALE_H //Not sure how many people have swscale.
        #include <ffmpeg/swscale.h>
    #else //HAVE_FFMPEG_SWSCALE_H
        #ifdef HAVE_LIBSWSCALE_SWSCALE_H
            #include <libswscale/swscale.h>
        #else //HAVE_LIBSWSCALE_SWSCALE_H
            #ifdef HAVE_SWSCALE_H
                #include <swscale.h>
            #else //HAVE_SWSCALE_H
                #define DEPRECATED_IMG_CONVERT 1
            #endif //HAVE_SWSCALE_H
        #endif //HAVE_LIBSWSCALE_SWSCALE_H
    #endif //HAVE_FFMPEG_SWSCALE_H

    #ifdef HAVE_LIBAVCODEC_AVCODEC_H
        #include <libavcodec/avcodec.h>
    #else //HAVE_LIBAVCODEC_AVCODEC_H
        #ifdef HAVE_AVCODEC_H
            #include <avcodec.h>
        #else //HAVE_AVCODEC_H
            #include <ffmpeg/avcodec.h>
        #endif //HAVE_AVCODEC_H
    #endif //HAVE_LIBAVCODEC_AVCODEC_H

    #ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
        #include <libavformat/avformat.h>
    #else //HAVE_LIBAVFORMAT_AVFORMAT_H
        #ifdef HAVE_AVFORMAT_H
            #include <avformat.h>
        #else //HAVE_AVFORMAT_H
            #include <ffmpeg/avformat.h>
        #endif //HAVE_AVFORMAT_H
    #endif //HAVE_LIBAVFORMAT_AVFORMAT_H

    #ifdef HAVE_LIBAVFORMAT_AVIO_H
        #include <libavformat/avio.h>
    #else //HAVE_LIBAVFORMAT_AVIO_H
        #ifdef HAVE_AVIO_H
            #include <avio.h>
        #else //HAVE_AVIO_H
            #include <ffmpeg/avio.h>
        #endif //HAVE_AVIO_H
    #endif //HAVE_LIBAVFORMAT_AVIO_H
}


#if (LIBAVCODEC_VERSION_MAJOR > 52)
#define CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO
#define CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
#else //(LIBAVCODEC_VERSION_MAJOR > 52)
#ifndef avformat_open_input
#define avformat_open_input(ctx, path, fmt, ap) av_open_input_file(ctx, path, fmt, 0, ap)
#endif //avformat_open_input
#endif //(LIBAVCODEC_VERSION_MAJOR > 52)


#endif //HAVE_FFMPEG

#endif //VEGA_STRIKE_ENGINE_FFMPEG_INIT_H
