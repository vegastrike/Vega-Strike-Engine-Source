/*
 * vid_file.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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

#include "vid_file.h"
#include "vsfilesystem.h"
#include "vs_logging.h"
#include "ffmpeg_init.h"

#include <string.h>
#include <math.h>
#include <utility>

//define a 128k buffer for video streamers
#define BUFFER_SIZE ( 128*(1<<10) )

#ifndef ENOENT
#define ENOENT (2)
#endif
#include <sys/types.h>

/*
 * FOLLOWING CODE IS ONLY INCLUDED IF YOU HAVE FFMPEG
 * ********************************************
 */
#ifdef HAVE_FFMPEG
#ifndef offset_t
#if (LIBAVCODEC_VERSION_MAJOR >= 52) || (LIBAVCODEC_VERSION_INT >= ( ( 51<<16)+(49<<8)+0 ) ) || defined (__amd64__) \
    || defined (_M_AMD64) || defined (__x86_64) || defined (__x86_64__)
typedef int64_t offset_t;
#else
typedef int     offset_t;
#endif
#endif
using namespace VSFileSystem;

class VidFileImpl
{
private:
    AVFormatContext *pFormatCtx;
    AVCodecContext  *pCodecCtx;
    AVCodec  *pCodec;
    AVFrame  *pFrameRGB;
    AVFrame  *pFrameYUV;
    AVFrame  *pNextFrameYUV;
    AVStream *pStream;
    int         videoStreamIndex;
    bool        frameReady;

    uint8_t    *packetBuffer;
    size_t      packetBufferSize;
    AVPacket    packet;

/** Framebuffer dimensions limit, useful for bandwidth-limited GPUs */
    size_t      fbDimensionLimit;
    bool        fbForcePOT;

#ifndef DEPRECATED_IMG_CONVERT
    SwsContext *pSWSCtx;
#endif

    uint64_t    fbPTS;
    uint64_t    sizePTS;
    uint64_t    prevPTS;

    void convertFrame()
    {
        if (frameReady) {
#ifdef DEPRECATED_IMG_CONVERT
            img_convert(
                (AVPicture*) pFrameRGB, PIX_FMT_RGB24,
                (AVPicture*) pNextFrameYUV, pCodecCtx->pix_fmt,
                pCodecCtx->width, pCodecCtx->height );
#else
            sws_scale( pSWSCtx, pNextFrameYUV->data, pNextFrameYUV->linesize, 0,
                       pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize );
#endif
            prevPTS = fbPTS;
            fbPTS = pNextFrameYUV->pts;

            if (prevPTS > fbPTS)
                prevPTS = fbPTS;

            std::swap( pNextFrameYUV, pFrameYUV );
        }
    }

    void nextFrame(bool skip=false)
    {
        int bytesDecoded;
        int frameFinished;
        //Decode packets until we have decoded a complete frame
        while (true) {
            if (!skip) {
                //Work on the current packet until we have decoded all of it
                while (packetBufferSize > 0 && packet.size > 0) {
                    //Decode the next chunk of data
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
                    bytesDecoded = avcodec_decode_video2(
                        pCodecCtx, pNextFrameYUV, &frameFinished,
                        &packet );
#else
                    bytesDecoded = avcodec_decode_video(
                        pCodecCtx, pNextFrameYUV, &frameFinished,
                        packetBuffer, packetBufferSize );
#endif
                    VS_LOG(trace, (boost::format("dts %1%: Decoded %2% bytes %3%") % int64_t(packet.dts) % int(bytesDecoded) %
                                                    (frameFinished ? "Got frame" : "")));
                    //Was there an error?
                    if (bytesDecoded <= 0) throw VidFile::FrameDecodeException( "Error decoding frame" );
                    //Crappy ffmpeg!
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
                    if (bytesDecoded > packet.size)
                        bytesDecoded = packet.size;
                    packet.size -= bytesDecoded;
                    packet.data += bytesDecoded;
#else
                    if (bytesDecoded > packetBufferSize)
                        bytesDecoded = packetBufferSize;
                    packetBufferSize -= bytesDecoded;
                    packetBuffer     += bytesDecoded;
#endif
                    //Did we finish the current frame? Then we can return
                    if (frameFinished) {
                        pNextFrameYUV->pts = packet.dts;
                        frameReady = true;
                        return;
                    }
                }
            }
            //Read the next packet, skipping all packets that aren't for this
            //stream
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
            packet.size = packetBufferSize;
            packet.data = packetBuffer;
#endif
            do {
                //Free old packet
                if (packet.data != NULL)
                    av_free_packet( &packet );
                //Read new packet
                if (av_read_frame( pFormatCtx, &packet ) < 0)
                    throw VidFile::EndOfStreamException();
            } while (packet.stream_index != videoStreamIndex);
            packetBufferSize = packet.size;
            packetBuffer     = packet.data;

            if (skip)
                break;
        }
    }

public:
    float    frameRate;
    float    duration;
    uint8_t *_frameBuffer;
    uint8_t *frameBuffer;
    offset_t frameBufferStride;
    size_t   frameBufferSize;
    size_t   width;
    size_t   height;

    VidFileImpl( size_t maxDimensions, bool forcePOT ) :
        pFormatCtx( 0 )
        , pCodecCtx( 0 )
        , pCodec( 0 )
        , pFrameRGB( 0 )
        , pFrameYUV( 0 )
        , pNextFrameYUV( 0 )
        , pStream( 0 )
        , frameReady( false )
        , packetBuffer( 0 )
        , packetBufferSize( 0 )
        , fbDimensionLimit( maxDimensions )
        , fbForcePOT( forcePOT )
        , frameBuffer( 0 )
    {
        packet.data = 0;
    }

    ~VidFileImpl()
    {
        //Free framebuffer
        if (frameBuffer != nullptr) {
            delete[] _frameBuffer;
            _frameBuffer = nullptr;

        }
        if (pFrameRGB != nullptr) {
            av_free( pFrameRGB );
            pFrameRGB = nullptr;
        }
        if (pFrameYUV != nullptr) {
            av_free( pFrameYUV );
            pFrameYUV = nullptr;
        }
        if (pNextFrameYUV != nullptr) {
            av_free( pNextFrameYUV );
            pNextFrameYUV = nullptr;
        }
        //Close the codec
        if (pCodecCtx != nullptr) {
            avcodec_close( pCodecCtx );
            // delete pCodecCtx;
            pCodecCtx = nullptr;
        }
        //Close the file
        if (pFormatCtx != nullptr) {
            av_close_input_file( pFormatCtx );
            // delete pFormatCtx;
            pFormatCtx = nullptr;
        }
    }

    void open( const std::string &path )
    {
        if (pCodecCtx != 0) throw VidFile::Exception( "Already open" );
        //Initialize libavcodec/libavformat if necessary
        FFMpeg::initLibraries();

        //Open file
        std::string npath   = std::string( "vsfile:" )+path;
        std::string errbase = std::string( "Cannot open URL \"" )+npath+"\"";
        if ( ( 0 != avformat_open_input( &pFormatCtx, npath.c_str(), NULL, NULL ) )
            || ( 0 > av_find_stream_info( pFormatCtx ) ) ) throw VidFile::FileOpenException( errbase+" (wrong format or)" );
        //Dump format info in case we want to know...
#ifdef VS_DEBUG
        dump_format( pFormatCtx, 0, npath.c_str(), false );
#endif

        //Find first video stream
        pCodecCtx = 0;
        videoStreamIndex = -1;
        VS_LOG(debug, (boost::format("Loaded %1%") % path));
        for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
            VS_LOG(trace, (boost::format("  Stream %1%: type %2% (%3%) first dts %4%") % i %
                                            ((pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
                                                 ? "Video"
                                                 : ((pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) ? "Audio" : "unk")) %
                                            pFormatCtx->streams[i]->codec->codec_type % int64_t(pFormatCtx->streams[i]->start_time)));
            if ((pCodecCtx == 0) && (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)) {
                pCodecCtx = (pStream = pFormatCtx->streams[videoStreamIndex = i])->codec;
            }
        }
        if (pCodecCtx == 0) throw VidFile::FileOpenException( errbase+" (no video stream)" );
        VS_LOG(trace, (boost::format("  Codec Timebase: %1%/%2%") % pCodecCtx->time_base.num % pCodecCtx->time_base.den));

        //Find codec for video stream and open it
        pCodec        = avcodec_find_decoder( pCodecCtx->codec_id );
        if (pCodec == 0) throw VidFile::UnsupportedCodecException( errbase+" (unsupported codec)" );
        if (avcodec_open( pCodecCtx, pCodec ) < 0) throw VidFile::UnsupportedCodecException( errbase+" (unsupported codec)" );
        pFrameYUV     = avcodec_alloc_frame();
        pNextFrameYUV = avcodec_alloc_frame();
        if ( (pFrameYUV == 0) || (pNextFrameYUV == 0) ) throw VidFile::Exception(
                "Problem during YUV framebuffer initialization" );
        //Get some info
        frameRate = float(pStream->r_frame_rate.num)/float(pStream->r_frame_rate.den);
        duration  = float(pStream->duration*pStream->time_base.num)/float(pStream->time_base.den);
        VS_LOG(trace, (boost::format("  Framerate: %1%/%2%") % pStream->r_frame_rate.num % pStream->r_frame_rate.den));
        VS_LOG(trace, (boost::format("  Stream timebase: %1%/%2%") % pStream->time_base.num % pStream->time_base.den));

        //Get POT dimensions
        if (fbForcePOT) {
            width = height = 1;
            while (width < pCodecCtx->width && width <= (fbDimensionLimit/2)) width *= 2;
            while (height < pCodecCtx->height && height <= (fbDimensionLimit/2)) height *= 2;
        } else {
            width = pCodecCtx->width;
            height = pCodecCtx->height;
            while ( (width > fbDimensionLimit) || (height > fbDimensionLimit) ) {
                width  /= 2;
                height /= 2;
            }
        }
        VS_LOG(debug, (boost::format("  playing at %1%x%2%") % width % height));

        //Allocate RGB frame buffer
        pFrameRGB         = avcodec_alloc_frame();
        if (pFrameRGB == 0) throw VidFile::Exception( "Problem during RGB framebuffer initialization" );
        frameBufferSize   = avpicture_get_size( PIX_FMT_RGB24, width, height );
        _frameBuffer      = new uint8_t[frameBufferSize];
        if (_frameBuffer == 0) throw VidFile::Exception( "Problem during RGB framebuffer initialization" );
        avpicture_fill( (AVPicture*) pFrameRGB, _frameBuffer, PIX_FMT_RGB24, width, height );
        frameBuffer       = pFrameRGB->data[0];
        frameBufferSize   = pFrameRGB->linesize[0]*height;
        frameBufferStride = pFrameRGB->linesize[0];

        //Initialize timebase counters
        prevPTS =
        fbPTS =
        pFrameYUV->pts =
        pNextFrameYUV->pts = 0;

#ifndef DEPRECATED_IMG_CONVERT
        pSWSCtx = sws_getContext( pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                  width, height, PIX_FMT_RGB24, SWS_LANCZOS|SWS_PRINT_INFO, NULL, NULL, NULL );
#endif
    }

    bool seek( float time )
    {
        if (time < 0)
            time = 0;

        //Translate float time to frametime
        uint64_t targetPTS = uint64_t( floor( double(time)*pStream->time_base.den/pStream->time_base.num ) );
        VS_LOG(trace, (boost::format("Seeking to %1$.3fs pts %2%") % time % targetPTS));
        if ( (targetPTS >= prevPTS) && (targetPTS < pNextFrameYUV->pts) ) {
            //same frame
            if (targetPTS >= fbPTS) {
                try {
                    prevPTS = fbPTS;
                    convertFrame();
                    nextFrame();
                    return true;
                }
                catch (const VidFile::EndOfStreamException& e) {
                    sizePTS = fbPTS+1; throw e;
                }
            }
            return false;
        } else {
            if (targetPTS < fbPTS) {
                //frame backwards
                int64_t backPTS = targetPTS - 1 - pStream->time_base.den/pStream->time_base.num/2;
                if (backPTS < 0)
                    backPTS = 0;

                VS_LOG(trace, (boost::format("backseeking to %1% (at %2%)") % backPTS % int64_t(pNextFrameYUV->pts)));
                av_seek_frame( pFormatCtx, videoStreamIndex, backPTS, AVSEEK_FLAG_BACKWARD );

                prevPTS = backPTS;
                nextFrame();
            }
            //frame forward
            try {
                // Try one frame, decoding
                if (pNextFrameYUV->pts < targetPTS) {
                    prevPTS = pNextFrameYUV->pts;
                    nextFrame();
                    VS_LOG(trace, (boost::format("decoding to %1% (at %2%-%3%)")
                                % targetPTS
                                % prevPTS
                                % int64_t(pNextFrameYUV->pts)));
                }
                // If we have to skip more frames, don't decode, only skip data
                while (packet.dts < targetPTS) {
                    prevPTS = packet.dts;
                    nextFrame(true);
                    VS_LOG(trace, (boost::format("skipping to %1% (at %2%-%3%)")
                                % targetPTS
                                % prevPTS
                                % int64_t(packet.dts)));
                }
                // we're close, decode now
                while (pNextFrameYUV->pts < targetPTS) {
                    prevPTS = pNextFrameYUV->pts;
                    nextFrame();
                    VS_LOG(trace, (boost::format("decoding to %1% (at %2%-%3%)")
                                % targetPTS
                                % prevPTS
                                % int64_t(pNextFrameYUV->pts)));
                }
                convertFrame();
                nextFrame();
            }
            catch (const VidFile::EndOfStreamException& e) {
                sizePTS = fbPTS+1; throw e;
            }

            return true;
        }
    }
};

#else /* !HAVE_FFMPEG */
class VidFileImpl {
private:
    VidFileImpl(size_t, bool) {
    }

public:
    //Avoid having to put ifdef's everywhere.
    float frameRate, duration;
    int width, height;
    void *frameBuffer;
    int frameBufferStride;

    bool seek(float time) {
        return false;
    }
};

#endif /* !HAVE_FFMPEG */

/* ************************************ */

VidFile::VidFile() :
        impl(nullptr) {
}

VidFile::~VidFile() {
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
}

bool VidFile::isOpen() const {
    return impl != nullptr;
}

void VidFile::open(const std::string &path, size_t maxDimension, bool forcePOT) {
#ifdef HAVE_FFMPEG
    if (!impl)
        impl = new VidFileImpl( maxDimension, forcePOT );
    if (impl)
        impl->open( path );
#endif
}

void VidFile::close() {
    if (impl != nullptr) {
        delete impl;
        impl = nullptr;
    }
}

float VidFile::getFrameRate() const {
    return impl ? impl->frameRate : 0;
}

float VidFile::getDuration() const {
    return impl ? impl->duration : 0;
}

int VidFile::getWidth() const {
    return impl ? impl->width : 0;
}

int VidFile::getHeight() const {
    return impl ? impl->height : 0;
}

void *VidFile::getFrameBuffer() const {
    return impl ? impl->frameBuffer : 0;
}

int VidFile::getFrameBufferStride() const {
    return impl ? impl->frameBufferStride : 0;
}

bool VidFile::seek(float time) {
    return (impl != 0) && impl->seek(time);
}
