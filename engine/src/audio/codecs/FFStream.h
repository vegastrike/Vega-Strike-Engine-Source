//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_FF_STREAM_H__INCLUDED__
#define __AUDIO_FF_STREAM_H__INCLUDED__

#ifdef HAVE_FFMPEG

#include "../Stream.h"

#include "vsfilesystem.h"

namespace Audio {

    namespace __impl {
        struct FFData;
    };

    /**
     * FFStream class, used by the FFCodec to decode various audio streams
     *
     * @remarks Container formats with multiple embedded streams are supported by
     *      using the special path form "[path]|[stream number]". By default, the
     *      first audio stream is opened.
     *
     * @see Stream, CodecRegistry.
     *
     */
    class FFStream : public Stream
    {
    private: 
        __impl::FFData *ffData;
        
    public:
        /** Open the specified OGG file, or whine about it
         * @param path the file path
         * @param streamIndex the substream index (for multiple stream containing formats). By
         *      default, the first audio stream is opened.
         * @param type the file type, used by resource management APIs
         */
        FFStream(const std::string& path, int streamIndex = 0, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) throw(Exception);
        
        virtual ~FFStream();
        
    protected:
        
        /** @see Stream::getLengthImpl */
        virtual double getLengthImpl() const throw(Exception);
        
        /** @see Stream::getPositionImpl */
        virtual double getPositionImpl() const throw();
        
        /** @see Stream::seekImpl */
        virtual void seekImpl(double position) throw(Exception);
        
        /** @see Stream::getBufferImpl */
        virtual void getBufferImpl(void *&buffer, unsigned int &bufferSize) throw(Exception);
        
        /** @see Stream::nextBufferImpl */
        virtual void nextBufferImpl() throw(Exception);
    };

};

#endif//HAVE_FFMPEG

#endif//__AUDIO_FF_STREAM_H__INCLUDED__
