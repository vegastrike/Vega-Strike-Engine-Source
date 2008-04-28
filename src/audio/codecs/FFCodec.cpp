//
// C++ Implementation: Audio::FFCodec
//

#ifdef HAVE_FFMPEG

#include "../CodecRegistry.h"

#include "FFCodec.h"
#include "FFStream.h"
#include "config.h"

namespace Audio {

    FFCodec::FFCodec() : Codec("ffmpeg")
    {
    }
    
    FFCodec::~FFCodec()
    {
    }
    
    bool FFCodec::canHandle(const std::string& path, bool canOpen, VSFileSystem::VSFileType type) throw()
    {
        if (canOpen) {
            // I don't really know a way to test files in ffmpeg
            // TODO: find out
            return true;
        } else {
            // Can't know for sure
            return true;
        }
    }
    
    Stream* FFCodec::open(const std::string& path, VSFileSystem::VSFileType type) throw(Exception)
    {
        size_t sep = path.find_last_of('|');
        int streamIndex = (sep != std::string::npos) ? atoi(path.c_str() + sep + 1) : 0;
        std::string filepath = path.substr(0, sep);
        
        return new FFStream(filepath, streamIndex, type);
    }

    static CodecRegistration registration(new FFCodec());

};

#endif
