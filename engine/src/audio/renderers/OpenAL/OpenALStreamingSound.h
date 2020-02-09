//
// C++ Interface: Audio::OpenALSimpleSound
//
#ifndef __AUDIO_OPENALSTREAMINGSOUND_H__INCLUDED__
#define __AUDIO_OPENALSTREAMINGSOUND_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Types.h"
#include "../../Format.h"
#include "../../SimpleSound.h"
#include "../../SoundBuffer.h"

#include "al.h"

namespace Audio {
    
    class OpenALRenderableSource;

    /**
     * OpenAL Streaming Sound implementation class
     *
     * @remarks This class implements streaming OpenAL sounds.
     *      This will load the sound in chunks into OpenAL buffers,
     *      with double-buffering to make certain there's something
     *      to be played while the application is loading the next.
     *
     *      A package-private function is called to fill buffers
     *      for a configurable amount of buffer time - whenever a source
     *      is playing this sound, this has to happen regularly.
     *
     * @see Sound, SimpleSound
     *
     */
    class OpenALStreamingSound : public SimpleSound
    {
        ALBufferHandle bufferHandles[2];
        Timestamp bufferStarts[2];
        
        SoundBuffer buffer;
        
        Format targetFormat;
        
        size_t bufferSamples;
        
        unsigned char readBufferIndex;
        unsigned char playBufferIndex;
        
    public:
        /** Internal constructor used by derived classes 
         * @param name the resource's name
         * @param type the resource's type
         * @param bufferSamples how many samples a single buffer should hold.
         *      remember double buffering is used, so this holds the number of
         *      samples below which a read would be triggered.
         */
        OpenALStreamingSound(const std::string& name, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile,
            unsigned int bufferSamples = 0) throw();
        
    public:
        virtual ~OpenALStreamingSound();
        
        // The following section contains supporting methods for accessing the stream.
        // Subclasses need not bother with actual stream management, they need only worry
        // about sending the samples to where they're needed.
    protected:
        /** @copydoc Sound::loadImpl */
        virtual void loadImpl(bool wait) throw(Exception);
        
        /** @copydoc Sound::unloadImpl */
        virtual void unloadImpl() throw();
        
        // The following section contains package-private methods.
        // Only OpenAL renderer classes should access them, NOT YOU
    public:
        /** Keep buffers going.
         *
         * @returns An AL buffer handle that can be queued in an AL streaming source, or
         *      AL_NULL_BUFFER if there's no available buffer for the operation (which means
         *      the source should free some buffers)
         * @remarks It will check the buffer queue, and if there are free buffers, it will
         *      free one with new data.
         *          Basically, if you call this function often enough, you'll keep the source
         *      playing.
         * @throws EndOfStreamException when there's no more data to feed from the stream.
         *      You may seek the stream and keep going, for instance, for a looping stream.
         *      Any other exception would be fatal.
         */
        ALBufferHandle readAndFlip() throw(Exception);
        
        /** Notify a dequeued buffer
         *
         * @remarks The function will not do anything, but it will mark the specified buffer
         *      as dequeued, allowing readAndFlip() to use it for new data. The caller is 
         *      expected to have detached the buffer from the source.
         */
        void unqueueBuffer(ALBufferHandle buffer) throw(Exception);
        
        /** Reset the buffer queue */
        void flushBuffers() throw();
        
        /** Get the time base of the stream
         *
         * @returns The timestamp of the first unreturned buffer's starting point.
         */
        Timestamp getTimeBase() const throw();
        
        /** 
         * Set the stream's position, in seconds
         * @see Stream::seek(double)
         */
        void seek(double position) throw(Exception);
    };

};

#endif//__AUDIO_OPENALSTREAMINGSOUND_H__INCLUDED__
