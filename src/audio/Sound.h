//
// C++ Interface: Audio::Sound
//
#ifndef __AUDIO_SOUND_H__INCLUDED__
#define __AUDIO_SOUND_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

namespace Audio {

    // Forward declaration of Streams
    class Stream;

    /**
     * Sound abstract class
     *
     * @remarks This is the interface to all kinds of sound resources.
     * @see Stream, Codec, CodecRegistry, for data sources.
     *
     */
    class Sound
    {
    private:
        std::string name;
        Format format;
    
    protected:
        /** Loaded state of the resource.
         * @note Accessible to derived classes to support external unloading (ie: memory-short events)
         */
        bool loaded;
        
    protected:
        /** Internal constructor used by derived classes */
        Sound(const std::string& name) throw();
        
        /** Protected Write access to the sound's format, for implementations. */
        Format& getFormat() throw() { return format; };
        
        
    public:
        virtual ~Sound();
        
        /** Return the path of the associated file. */
        const std::string& getName() const throw() { return name; };
        
        /** Return the format of the sound resource. */
        const Format& getFormat() const throw() { return format; };
        
        /** Return whether the resource has been loaded or not */
        bool isLoaded() const throw() { return loaded; }
        
        /** Load the resource if not loaded */
        void load() throw(Exception);
        
        /** Unload the resource if loaded */
        void unload() throw();
        
        // The following section contains all the virtual functions that need be implemented
        // by a concrete Sound class. All are protected, so the stream interface is independent
        // of implementations.
    protected:
        
        /** Load the resource
         * @note Assume it is unloaded
         */
        virtual double loadImpl() const throw(Exception) = 0;
        
        /** Unload the resource.
         * @note Assume it is loaded
         */
        virtual double unloadImpl() const throw() = 0;
        
    };

};

#endif//__AUDIO_SOUND_H__INCLUDED__
