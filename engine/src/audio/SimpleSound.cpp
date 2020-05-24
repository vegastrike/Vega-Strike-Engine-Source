//
// C++ Implementation: Audio::SimpleSound
//

#include "SimpleSound.h"

#include "CodecRegistry.h"
#include "Stream.h"

namespace Audio {

    SimpleSound::SimpleSound(const std::string& name, VSFileSystem::VSFileType _type, bool streaming) 
        : Sound(name, streaming),
          type(_type)
    {
    }
    
    SimpleSound::~SimpleSound()
    {
    }

    void SimpleSound::loadStream() 
    {
        if (isStreamLoaded())
            throw(ResourceAlreadyLoadedException());
        
        // Open stream and initialize shared pointer
        stream.reset( 
            CodecRegistry::getSingleton()->open(
                getName(),
                getType()
            )
        );
        
        // Copy format
        getFormat() = getStream()->getFormat();
    }
    
    void SimpleSound::closeStream() 
    {
        if (!isStreamLoaded())
            throw(ResourceNotLoadedException());
        stream.reset();
    }
    
    SharedPtr<Stream> SimpleSound::getStream() const 
    {
        if (!isStreamLoaded())
            throw(ResourceNotLoadedException());
        return stream;
    }
    
    void SimpleSound::readBuffer(SoundBuffer &buffer) 
    {
        if (buffer.getFormat() == getFormat()) {
            // Same formats, so all we have to do is read bytes ;)
            buffer.setUsedBytes(
                getStream()->read( buffer.getBuffer(), buffer.getByteCapacity() )
            );
        } else {
            // Save the buffer format, we'll have to reformat to this format
            Format targetFormat = buffer.getFormat();
            
            // Set buffer format to stream format
            buffer.setFormat(getFormat());
            
            // Now read bytes from the stream
            buffer.setUsedBytes(
                getStream()->read( buffer.getBuffer(), buffer.getByteCapacity() )
            );
            
            // Finally we have to reformat the buffer back to the original format
            buffer.reformat(targetFormat);
        }
    }
    
    void SimpleSound::abortLoad()
    {
        // Intentionally blank
    }

};
