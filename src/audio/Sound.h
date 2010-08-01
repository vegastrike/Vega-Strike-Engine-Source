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
        /** @note Accessible to derived classes to support external unloading (ie: memory-short events) */
        struct Flags {
            /** Loaded state of the resource.
            */
            int loaded : 1;
            
            /** Background loading state of the resource.
            * @note Accessible to derived classes to support easier and safer threading
            */
            int loading : 1; 
            
            /** Sound is a streaming resource 
            * @note Accessible to derived classes to support easier and safer threading
             */
            int streaming : 1;
        } flags;
        
    protected:
        /** Internal constructor used by derived classes */
        Sound(const std::string& name, bool streaming) throw();
        
        /** Protected Write access to the sound's format, for implementations. */
        Format& getFormat() throw() { return format; };
        
        
    public:
        virtual ~Sound();
        
        /** Return the path of the associated file. */
        const std::string& getName() const throw() { return name; };
        
        /** Return the format of the sound resource. */
        const Format& getFormat() const throw() { return format; };
        
        /** Return whether the resource has been loaded or not */
        bool isLoaded() const throw() { return flags.loaded; }
        
        /** Return whether the resource is being loaded in the background */
        bool isLoading() const throw() { return flags.loading; }
        
        /** Return whether the resource is being loaded in the background */
        bool isStreaming() const throw() { return flags.streaming; }
        
        /** Load the resource if not loaded 
         * @param wait If true, the function will return only when the resource
         *      has been loaded (or failed to load). Exceptions will be risen on
         *      failure. If false, however, a request for background loading is
         *      issued which may or may not be performed asynchronously. In the
         *      event the implementation does provide background loading of resources,
         *      exceptions and error conditions will be masked because of the
         *      asynchronicity. Only subsequent calls to isLoading() / isLoaded
         *      will allow for such events to be recognized: if it ceases to be
         *      in loading state yet it isn't loaded, either it has been immediately
         *      unloaded (memory short), or an error ocurred during load and it never
         *      became loaded.
         */
        void load(bool wait = true) throw(Exception);
        
        /** Unload the resource if loaded */
        void unload() throw();
        
        // The following section contains all the virtual functions that need be implemented
        // by a concrete Sound class. All are protected, so the stream interface is independent
        // of implementations.
    protected:
    
        /** loadImpl should call this upon process termination 
         * @remarks Either from the foreground thread or the background thread, 
         *      whichever is performing the actual load. 
         *      The method guaranteed to be threadsafe.
         * @param success Whether or not the process succeeded in loading the resource
         */
        virtual void onLoaded(bool success) throw();
        
        /** Wait for background loading to finish
         * @remarks The base implementation checks for completion periodically.
         *      Implementations are likely to have better ways to do this, so they're
         *      welcome to override this method.
         */
        virtual void waitLoad() throw(Exception);
        
        /** Load the resource
         * @note Assume it is unloaded and not loading
         */
        virtual void loadImpl(bool wait) throw(Exception) = 0;
        
        /** Abort an in-progress background load procedure
         * @note Although no exceptions should be risen, the abort request may
         *      not be carried out for various reasons. The caller should check
         *      that on return by calling isLoaded() / isLoading().
         */
        virtual void abortLoad() throw() = 0;
        
        
        /** Unload the resource.
         * @note Assume it is loaded
         */
        virtual void unloadImpl() throw() = 0;
        
    };

};

#endif//__AUDIO_SOUND_H__INCLUDED__
