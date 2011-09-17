//
// C++ Implementation: Audio::OpenALSimpleSound
//

#include "OpenALStreamingSound.h"
#include "OpenALHelpers.h"
#include "config.h"

#include "../../CodecRegistry.h"
#include "../../Stream.h"
#include "al.h"

#ifdef max
#undef max
#endif

#include <utility>
#include <list>
#include <string>

using namespace Audio::__impl::OpenAL;

// Handy macro
#define NUM_BUFFERS (sizeof(bufferHandles) / sizeof(bufferHandles[0]))


namespace Audio {
    
    OpenALStreamingSound::OpenALStreamingSound(const std::string& name, VSFileSystem::VSFileType type,
            unsigned int _bufferSamples)
        throw() :
        SimpleSound(name, type, true),
        bufferSamples(_bufferSamples)
    {
        for (size_t i=0; i < NUM_BUFFERS; ++i)
            bufferHandles[i] = AL_NULL_BUFFER;
    }
    
    OpenALStreamingSound::~OpenALStreamingSound()
    {
    }

    void OpenALStreamingSound::loadImpl(bool wait) 
        throw(Exception)
    {
        // just in case
        unloadImpl();
        
        try {
        
            flags.loading = 1;
            
            // load the stream
            try {
                loadStream();
            } catch(ResourceAlreadyLoadedException e) {
                // Weird...
                getStream()->seek(0);
            }
            SharedPtr<Stream> stream = getStream();
    
            // setup formatted buffer
            // if the format does not match an OpenAL built-in format, we must convert it.
            targetFormat = stream->getFormat();
            targetFormat.signedSamples = (targetFormat.bitsPerSample > 8);
            targetFormat.nativeOrder = 1;
            if (targetFormat.bitsPerSample > 8)
                targetFormat.bitsPerSample = 16;
            else
                targetFormat.bitsPerSample = 8;
            
            // Set capacity to a quarter second or 16k samples, whatever's bigger
            // TODO: make it configurable. But first, implement a central configuration repository.
            bufferSamples = std::max( 16384U, targetFormat.sampleFrequency/4 );
            
            // Prepare a buffer, so we avoid repeated allocation/deallocation
            buffer.reserve(bufferSamples, targetFormat);

            // Prepare AL buffers
            clearAlError();
            alGenBuffers(NUM_BUFFERS, bufferHandles);
            checkAlError();
            
            // Initialize the buffer queue
            flushBuffers();
            
            onLoaded(true);
        } catch(Exception e) {
            onLoaded(false);
            throw e;
        }
    }
    
    void OpenALStreamingSound::flushBuffers()
        throw()
    {
        // Mark as detached, so that readAndFlip() knows to initialize the source
        // and streaming indices
        readBufferIndex = 0;
        
        // Mark the playBufferIndex as uninitialized, by setting it to NUM_BUFFERS
        // readAndFlip will initialize it later.
        playBufferIndex = NUM_BUFFERS;
    }
    
    void OpenALStreamingSound::unloadImpl() 
        throw()
    {
        if (isStreamLoaded())
            closeStream();
        if (bufferHandles[0] != AL_NULL_BUFFER)
            alDeleteBuffers(sizeof(bufferHandles)/sizeof(bufferHandles[0]), bufferHandles);
    }
    
    ALBufferHandle OpenALStreamingSound::readAndFlip()
        throw(Exception)
    {
        if (!isLoaded())
            throw ResourceNotLoadedException(getName());
        
        // Check for a full queue
        if (playBufferIndex == readBufferIndex)
            return AL_NULL_BUFFER;
        
        bufferStarts[readBufferIndex] = getStream()->getPosition();
        
        readBuffer(buffer);
        
        // Break if there's no more data
        if (buffer.getUsedBytes() == 0)
            throw EndOfStreamException();
        
        ALBufferHandle bufferHandle = bufferHandles[readBufferIndex];

        clearAlError();
        alBufferData(bufferHandle, 
            asALFormat(targetFormat), 
            buffer.getBuffer(), buffer.getUsedBytes(), 
            targetFormat.sampleFrequency);
        checkAlError();
        
        if (playBufferIndex == NUM_BUFFERS)
            playBufferIndex = readBufferIndex;
        
        readBufferIndex = (readBufferIndex + 1) % NUM_BUFFERS;
        return bufferHandle;
    }
    
    void OpenALStreamingSound::unqueueBuffer(ALBufferHandle buffer) 
        throw(Exception)
    {
        if (playBufferIndex < NUM_BUFFERS && buffer == bufferHandles[playBufferIndex]) {
            playBufferIndex = (playBufferIndex + 1) % NUM_BUFFERS;
        }
    }
    
    void OpenALStreamingSound::seek(double position)
        throw(Exception)
    {
        if (!isLoaded())
            throw ResourceNotLoadedException(getName());
        
        getStream()->seek(position);
    }
    
    Timestamp OpenALStreamingSound::getTimeBase() const
        throw()
    {
        return bufferStarts[playBufferIndex];
    }
    
};
