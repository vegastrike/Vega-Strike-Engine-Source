//
// C++ Implementation: Audio::OggCodec
//


#ifdef HAVE_OGG

#include "../CodecRegistry.h"

#include "OggCodec.h"
#include "OggStream.h"
#include "OggData.h"

namespace Audio {

    OggCodec::OggCodec() : Codec("ogg")
    {
    }
    
    OggCodec::~OggCodec()
    {
    }
    
    const Codec::Extensions* OggCodec::getExtensions() const
    {
        static Extensions ext;
        if (ext.empty() == 0) {
            ext.push_back(".ogg");
        }
        return &ext;
    }
    
    bool OggCodec::canHandle(const std::string& path, bool canOpen, VSFileSystem::VSFileType type)
    {
        if (canOpen) {
            try {
                VSFileSystem::VSFile file;
                if ( file.OpenReadOnly(path, type) <= VSFileSystem::Ok )
                    return false;
                
                Format fmt;
                __impl::OggData test(file, fmt, 0);
                
                return true;
            } catch(const Exception& e) {
                return false;
            }
        } else {
            // Can't know for sure
            return true;
        }
    }
    
    Stream* OggCodec::open(const std::string& path, VSFileSystem::VSFileType type)
    {
        return new OggStream(path, type);
    }

    static CodecRegistration registration(new OggCodec());

};

#endif
