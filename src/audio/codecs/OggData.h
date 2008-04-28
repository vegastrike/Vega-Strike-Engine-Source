//
// C++ implementation: Audio::__impl::OggData
//

#ifdef HAVE_OGG

#include "../Format.h"
#include <vorbis/vorbisfile.h>

#include "vsfilesystem.h"

namespace Audio {

    namespace __impl {
        struct OggData {
            OggVorbis_File vorbisFile;
            ov_callbacks callbacks;
            int streamIndex;
            
            static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
            static int seek_func(void *datasource, ogg_int64_t offset, int whence);
            static int close_func(void *datasource);
            static long tell_func(void *datasource);
            static int nativeIsLsb();
            
            OggData(VSFileSystem::VSFile &file, Format &fmt, int streamIdx, bool test = false);
            ~OggData();
        };
    }

};

#endif // HAVE_OGG

