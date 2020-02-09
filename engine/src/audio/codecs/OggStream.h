//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_OGG_STREAM_H__INCLUDED__
#define __AUDIO_OGG_STREAM_H__INCLUDED__

#ifdef HAVE_OGG

#include "../Stream.h"

#include "vsfilesystem.h"

namespace Audio {

    namespace __impl {
        struct OggData;
    };

    /**
     * OggStream class, used by the OggCodec to decode Ogg-vorbis audio streams
     *
     * @remarks Container formats with multiple embedded streams are supported by
     *      using the special path form "[path]|[stream number]". By default, the
     *      first audio stream is opened.
     *
     * @see Stream, CodecRegistry.
     *
     */
    class OggStream : public Stream
    {
    private: 
        double duration;
        __impl::OggData *oggData;
        VSFileSystem::VSFile file;
        
        void *readBuffer;
        unsigned int readBufferAvail;
        unsigned int readBufferSize;
        
    public:
        /** Open the specified OGG file, or whine about it
         * @remarks Container formats with multiple embedded streams are supported by
         *      using the special path form "[path]|[stream number]". By default, the
         *      first audio stream is opened.
         */
        OggStream(const std::string& path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) throw(Exception);
        
        virtual ~OggStream();
        
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

#endif//HAVE_OGG

#endif//__AUDIO_OGG_STREAM_H__INCLUDED__
