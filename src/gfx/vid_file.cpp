//
//C++ Implementation: vid_file
//

#include "vid_file.h"
#include "vsfilesystem.h"
#include "config.h"
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

    void nextFrame(bool skip=false) throw (VidFile::Exception)
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
                    VSFileSystem::vs_dprintf(3, "dts %lld: Decoded %d bytes %s\n", 
                        int64_t(packet.dts),
                        int(bytesDecoded),
                        (frameFinished ? "Got frame" : "")
                    );
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
        if (frameBuffer)
            delete[] _frameBuffer;
        if (pFrameRGB)
            av_free( pFrameRGB );
        if (pFrameYUV)
            av_free( pFrameYUV );
        if (pNextFrameYUV)
            av_free( pNextFrameYUV );
#ifndef DEPRECATED_IMG_CONVERT
        if (pSWSCtx)
            sws_freeContext( pSWSCtx );
#endif
        //Close the codec
        if (pCodecCtx)
            avcodec_close( pCodecCtx );
        //Close the file
        if (pFormatCtx)
            av_close_input_file( pFormatCtx );
    }

    void open( const std::string &path ) throw (VidFile::Exception)
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
        VSFileSystem::vs_dprintf(2, "Loaded %s\n", path.c_str());
        for (unsigned int i = 0; i < pFormatCtx->nb_streams; ++i) {
            VSFileSystem::vs_dprintf(3, "  Stream %d: type %s (%d) first dts %lld\n", 
                i,
                ( (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) ? "Video"
                    : ( (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) ? "Audio" : "unk" ) ),
                pFormatCtx->streams[i]->codec->codec_type,
                int64_t(pFormatCtx->streams[i]->start_time)
            );
            if ((pCodecCtx == 0) && (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO))
                pCodecCtx = (pStream = pFormatCtx->streams[videoStreamIndex = i])->codec;
        }
        if (pCodecCtx == 0) throw VidFile::FileOpenException( errbase+" (no video stream)" );
        VSFileSystem::vs_dprintf(3, "  Codec Timebase: %d/%d\n", pCodecCtx->time_base.num, pCodecCtx->time_base.den);

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
        VSFileSystem::vs_dprintf(3, "  Framerate: %d/%d\n", pStream->r_frame_rate.num, pStream->r_frame_rate.den);
        VSFileSystem::vs_dprintf(3, "  Stream timebase: %d/%d\n", pStream->time_base.num, pStream->time_base.den);
        
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
        VSFileSystem::vs_dprintf(2, "  playing at %dx%d\n", width, height);
        
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
        VSFileSystem::vs_dprintf(3, "Seeking to %.3fs pts %lld\n", time, targetPTS);
        if ( (targetPTS >= prevPTS) && (targetPTS < pNextFrameYUV->pts) ) {
            //same frame
            if (targetPTS >= fbPTS) {
                try {
                    prevPTS = fbPTS;
                    convertFrame();
                    nextFrame();
                    return true;
                }
                catch (VidFile::EndOfStreamException e) {
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
                    
                VSFileSystem::vs_dprintf(3, "backseeking to %lld (at %lld)\n", backPTS, int64_t(pNextFrameYUV->pts));
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
                    VSFileSystem::vs_dprintf(3, "decoding to %lld (at %lld-%lld)\n", targetPTS, prevPTS, int64_t(pNextFrameYUV->pts));
                }
                // If we have to skip more frames, don't decode, only skip data
                while (packet.dts < targetPTS) {
                    prevPTS = packet.dts;
                    nextFrame(true);
                    VSFileSystem::vs_dprintf(3, "skipping to %lld (at %lld-%lld)\n", targetPTS, prevPTS, int64_t(packet.dts));
                }
                // we're close, decode now
                while (pNextFrameYUV->pts < targetPTS) {
                    prevPTS = pNextFrameYUV->pts;
                    nextFrame();
                    VSFileSystem::vs_dprintf(3, "decoding to %lld (at %lld-%lld)\n", targetPTS, prevPTS, int64_t(pNextFrameYUV->pts));
                }
                convertFrame();
                nextFrame();
            }
            catch (VidFile::EndOfStreamException e) {
                sizePTS = fbPTS+1; throw e;
            }

            return true;
        }
    }
};

#else /* !HAVE_FFMPEG */
class VidFileImpl
{
private:
    VidFileImpl(size_t, bool) {}
public:
    //Avoid having to put ifdef's everywhere.
    float frameRate, duration;
    int   width, height;
    void *frameBuffer;
    int   frameBufferStride;
    bool seek( float time )
    {
        return false;
    }
};

#endif /* !HAVE_FFMPEG */
/* ************************************ */

VidFile::VidFile() throw () :
    impl( NULL )
{}

VidFile::~VidFile()
{
    if (impl)
        delete impl;
}

bool VidFile::isOpen() const throw ()
{
    return impl != NULL;
}

void VidFile::open( const std::string &path, size_t maxDimension, bool forcePOT ) throw (Exception)
{
#ifdef HAVE_FFMPEG
    if (!impl)
        impl = new VidFileImpl( maxDimension, forcePOT );
    if (impl)
        impl->open( path );
#endif
}

void VidFile::close() throw ()
{
    if (impl) {
        delete impl;
        impl = 0;
    }
}

float VidFile::getFrameRate() const throw ()
{
    return impl ? impl->frameRate : 0;
}

float VidFile::getDuration() const throw ()
{
    return impl ? impl->duration : 0;
}

int VidFile::getWidth() const throw ()
{
    return impl ? impl->width : 0;
}

int VidFile::getHeight() const throw ()
{
    return impl ? impl->height : 0;
}

void* VidFile::getFrameBuffer() const throw ()
{
    return impl ? impl->frameBuffer : 0;
}

int VidFile::getFrameBufferStride() const throw ()
{
    return impl ? impl->frameBufferStride : 0;
}

bool VidFile::seek( float time ) throw (Exception)
{
    return (impl != 0) && impl->seek( time );
}

