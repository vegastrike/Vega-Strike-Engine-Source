//
// C++ Implementation: Audio::OggStream
//

#ifdef HAVE_FFMPEG

#include "FFStream.h"
#include "config.h"

#include <utility>
#include <limits>
#include <math.h>
#include <assert.h>

#include "vsfilesystem.h"

#ifndef BUFFER_SIZE
// in samples
#define BUFFER_SIZE 4096
#endif

#ifndef BUFFER_ALIGNMENT
#define BUFFER_ALIGNMENT 0x20
#endif

extern "C" {
    #include <ffmpeg/avcodec.h>
    #include <ffmpeg/avformat.h>
}

#include "ffmpeg_init.h"

using namespace std;

namespace Audio {

    /**
     * Packet decode exception
     * @remarks thrown when an attempt to decode a frame from a packet with pending data 
     *      fails, forcing a discard of all remaining data.
     */
    class PacketDecodeException : public Exception {
    public:
        PacketDecodeException() {}
        PacketDecodeException(const CodecNotFoundException &other) : Exception(other) {}
    };
    

    namespace __impl {
        struct FFData {
            AVFormatContext *pFormatCtx;
            AVCodecContext *pCodecCtx;
            AVCodec *pCodec;
            AVStream *pStream;
            int streamIndex;
            
            uint8_t *packetBuffer;
            size_t packetBufferSize;
            AVPacket packet;
            
            size_t sampleSize;
            size_t streamSize; // in samples
            
            void *sampleBufferBase;
            void *sampleBufferAligned;
            void *sampleBuffer;
            size_t sampleBufferSize; // in samples
            size_t sampleBufferAlloc; // in bytes
            uint64_t sampleBufferStart; // in samples
            
            
            FFData(const std::string &path, VSFileSystem::VSFileType type, Format &fmt, int streamIdx) throw(Exception) :
                pFormatCtx(0),
                pCodecCtx(0),
                pCodec(0),
                pStream(0),
                packetBuffer(0),
                packetBufferSize(0),
                sampleBufferBase(0)
            {
                packet.data = 0;
                
                char buf[(sizeof(type)+1)/2+1];
                sprintf(buf, "%d", type);
                
                // Initialize libavcodec/libavformat if necessary
                FFMpeg::initLibraries();
                
                // Open file
                std::string npath = std::string("vsfile:") + path + "|" + buf;
                std::string errbase = std::string("Cannot open URL \"") + npath + "\"";
                
                if (  (0 != av_open_input_file(&pFormatCtx, npath.c_str(), NULL, BUFFER_SIZE, NULL))
                    ||(0 >  av_find_stream_info(pFormatCtx))  )
                    throw FileOpenException(errbase + " (wrong format or)"); 
                
                // Dump format info in case we want to know...
                #ifdef VS_DEBUG
                dump_format(pFormatCtx, 0, npath.c_str(), false);
                #endif
                
                // Find audio stream
                pCodecCtx = 0;
                streamIndex = -1;
                for (int i=0; (pCodecCtx==0) && (i < pFormatCtx->nb_streams); ++i)
                    if ((pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) && (streamIdx-- == 0))
                        pCodecCtx = (pStream = pFormatCtx->streams[streamIndex = i])->codec;
                if (pCodecCtx == 0)
                    throw FileOpenException(errbase + " (wrong or no audio stream)");
                
                // Find codec for the audio stream and open it
                pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
                if(pCodec == 0)
                    throw CodecNotFoundException(errbase + " (unsupported codec)");
                
                if(avcodec_open(pCodecCtx, pCodec) < 0)
                    throw CodecNotFoundException(errbase + " (unsupported codec)");
                
                // Get some info
                fmt.sampleFrequency = pCodecCtx->sample_rate;
                fmt.channels = pCodecCtx->channels;
                fmt.nativeOrder = 1; // always so for ffmpeg
                switch (pCodecCtx->sample_fmt) {
                case SAMPLE_FMT_U8:  fmt.bitsPerSample = 8;
                                     fmt.signedSamples = 0;
                                     break;
                case SAMPLE_FMT_S16: fmt.bitsPerSample = 16;
                                     fmt.signedSamples = 1;
                                     break;
                case SAMPLE_FMT_S24: fmt.bitsPerSample = 24;
                                     fmt.signedSamples = 1;
                                     break;
                case SAMPLE_FMT_S32: fmt.bitsPerSample = 32;
                                     fmt.signedSamples = 1;
                                     break;
                default:             throw CodecNotFoundException(errbase + " (unsupported audio format)");
                }
                sampleSize = (fmt.bitsPerSample + 7) / 8 * fmt.channels;
                assert(sampleSize > 0);
                
                // Initialize timebase counter
                sampleBufferStart = 0;
                streamSize = 0;
            
                // Initialize sample buffer
                sampleBufferBase = malloc(sampleSize * BUFFER_SIZE + BUFFER_ALIGNMENT);
                int offs = (*((int*)sampleBufferBase) & *(((int*)BUFFER_ALIGNMENT)-1));
                sampleBufferAligned = ((char*)sampleBufferBase) + BUFFER_ALIGNMENT - offs;
                sampleBufferAlloc = sampleSize * BUFFER_SIZE;
                sampleBuffer = 0;
                sampleBufferSize = 0;
            }
            
            ~FFData()
            {
                // Free sample buffer
                if (sampleBufferBase)
                    free(sampleBufferBase);
            
                // Close the codec
                if (pCodecCtx)
                    avcodec_close(pCodecCtx);
                
                // Close the file
                if (pFormatCtx)
                    av_close_input_file(pFormatCtx);
            }
            
            int64_t timeToPts(double time) const throw()
            {
                return int64_t(floor(time * pCodecCtx->time_base.den / pCodecCtx->time_base.num));
            }
            
            double ptsToTime(int64_t pts) const throw()
            {
                return double(pts) * pCodecCtx->time_base.num / pCodecCtx->time_base.den;
            }
            
            bool hasFrame() const throw()
            {
                return sampleBuffer && sampleBufferSize;
            }
            
            bool hasPacket() const throw()
            {
                return packetBuffer && packetBufferSize;
            }
            
            void readPacket() throw(EndOfStreamException)
            {
                // Read the next packet, skipping all packets that aren't for this stream
                do {
                    // Free old packet
                    if(packet.data != NULL)
                        av_free_packet(&packet);
                    
                    // Read new packet
                    if(av_read_frame(pFormatCtx, &packet) < 0)
                        throw EndOfStreamException();
                } while(packet.stream_index != streamIndex);
        
                packetBufferSize = packet.size;
                packetBuffer = packet.data;
            }
            
            void syncPts() throw(EndOfStreamException)
            {
                if (!hasPacket())
                    throw EndOfStreamException();
                sampleBufferSize = 0;
                sampleBufferStart = int64_t(floor(ptsToTime(packet.pts) * pCodecCtx->sample_rate));
                
                if (sampleBufferStart > streamSize)
                    streamSize = sampleBufferStart;
            }
            
            void decodeFrame() throw(PacketDecodeException)
            {
                if (!hasPacket())
                    throw PacketDecodeException();
                
                int dataSize = sampleBufferAlloc;
                int used = avcodec_decode_audio2(
                    pCodecCtx,
                    (int16_t*)sampleBufferAligned, &dataSize,
                    packetBuffer, packetBufferSize);
                
                if (used < 0)
                    throw PacketDecodeException();
                
                if (used > packetBufferSize)
                    used = packetBufferSize;
                
                (char*&)packetBuffer += used;
                packetBufferSize -= used;
                
                if (dataSize < 0)
                    dataSize = 0;
                
                sampleBuffer = sampleBufferAligned;
                sampleBufferStart += sampleBufferSize;
                sampleBufferSize = dataSize / sampleSize;
                
                if (sampleBufferStart + sampleBufferSize > streamSize)
                    streamSize = sampleBufferStart + sampleBufferSize;
            }
        };
    }

    FFStream::FFStream(const std::string& path, int streamIndex, VSFileSystem::VSFileType type) throw(Exception)
        : Stream(path)
    {
        ffData = new __impl::FFData(path, type, getFormat(), streamIndex);
    }

    FFStream::~FFStream()
    {
        // destructor closes the file already
        delete ffData;
    }

    double FFStream::getLengthImpl() const throw(Exception)
    {
        return double(ffData->streamSize) / getFormat().sampleFrequency;
    }
    
    double FFStream::getPositionImpl() const throw()
    {
        return double(ffData->sampleBufferStart) / getFormat().sampleFrequency;
    }
    
    void FFStream::seekImpl(double position) throw(Exception)
    {
        // Translate float time to frametime
        int64_t targetSample = int64_t(position * getFormat().sampleFrequency);
        
        if (   (targetSample >= ffData->sampleBufferStart) 
            && (targetSample < ffData->sampleBufferStart + ffData->sampleBufferSize)   ) 
        {
            // just skip data
            int advance = int(targetSample - ffData->sampleBufferStart);
            (char*&)ffData->sampleBuffer += ffData->sampleSize * advance;
            ffData->sampleBufferStart += advance;
            ffData->sampleBufferSize -= advance;
        } else {
            // rough seek
            avcodec_flush_buffers(ffData->pCodecCtx);
            av_seek_frame(ffData->pFormatCtx, ffData->streamIndex, ffData->timeToPts(position), AVSEEK_FLAG_BACKWARD);
            ffData->syncPts();
            
            // just skip data (big steps)
            do {
                nextBufferImpl();
            } while (targetSample >= ffData->sampleBufferStart + ffData->sampleBufferSize);
            
            // just skip data (small steps)
            int advance = int(targetSample - ffData->sampleBufferStart);
            (char*&)ffData->sampleBuffer += ffData->sampleSize * advance;
            ffData->sampleBufferStart += advance;
            ffData->sampleBufferSize -= advance;
        }
    }
    
    void FFStream::getBufferImpl(void *&buffer, unsigned int &bufferSize) throw(Exception)
    {
        if (!ffData->hasFrame())
            throw NoBufferException();
        
        buffer = ffData->sampleBuffer;
        bufferSize = ffData->sampleSize * ffData->sampleBufferSize;
    }
    
    void FFStream::nextBufferImpl() throw(Exception)
    {
        if (!ffData->hasPacket())
            ffData->readPacket();
        ffData->decodeFrame();
    }


};

#endif // HAVE_FFMPEG

