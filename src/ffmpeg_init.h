//
// C++ Interface: FFMpeg initialization
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
    #ifdef HAVE_FFMPEG_SWSCALE_H // Not sure how many people have swscale.
        #include <ffmpeg/swscale.h>
    #else
        #ifdef HAVE_LIBSWSCALE_SWSCALE_H
            #include <libswscale/swscale.h>
        #else
            #define DEPRECATED_IMG_CONVERT 1
        #endif
    #endif
    #ifdef HAVE_LIBAVCODEC_AVCODEC_H
        #include <libavcodec/avcodec.h>
    #else
        #include <ffmpeg/avcodec.h>
    #endif
    #ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
        #include <libavformat/avformat.h>
    #else
        #include <ffmpeg/avformat.h>
    #endif
    #ifdef HAVE_LIBAVFORMAT_AVIO_H
        #include <libavformat/avio.h>
    #else
        #include <ffmpeg/avio.h>
    #endif
}

#endif

#endif//__FFMPEG_INIT_H__INCLUDED__
