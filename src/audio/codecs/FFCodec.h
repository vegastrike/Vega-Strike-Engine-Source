//
// C++ Interface: Audio::OggCodec
//
#ifndef __AUDIO_FFCODEC_H__INCLUDED__
#define __AUDIO_FFCODEC_H__INCLUDED__

#include "Codec.h"

namespace Audio {

    /**
     * OggCodec factory class, for Ogg audio streams.
     * @see CodecRegistry to create OggCodec instances.
     */
    class FFCodec : public Codec
    {
    public:
        FFCodec();
        
        virtual ~FFCodec();
        
        /** @see Codec::canHandle */
        virtual bool canHandle(const std::string& path, bool canOpen, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) throw();
        
        /** @see Codec::open */
        virtual Stream* open(const std::string& path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) throw(Exception);
    };

};

#endif//__AUDIO_FFCODEC_H__INCLUDED__
