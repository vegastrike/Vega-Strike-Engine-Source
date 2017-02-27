//
// C++ Interface: Audio::OpenALSimpleSound
//
#ifndef __AUDIO_OPENALSIMPLESOUND_H__INCLUDED__
#define __AUDIO_OPENALSIMPLESOUND_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Types.h"
#include "../../Format.h"
#include "../../SimpleSound.h"
#include "../../SoundBuffer.h"

#include "al.h"

namespace Audio {

    /**
     * OpenAL Simple Sound implementation class
     *
     * @remarks This class implements simple (non-streaming) OpenAL sounds.
     *      This will load the whole sound into a single OpenAL buffer.
     * @see Sound, SimpleSound
     *
     */
    class OpenALSimpleSound : public SimpleSound
    {
        ALBufferHandle bufferHandle;
        
    public:
        /** Internal constructor used by derived classes */
        OpenALSimpleSound(const std::string& name, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) throw();
        
        /** Package-private: the OpenAL renderer package uses this, YOU DON'T */
        ALBufferHandle getAlBuffer() const { return bufferHandle; }
        
    public:
        virtual ~OpenALSimpleSound();
        
        // The following section contains supporting methods for accessing the stream.
        // Subclasses need not bother with actual stream management, they need only worry
        // about sending the samples to where they're needed.
    protected:
        /** @copydoc Sound::loadImpl */
        virtual void loadImpl(bool wait) throw(Exception);
        
        /** @copydoc Sound::unloadImpl */
        virtual void unloadImpl() throw();
    };

};

#endif//__AUDIO_SIMPLESOUND_H__INCLUDED__
