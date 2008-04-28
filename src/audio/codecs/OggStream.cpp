//
// C++ Implementation: Audio::OggStream
//

#ifdef HAVE_OGG

#include "OggStream.h"
#include "OggData.h"
#include "config.h"

#include <utility>
#include <limits>

#include <vorbis/vorbisfile.h>
#include "vsfilesystem.h"

#ifndef OGG_BUFFER_SIZE
#define OGG_BUFFER_SIZE 4096*2*2
#endif

using namespace std;

namespace Audio {

    OggStream::OggStream(const std::string& path, VSFileSystem::VSFileType type) throw(Exception)
        : Stream(path)
    {
        if ( file.OpenReadOnly(path, type) <= VSFileSystem::Ok )
            throw FileOpenException("Error opening file \"" + path + "\"");
        oggData = new __impl::OggData(file, getFormat(), 0);
        
        // Cache duration in case ov_time_total gets expensive
        duration = ov_time_total( &oggData->vorbisFile, oggData->streamIndex );
        
        // Allocate read buffer
        readBufferSize = OGG_BUFFER_SIZE;
        readBufferAvail = 0;
        readBuffer = malloc(readBufferSize);
    }

    OggStream::~OggStream()
    {
        // destructor closes the file already
        delete oggData;
    }

    double OggStream::getLengthImpl() const throw(Exception)
    {
        return duration;
    }
    
    double OggStream::getPositionImpl() const throw()
    {
        return ov_time_tell( &oggData->vorbisFile );
    }
    
    void OggStream::seekImpl(double position) throw(Exception)
    {
        if (position >= duration)
            throw EndOfStreamException();
        
        readBufferAvail = 0;
        
        switch (ov_time_seek(&oggData->vorbisFile, position)) {
        case 0: break;
        case OV_ENOSEEK: throw Exception("Stream not seekable");
        case OV_EINVAL:  throw Exception("Invalid argument or state");
        case OV_EREAD:   throw Exception("Read error");
        case OV_EFAULT:  throw Exception("Internal logic fault, bug or heap/stack corruption");
        case OV_EBADLINK:throw CorruptStreamException(false);
        default:         throw Exception("Unidentified error code");
        }
    }
    
    void OggStream::getBufferImpl(void *&buffer, unsigned int &bufferSize) throw(Exception)
    {
        if (readBufferAvail == 0)
            throw NoBufferException();
        
        buffer = readBuffer;
        bufferSize = readBufferAvail;
    }
    
    void OggStream::nextBufferImpl() throw(Exception)
    {
        int curStream = oggData->streamIndex;
        long ovr;
        switch( ovr = ov_read(&oggData->vorbisFile, 
                              (char*)readBuffer, readBufferSize,
                              0, 2, 1, &curStream) )
        {
        case OV_HOLE:     throw CorruptStreamException(false);
        case OV_EBADLINK: throw CorruptStreamException(false);
        case 0:           throw EndOfStreamException();
        default:          readBufferSize = ovr;
        }
    }


};

#endif // HAVE_OGG

