//
// C++ Implementation: vid_file
//

#include "vid_file.h"
#include "vsfilesystem.h"
#include "config.h"

#include <string.h>
#include <math.h>
#include <utility>

#ifdef HAVE_FFMPEG

extern "C" {
    #ifdef HAVE_FFMPEG_SWSCALE_H // Not sure how many people have swscale.
        #include <ffmpeg/swscale.h>
    #else
        #define DEPRECATED_IMG_CONVERT 1
    #endif
    #include <ffmpeg/avcodec.h>
    #include <ffmpeg/avformat.h>
    #include <ffmpeg/avio.h>
}

#endif

// define a 128k buffer for video streamers
#define BUFFER_SIZE 128*(1<<10)

#ifndef ENOENT
#define ENOENT 2
#endif


/* FOLLOWING CODE IS ONLY INCLUDED IF YOU HAVE FFMPEG */
/* ******************************************** */
#ifdef HAVE_FFMPEG

using namespace VSFileSystem;

extern "C" int _url_open(URLContext *h, const char *filename, int flags)
{
    if (strncmp(filename,"vsfile:",7)!=0)
        return AVERROR_NOENT;
        
    std::string path = filename+7;
    
    VSFile *f = new VSFile();
    if (f->OpenReadOnly(path, VSFileSystem::VideoFile) > VSFileSystem::Ok) {
        delete f;
        return AVERROR_NOENT;
    } else {
        h->priv_data = f;
        return 0;
    }
}

extern "C" int _url_close(URLContext *h)
{
    delete (VSFile*)(h->priv_data);
    return 0;
}

extern "C" int _url_read(URLContext *h, unsigned char *buf, int size)
{
    return ((VSFile*)(h->priv_data))->Read(buf, size);
} 

extern "C" int _url_write(URLContext *h, unsigned char *buf, int size)
{
    // read-only please
    return 0;
} 

extern "C" offset_t _url_seek(URLContext *h, offset_t pos, int whence)
{
    if (whence != AVSEEK_SIZE) {
        ((VSFile*)(h->priv_data))->GoTo(long(pos));
        return ((VSFile*)(h->priv_data))->GetPosition();
    } else {
        return ((VSFile*)(h->priv_data))->Size();
    }
}

    
struct URLProtocol vsFileProtocol = {
    "vsfile",
    _url_open,
    _url_read,
    _url_write,
    _url_seek,
    _url_close,
};


class VideoFileImpl {
private:
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrameRGB;
    AVFrame *pFrameYUV;
    AVFrame *pNextFrameYUV;
    AVStream *pStream;
    int videoStreamIndex;
    bool frameReady;
    
    uint8_t *packetBuffer;
    size_t packetBufferSize;
    AVPacket packet;
    
    /** Framebuffer dimensions limit, useful for bandwidth-limited GPUs */
    size_t fbDimensionLimit;
	
#ifndef DEPRECATED_IMG_CONVERT
    SwsContext *pSWSCtx;
#endif
    
    uint64_t fbPTS;
    uint64_t sizePTS;

    void initLibraries()
    {
        static bool initted = false;
        if (!initted) {
            initted = true;
            av_register_all();
            register_protocol(&vsFileProtocol);
        }
    }
    
    void convertFrame()
    {
        if (frameReady) {
#ifdef DEPRECATED_IMG_CONVERT
            img_convert(
                (AVPicture*)pFrameRGB, PIX_FMT_RGB24, 
                (AVPicture*)pNextFrameYUV, pCodecCtx->pix_fmt, 
                pCodecCtx->width, pCodecCtx->height);
#else
            sws_scale(pSWSCtx, pNextFrameYUV->data, pNextFrameYUV->linesize, 0,
                      pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
#endif
            fbPTS = pNextFrameYUV->pts;
            
            std::swap(pNextFrameYUV, pFrameYUV);
        }
    }
    
    void nextFrame() throw(VideoFile::Exception)
    {
        int bytesDecoded;
        int frameFinished;
    
        // Decode packets until we have decoded a complete frame
        while(true) {
            // Work on the current packet until we have decoded all of it
            while(packetBufferSize > 0) {
                // Decode the next chunk of data
                bytesDecoded = avcodec_decode_video(
                    pCodecCtx, pNextFrameYUV, &frameFinished, 
                    packetBuffer, packetBufferSize);
    
                // Was there an error?
                if(bytesDecoded < 0)
                    throw VideoFile::FrameDecodeException("Error decoding frame");
    
                // Crappy ffmpeg!
                if (bytesDecoded > packetBufferSize)
                    bytesDecoded = packetBufferSize;
                    
                packetBufferSize -= bytesDecoded;
                packetBuffer += bytesDecoded;
    
                // Did we finish the current frame? Then we can return
                if (frameFinished) {
                    if (pNextFrameYUV->pts == 0)
                        pNextFrameYUV->pts = packet.pts;
                    frameReady = true;
                    return;
                }
            }
    
            // Read the next packet, skipping all packets that aren't for this
            // stream
            do {
                // Free old packet
                if(packet.data != NULL)
                    av_free_packet(&packet);
                
                // Read new packet
                if(av_read_frame(pFormatCtx, &packet) < 0)
                    throw VideoFile::EndOfStreamException();
            } while(packet.stream_index != videoStreamIndex);
    
            packetBufferSize = packet.size;
            packetBuffer = packet.data;
        }
    }

public:
    float frameRate;
    float duration;
    uint8_t *_frameBuffer;
    uint8_t *frameBuffer;
    offset_t frameBufferStride;
    size_t frameBufferSize;
    size_t width;
    size_t height;

    VideoFileImpl(size_t maxDimensions) :
        pFormatCtx(0),
        pCodecCtx(0),
        pCodec(0),
        pStream(0),
        pFrameRGB(0),
        pFrameYUV(0),
        pNextFrameYUV(0),
        frameBuffer(0),
        packetBuffer(0),
        packetBufferSize(0),
        frameReady(false),
        fbDimensionLimit(maxDimensions)
    {
        packet.data = 0;
    }
    
    ~VideoFileImpl()
    {
        // Free framebuffer
        if (frameBuffer)
            delete[] _frameBuffer;
        if (pFrameRGB)
            av_free(pFrameRGB);
        if (pFrameYUV)
            av_free(pFrameYUV);
        if (pNextFrameYUV)
            av_free(pNextFrameYUV);
        
#ifndef DEPRECATED_IMG_CONVERT
        if (pSWSCtx) {
            sws_freeContext(pSWSCtx);
        }
#endif
    
        // Close the codec
        if (pCodecCtx)
            avcodec_close(pCodecCtx);
        
        // Close the file
        if (pFormatCtx)
            av_close_input_file(pFormatCtx);
    }
    
    void open(const std::string& path) throw(VideoFile::Exception)
    {
        if (pCodecCtx != 0)
            throw VideoFile::Exception("Already open");
        
        // Initialize libavcodec/libavformat if necessary
        initLibraries();
        
        // Open file
        std::string npath = std::string(vsFileProtocol.name) + ":" + path;
        std::string errbase = std::string("Cannot open URL \"") + npath + "\"";
        
        if (  (0 != av_open_input_file(&pFormatCtx, npath.c_str(), NULL, BUFFER_SIZE, NULL))
            ||(0 >  av_find_stream_info(pFormatCtx))  )
            throw VideoFile::FileOpenException(errbase + " (wrong format or)"); 
        
        // Dump format info in case we want to know...
        #ifdef VS_DEBUG
        dump_format(pFormatCtx, 0, npath.c_str(), false);
        #endif
        
        // Find first video stream
        pCodecCtx = 0;
        videoStreamIndex = -1;
        for (int i=0; (pCodecCtx==0) && (i < pFormatCtx->nb_streams); ++i)
            if(pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
                pCodecCtx = (pStream = pFormatCtx->streams[videoStreamIndex = i])->codec;
        if (pCodecCtx == 0)
            throw VideoFile::FileOpenException(errbase + " (no video stream)");
        
        // Find codec for video stream and open it
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec == 0)
            throw VideoFile::UnsupportedCodecException(errbase + " (unsupported codec)");
        
        if(avcodec_open(pCodecCtx, pCodec) < 0)
            throw VideoFile::UnsupportedCodecException(errbase + " (unsupported codec)");
        
        pFrameYUV = avcodec_alloc_frame();
        pNextFrameYUV = avcodec_alloc_frame();
        if ((pFrameYUV == 0) || (pNextFrameYUV == 0))
            throw VideoFile::Exception("Problem during YUV framebuffer initialization");
        
        // Get some info
        frameRate = float(pStream->r_frame_rate.num) / float(pStream->r_frame_rate.den);
        duration = float(pStream->duration * pStream->time_base.num) / float(pStream->time_base.den);
        width = pCodecCtx->width;
        height = pCodecCtx->height;
        while ( (width > fbDimensionLimit) || (height > fbDimensionLimit) ) {
            width /= 2;
            height /= 2;
        }
        
        // Allocate RGB frame buffer
        pFrameRGB = avcodec_alloc_frame();
        if(pFrameRGB == 0)
            throw VideoFile::Exception("Problem during RGB framebuffer initialization");
            
        frameBufferSize = avpicture_get_size(PIX_FMT_RGB24, width, height);
        _frameBuffer = new uint8_t[frameBufferSize];
        if(_frameBuffer == 0)
            throw VideoFile::Exception("Problem during RGB framebuffer initialization");
        
        avpicture_fill((AVPicture *)pFrameRGB, _frameBuffer, PIX_FMT_RGB24, width, height);
        frameBuffer = pFrameRGB->data[0];
        frameBufferSize = pFrameRGB->linesize[0] * height;
        frameBufferStride = pFrameRGB->linesize[0];
        
        // Initialize timebase counter
        fbPTS = pFrameYUV->pts = 0;
        
#ifndef DEPRECATED_IMG_CONVERT
        pSWSCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                 width, height, PIX_FMT_RGB24, SWS_LANCZOS, NULL, NULL, NULL);
#endif
    
    }
    
    bool seek(float time)
    {
        // Translate float time to frametime
        int64_t targetPTS = int64_t(floor(double(time) * pCodecCtx->time_base.den / pCodecCtx->time_base.num));
        
        if ((targetPTS >= fbPTS) && (targetPTS < pNextFrameYUV->pts)) {
            // same frame
            return false;
        } else {
            if ( targetPTS < fbPTS ) {
                // frame backwards
                av_seek_frame(pFormatCtx, videoStreamIndex, targetPTS, AVSEEK_FLAG_BACKWARD);
                nextFrame();
            }
            
            // frame forward
            try {
                while ( pNextFrameYUV->pts < targetPTS )
                    nextFrame();
                convertFrame();
                nextFrame();
            } catch(VideoFile::EndOfStreamException e) {
                sizePTS = fbPTS+1;
                throw e;
            }
            
            return true;
        }
    }
};

#else /* !HAVE_FFMPEG */
class VideoFileImpl {
private:
	// Compile error.
	VideoFileImpl() {}
public:
	// Avoid having to put ifdef's everywhere.
	float frameRate, duration;
	int width, height;
	void *frameBuffer;
	int frameBufferStride;
	bool seek(float time) {return false;}
};

#endif /* !HAVE_FFMPEG */
/* ************************************ */

VideoFile::VideoFile() throw()
    : impl(NULL)
{
}

VideoFile::~VideoFile()
{
    if (impl)
        delete impl;
}

bool VideoFile::isOpen() const throw()
{
    return impl != NULL;
}

void VideoFile::open(const std::string& path, size_t maxDimension) throw(Exception)
{
#ifdef HAVE_FFMPEG
    if (!impl)
        impl = new VideoFileImpl(maxDimension);
    if (impl)
        impl->open(path);
#endif
}

void VideoFile::close() throw()
{
    if (impl) {
        delete impl;
        impl = 0;
    }
}

float VideoFile::getFrameRate() const throw()
{
    return impl ? impl->frameRate : 0;
}

float VideoFile::getDuration() const throw()
{
    return impl ? impl->duration : 0;
}

int VideoFile::getWidth() const throw()
{
    return impl ? impl->width : 0;
}

int VideoFile::getHeight() const throw()
{
    return impl ? impl->height : 0;
}

void* VideoFile::getFrameBuffer() const throw()
{
    return impl ? impl->frameBuffer : 0;
}

int VideoFile::getFrameBufferStride() const throw()
{
    return impl ? impl->frameBufferStride : 0;
}

bool VideoFile::seek(float time) throw(Exception)
{
    return (impl != 0) && impl->seek(time);
}

