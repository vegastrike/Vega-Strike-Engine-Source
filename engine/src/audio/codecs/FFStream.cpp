/*
 * FFStream.cpp
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
// C++ Implementation: Audio::OggStream
//


#ifdef HAVE_FFMPEG

#include "FFStream.h"

#include <utility>
#include <limits>
#include <math.h>
#include <assert.h>

#include "vegadisk/vsfilesystem.h"

#ifndef BUFFER_SIZE
// in samples
#define BUFFER_SIZE 4096
#endif

#ifndef BUFFER_ALIGNMENT
#define BUFFER_ALIGNMENT 0x20
#endif

#include "ffmpeg_init.h"

#if (defined(AVCODEC_MAX_AUDIO_FRAME_SIZE) && ((AVCODEC_MAX_AUDIO_FRAME_SIZE) > (BUFFER_SIZE)))

#undef BUFFER_SIZE
#define BUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE*3)/2)

#endif

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

            std::string filepath;
            VSFileSystem::VSFileType filetype;
            int audioStreamIndex;

            FFData(const std::string &path, VSFileSystem::VSFileType type, Format &fmt, int streamIdx) :
                pFormatCtx(0),
                pCodecCtx(0),
                pCodec(0),
                pStream(0),
                packetBuffer(0),
                packetBufferSize(0),
                sampleBufferBase(0),
                filepath(path),
                filetype(type),
                audioStreamIndex(streamIdx)
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
                    throw FileOpenException(errbase + " (wrong format or file not found)");

                // Dump format info in case we want to know...
#ifdef VS_DEBUG
                dump_format(pFormatCtx, 0, npath.c_str(), false);
#endif

                // Find audio stream
                pCodecCtx = 0;
                streamIndex = -1;
                for (unsigned int i=0; (pCodecCtx==0) && (i < pFormatCtx->nb_streams); ++i)
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
#ifdef SAMPLE_FMT_S24
                case SAMPLE_FMT_S24: fmt.bitsPerSample = 24;
                                     fmt.signedSamples = 1;
                                     break;
#endif
#ifdef SAMPLE_FMT_S32
                case SAMPLE_FMT_S32: fmt.bitsPerSample = 32;
                                     fmt.signedSamples = 1;
                                     break;
#endif
                default:             throw CodecNotFoundException(errbase + " (unsupported audio format)");
                }
                sampleSize = (fmt.bitsPerSample + 7) / 8 * fmt.channels;
                assert(sampleSize > 0);

                // Initialize timebase counter
                sampleBufferStart = 0;
                streamSize = 0;

                // Initialize sample buffer
                sampleBufferBase = malloc(sampleSize * BUFFER_SIZE + BUFFER_ALIGNMENT);
                ptrdiff_t offs = ((reinterpret_cast<ptrdiff_t>(sampleBufferBase)) & (BUFFER_ALIGNMENT-1));
                sampleBufferAligned = ((char*)sampleBufferBase) + BUFFER_ALIGNMENT - offs;
                sampleBufferAlloc = sampleSize * BUFFER_SIZE;
                sampleBuffer = 0;
                sampleBufferSize = 0;
            }

            ~FFData()
            {
                // Free sample buffer
                if (sampleBufferBase != nullptr) {
                    free(sampleBufferBase);
                    sampleBufferBase = nullptr;
                }

                // Close the codec
                if (pCodecCtx != nullptr) {
                    avcodec_close(pCodecCtx);
                    pCodecCtx = nullptr;
                }

                // Close the file
                if (pFormatCtx != nullptr) {
                    av_close_input_file(pFormatCtx);
                    pFormatCtx = nullptr;
                }
            }

            bool saneTimeStamps() const
            {
                return pStream->time_base.num != 0;
            }

            int64_t timeToPts(double time) const
            {
                return int64_t(floor(time * pStream->time_base.den / pStream->time_base.num));
            }

            double ptsToTime(int64_t pts) const
            {
                return double(pts) * pStream->time_base.num / pStream->time_base.den;
            }

            bool hasFrame() const
            {
                return sampleBuffer && sampleBufferSize;
            }

            bool hasPacket() const
            {
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
                return packetBuffer && packetBufferSize
                    && packet.data && packet.size;
#else
                return packetBuffer && packetBufferSize;
#endif
            }

            void readPacket()
            {
                // Read the next packet, skipping all packets that aren't for this stream
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
                packet.size = packetBufferSize;
                packet.data = packetBuffer;
#endif
                do {
                    // Free old packet
                    if (packet.data != NULL)
                        av_free_packet( &packet );

                    // Read new packet
                    if(av_read_frame(pFormatCtx, &packet) < 0)
                        throw EndOfStreamException();
                } while(packet.stream_index != streamIndex);

                packetBufferSize = packet.size;
                packetBuffer = packet.data;

                if (packet.pts != AV_NOPTS_VALUE)
                    sampleBufferStart = uint64_t(floor(ptsToTime(packet.pts) * pCodecCtx->sample_rate));
            }

            void syncPts()
            {
                if (!hasPacket())
                    throw EndOfStreamException();
                sampleBufferSize = 0;
                if (packet.pts != AV_NOPTS_VALUE)
                    sampleBufferStart = uint64_t(floor(ptsToTime(packet.pts) * pCodecCtx->sample_rate));

                if (sampleBufferStart > streamSize)
                    streamSize = sampleBufferStart;
            }

            void decodeFrame()
            {
                if (!hasPacket())
                    throw PacketDecodeException();

                int dataSize = sampleBufferAlloc;
                int used =
#if (LIBAVCODEC_VERSION_MAJOR >= 53)
                    avcodec_decode_audio3(
                        pCodecCtx,
                        (int16_t*)sampleBufferAligned, &dataSize,
                        &packet);
#else
                    avcodec_decode_audio2(
                        pCodecCtx,
                        (int16_t*)sampleBufferAligned, &dataSize,
                        packetBuffer, packetBufferSize);
#endif

                if (used < 0)
                    throw PacketDecodeException();

#if (LIBAVCODEC_VERSION_MAJOR >= 53)

                if ((size_t)used > packet.size)
                    used = packet.size;

                (char*&)(packet.data) += used;
                packet.size -= used;

#else

                if ((size_t)used > packetBufferSize)
                    used = packetBufferSize;

                (char*&)packetBuffer += used;
                packetBufferSize -= used;

#endif

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

    FFStream::FFStream(const std::string& path, int streamIndex, VSFileSystem::VSFileType type)
        : Stream(path)
    {
        ffData = new __impl::FFData(path, type, getFormatInternal(), streamIndex);
    }

    FFStream::~FFStream()
    {
        // destructor closes the file already
        delete ffData;
    }

    double FFStream::getLengthImpl() const
    {
        return double(ffData->streamSize) / getFormat().sampleFrequency;
    }

    double FFStream::getPositionImpl() const
    {
        return double(ffData->sampleBufferStart) / getFormat().sampleFrequency;
    }

    void FFStream::seekImpl(double position)
    {
        if (position < 0)
            position = 0;

        // Translate float time to frametime
        uint64_t targetSample = uint64_t(position * getFormat().sampleFrequency);

        if (   (targetSample >= ffData->sampleBufferStart)
            && (targetSample < ffData->sampleBufferStart + ffData->sampleBufferSize)   )
        {
            // just skip data
            int advance = int(targetSample - ffData->sampleBufferStart);
            (char*&)ffData->sampleBuffer += ffData->sampleSize * advance;
            ffData->sampleBufferStart += advance;
            ffData->sampleBufferSize -= advance;
        } else {
            if (ffData->saneTimeStamps()) {
                // rough seek
                avcodec_flush_buffers(ffData->pCodecCtx);
                av_seek_frame(ffData->pFormatCtx, ffData->streamIndex, ffData->timeToPts(position),
                    (targetSample < ffData->sampleBufferStart + ffData->sampleBufferSize) ? AVSEEK_FLAG_BACKWARD : 0);
                ffData->syncPts();
            } else if (targetSample < ffData->sampleBufferStart) {
                // cannot seek but have to seek backwards, so...
                // ...close the file and reopen (yack)
                std::string path = ffData->filepath;
                VSFileSystem::VSFileType type = ffData->filetype;
                int streamIndex = ffData->audioStreamIndex;

                delete ffData;
                ffData = new __impl::FFData(path, type, getFormatInternal(), streamIndex);
            }

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

    void FFStream::getBufferImpl(void *&buffer, unsigned int &bufferSize)
    {
        if (!ffData->hasFrame())
            throw NoBufferException();

        buffer = ffData->sampleBuffer;
        bufferSize = ffData->sampleSize * ffData->sampleBufferSize;
    }

    void FFStream::nextBufferImpl()
    {
        if (!ffData->hasPacket())
            ffData->readPacket();
        ffData->decodeFrame();
    }


};

#endif // HAVE_FFMPEG

